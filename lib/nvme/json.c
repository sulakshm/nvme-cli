#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>

#include "nvme.h"
#include "json.h"

#define json_obj_add_int64(obj, name, val) 			\
({								\
	struct json_object *__o = json_object_new_int64(val);	\
	if (!__o)						\
		goto free_ ## obj;				\
	json_object_object_add(obj, name, __o);			\
})

#define json_obj_add_int(obj, name, val)			\
({								\
	struct json_object *o = json_object_new_int(val);	\
	if (!o)							\
		goto free_ ## obj;				\
	json_object_object_add(obj, name, o);			\
})

#define json_obj_add_bool(obj, name, val)			\
({								\
	struct json_object *o = json_object_new_boolean(val);	\
	if (!o)							\
		goto free_ ## obj;				\
	json_object_object_add(obj, name, o);			\
})


#define json_obj_add_uint(obj, name, val) 			\
	json_obj_add_int(obj, name, val)

#define json_obj_add_le64(obj, name, val) 			\
	json_obj_add_int64(obj, name, le64_to_cpu(val))

#define json_obj_add_le32(obj, name, val) 			\
	json_obj_add_int(obj, name, le32_to_cpu(val))

#define json_obj_add_le16(obj, name, val) 			\
	json_obj_add_int(obj, name, le16_to_cpu(val))

#define json_obj_add_size(obj, name, val)			\
({								\
        struct json_object *__o = json_object_new_int64(val);	\
	if (!__o)						\
		goto free_ ## obj;				\
        json_object_set_serializer(__o, display_size, NULL, NULL); \
	json_object_object_add(obj, name, __o);			\
})

#define json_obj_add_bytes(obj, field, tstruct, show)		\
({ 								\
	struct json_object *field;				\
	field = json_object_new_string_len(			\
				(const char *)tstruct->field, 	\
				sizeof(tstruct->field));	\
	if (!field)						\
		goto free_ ## obj;				\
	json_object_set_serializer(field, show, NULL, NULL);	\
	json_object_object_add(obj, #field, field);		\
})

#define json_obj_add_str(obj, name, val)			\
({ 								\
	struct json_object *__o;				\
	char s[sizeof(val) + 1] = { 0 };			\
	snprintf(s, sizeof(s), "%-.*s", (int)sizeof(val), val);	\
	__o = json_object_new_string(s);			\
	if (!__o)						\
		goto free_ ## obj;				\
	json_object_object_add(obj, name, __o);			\
})

#define json_start(name) 					\
	struct json_object *name = json_object_new_object();	\
	if (!name)						\
		return NULL;

#define json_end(name)						\
	return name;						\
free_ ## name:							\
	json_object_put(name);					\
	return NULL;						\

#define json_obj_add_array(obj, name, ctor, field, i, max, show)\
({								\
	struct json_object *_name = json_object_new_array();	\
	for (i = 0; i < max; i++) {				\
		struct json_object *__name = ctor(&field[i]);	\
		if (!__name) {					\
			json_object_put(_name);			\
			goto free_ ## obj;			\
		}						\
		json_object_set_serializer(__name, show, NULL, NULL);	\
		json_object_array_add(_name, __name);		\
	}							\
	json_object_object_add(obj, #name, _name);		\
})

static void nvme_print_json_array(struct printbuf *pb, struct json_object *o, char *key, int depth)
{
	size_t i, len = json_object_array_length(o);

	for (i = 0; i < len; i++)
		sprintbuf(pb, "%-*s%lu :\n%s", depth - 2, key, i,
			json_object_to_json_string(json_object_array_get_idx(o, i)));
}

void nvme_print_json_object(struct json_object *o, int depth)
{
	int len = depth;

	json_object_object_foreach(o, tkey, tval) {
		if (strlen(tkey) + 1 > len)
			len = strlen(tkey) + 1;
	}

	json_object_object_foreach(o, key, val) {
		switch (json_object_get_type(val)) {
		case json_type_boolean:
		case json_type_double:
		case json_type_int:
		case json_type_string:
			printf("%-*s: %s\n", len, key,
				json_object_to_json_string(val));
			break;
		case json_type_object:
			printf("%-*s: %s\n", len, key,
				json_object_to_json_string(val));
			break;
		case json_type_array:
			printf("%-*s: \n", len, key);
			//nvme_print_json_array(val, key, len);
			break;
		default:
			break;
		}
	}
}

/*
static long double int128_to_double(uint8_t *data)
{
	long double result = 0;
	int i;

	for (i = 0; i < 16; i++) {
		result *= 256;
		result += data[15 - i];
	}
	return result;
}
*/

static int __fls(uint64_t word)
{
        int num = 63;

        if (!(word & (~0ull << 32))) {
                num -= 32;
                word <<= 32;
        }
        if (!(word & (~0ull << 48))) {
                num -= 16;
                word <<= 16;
        }
        if (!(word & (~0ull << 56))) {
                num -= 8;
                word <<= 8;
        }
        if (!(word & (~0ull << 60))) {
                num -= 4;
                word <<= 4;
        }
        if (!(word & (~0ull << 62))) {
                num -= 2;
                word <<= 2;
        }
        if (!(word & (~0ull << 63)))
                num -= 1;
        return num;
}

static int display_size_human(uint64_t v, struct printbuf *pb)
{
	uint16_t pui = 0, pli = 0;
	uint64_t b10, pl = 0, pu = 0;
	uint8_t shift;
	char buf[128];
	int bit, i;

	static const char *units[] = {
		"B",
		"KiB",
		"MiB",
		"GiB",
		"TiB",
		"PiB",
		"EiB",
		"ZiB",
		"YiB",
	};

	static const char *jedec[] = {
		"B",
		"KB",
		"MB",
		"GB",
		"TB",
		"PB",
		"EB",
		"ZB",
		"YB",
	};

	if (v)
		bit = __fls(v);
	else
		return 0;

	shift = 10 * (bit / 10);
	pui = v >> shift;
	if (shift)
		pli = (v & ~(~0ull << shift)) >> (shift - 10);
	pli = (pli * 100) / 1024;

	b10 = 1000;
	for (i = (shift / 10) - 1; i > 0; i--)
		b10 *= 1000;

	pu = v / b10;
	if (shift)
		pl = v - pu * b10;
	pl = ((pl / (b10 / 1000)) + 5) / 10;

	sprintf(buf, "%u.%02u %s (%lu.%02lu %s)", pui, pli, units[bit / 10],
		pu, pl, jedec[bit / 10]);
	return printbuf_memappend(pb, buf, strlen(buf));
}

static int int128_to_str(struct json_object *jso, struct printbuf *pb,
			 int level, int flags)
{
	uint8_t *value;
	uint64_t v = 0;
	int i;

	value = (uint8_t *)json_object_get_string(jso);
	for (i = 7; i >= 0; i--) {
		v *= 256;
		v += value[i];
	}
	return display_size_human(v, pb);
}

static int hex_array_to_str(struct json_object *jso, struct printbuf *pb,
			    int level, int flags)
{
	char buf[128] = { 0 }, *p = buf;
	size_t length, i;
	__u8 *s;

	s = (__u8 *)json_object_get_string(jso);
	length = json_object_get_string_len(jso);

	*p++ = '"';
	for (i = 0; i < length; i++) 
		p += sprintf(p, "%02x", s[i]);
	*p++ = '"';
	return printbuf_memappend(pb, buf, strlen(buf));
}

static int display_uuid(struct json_object *jobj, struct printbuf *pbuf,
		int level, int flags)
{
	char buf[40];
        uuid_t uuid;

	memcpy((void *)uuid, json_object_get_string(jobj),
		sizeof(uuid_t));
        if (uuid_is_null(uuid))
                return 0;
        uuid_unparse(uuid, buf);
	return printbuf_memappend(pbuf, buf, strlen(buf));
}

static int display_uuid_str(struct json_object *jobj, struct printbuf *pbuf,
		int level, int flags)
{
	printbuf_memappend(pbuf, "\"", 1);
	display_uuid(jobj, pbuf, level, flags);
	printbuf_memappend(pbuf, "\"", 1);

	return 0;
}

static int display_size(struct json_object *jobj, struct printbuf *pbuf,
		int level, int flags)
{
	uint64_t blocks = json_object_get_int64(jobj);
	uint64_t bytes = blocks << 9;
	return display_size_human(bytes, pbuf);
}

#if 0
static int nvme_psd_to_str(struct json_object *jso, struct printbuf *pb,
			   int level, int flags)
{
	char buf[4096] = { 0 }, *p = buf;

	json_object_object_foreach(jso, key, val) {
		switch (json_object_get_type(val)) {
		case json_type_boolean:
		case json_type_double:
		case json_type_int:
		case json_type_string:
			if (p != buf)
				*p++ = ' ';
			p += sprintf(p, "%s:%s", key, json_object_to_json_string(val));
			break;
		default:
			break;
		}
	}
	return printbuf_memappend(pb, buf, strlen(buf));
}
#endif

static int depth = 0;
static int space = 0;

static int nvme_json_to_plain(struct json_object *jso, struct printbuf *pb,
			    int level, int flags)
{
	int prev_depth = depth;
	int prev_space = space;
	int len = space;

	json_object_object_foreach(jso, tkey, tval) {
		if (strlen(tkey) + 1 > len)
			len = strlen(tkey) + 1;
	}

	space = len - 2;
	json_object_object_foreach(jso, key, val) {
		switch (json_object_get_type(val)) {
		case json_type_boolean:
		case json_type_double:
		case json_type_int:
		case json_type_string:
			sprintbuf(pb, "%*s%-*s: %s\n", depth, "", len, key,
				json_object_to_json_string(val));
			break;
		case json_type_object:
			depth += 2;
			sprintbuf(pb, "%*s%-*s:\n%s", depth, "", len, key,
				json_object_to_json_string(val));
			depth -= 2;
			break;
		case json_type_array:
			depth += 2;
			nvme_print_json_array(pb, val, key, len);
			depth -= 2;
		default:
			break;
		}
	}
	depth = prev_depth;
	space = prev_space;
	return 0;
}

static struct json_object *nvme_id_ns_lbaf_to_json(struct nvme_lbaf* lbaf)
{
	json_start(jlbaf);
	json_obj_add_le16(jlbaf, "ms", lbaf->ms);
	json_obj_add_uint(jlbaf, "ds", lbaf->ds);
	json_obj_add_uint(jlbaf, "rp", lbaf->rp);
	json_end(jlbaf);
}

#define is_set(x,f) !!(x & f)

struct json_object *nvme_id_ns_nsfeat_to_json(__u8 nsfeat)
{
	json_start(jnsfeat);

	__u8 ioopt = is_set(nsfeat, NVME_NS_FEAT_IO_OPT);
	__u8 idreuse = is_set(nsfeat, NVME_NS_FEAT_ID_REUSE);
	__u8 dulbe = is_set(nsfeat, NVME_NS_FEAT_DULBE);
	__u8 na = is_set(nsfeat, NVME_NS_FEAT_NATOMIC);
	__u8 thin = is_set(nsfeat, NVME_NS_FEAT_THIN);

	json_obj_add_uint(jnsfeat, "Value", nsfeat);
	json_obj_add_bool(jnsfeat, "IO Optimal Constraints", ioopt);
	json_obj_add_bool(jnsfeat, "ID Reuse", idreuse);
	json_obj_add_bool(jnsfeat, "Unwritten Block Error", dulbe);
	json_obj_add_bool(jnsfeat, "Namespace Atomics", na);
	json_obj_add_bool(jnsfeat, "Thin Provisioning", thin);
	json_end(jnsfeat);
}

struct json_object *nvme_id_ns_to_json(struct nvme_id_ns *ns, unsigned long flags)
{
	int i;

	json_start(jns);

/*
	if (!(flags & 1 << 1))
		json_object_set_serializer(jns, nvme_json_to_plain, NULL, NULL);
*/

	json_obj_add_size(jns, "nsze", ns->nsze);
	json_obj_add_size(jns, "ncap", ns->ncap);
	json_obj_add_size(jns, "nuse", ns->nuse);

	if (flags & 1 << 1 || 1)
		json_object_object_add(jns, "nsfeat",
			nvme_id_ns_nsfeat_to_json(ns->nsfeat));
	else
		json_obj_add_uint(jns, "nsfeat", ns->nsfeat);
	json_obj_add_uint(jns, "nlbaf", ns->nlbaf);
	json_obj_add_uint(jns, "flbas", ns->flbas);
	json_obj_add_uint(jns, "mc", ns->mc);
	json_obj_add_uint(jns, "dpc", ns->dpc);
	json_obj_add_uint(jns, "dps", ns->dps);
	json_obj_add_uint(jns, "nmic", ns->nmic);
	json_obj_add_uint(jns, "rescap", ns->rescap);
	json_obj_add_uint(jns, "fpi", ns->fpi);
	json_obj_add_uint(jns, "dlfeat", ns->dlfeat);

	if (ns->nsfeat & NVME_NS_FEAT_NATOMIC) {
		json_obj_add_le16(jns, "nawun", ns->nawun);
		json_obj_add_le16(jns, "nawupf", ns->nawupf);
		json_obj_add_le16(jns, "nacwu", ns->nacwu);
	}
	
	json_obj_add_le16(jns, "nabsn", ns->nabsn);
	json_obj_add_le16(jns, "nabo", ns->nabo);
	json_obj_add_le16(jns, "nabspf", ns->nabspf);
	json_obj_add_le16(jns, "noiob", ns->noiob);

	json_obj_add_bytes(jns, nvmcap, ns, int128_to_str);
	if (ns->nsfeat & NVME_NS_FEAT_IO_OPT) {
		json_obj_add_le16(jns, "npwg", ns->npwg);
		json_obj_add_le16(jns, "npwa", ns->npwa);
		json_obj_add_le16(jns, "npdg", ns->npdg);
		json_obj_add_le16(jns, "npda", ns->npda);
		json_obj_add_le16(jns, "nows", ns->nows);
	}

	json_obj_add_le32(jns, "anagrpid", ns->anagrpid);
	json_obj_add_uint(jns, "nsattr", ns->nsattr);
	json_obj_add_le16(jns, "nvmsetid", ns->nvmsetid);
	json_obj_add_le16(jns, "endgid", ns->endgid);
	json_obj_add_bytes(jns, nguid, ns, display_uuid_str);
	json_obj_add_bytes(jns, eui64, ns, hex_array_to_str);

	if (!(flags & 1 << 1))
		json_obj_add_array(jns, lbafs, nvme_id_ns_lbaf_to_json,
			   ns->lbaf, i, ns->nlbaf + 1, nvme_json_to_plain);
	else
		json_obj_add_array(jns, lbafs, nvme_id_ns_lbaf_to_json,
			   ns->lbaf, i, ns->nlbaf + 1, NULL);
	json_end(jns);
}

static json_object *nvme_id_ctrl_psd_to_json(struct nvme_id_power_state *psd)
{
	json_start(jpsd);

	json_obj_add_le16(jpsd, "mp", psd->max_power);
	json_obj_add_uint(jpsd, "flags", psd->flags);
	json_obj_add_le32(jpsd, "enlat", psd->entry_lat);
	json_obj_add_le32(jpsd, "exlat", psd->exit_lat);
	json_obj_add_uint(jpsd, "rrt", psd->read_tput);
	json_obj_add_uint(jpsd, "rrl", psd->read_lat);
	json_obj_add_uint(jpsd, "rwt", psd->write_tput);
	json_obj_add_uint(jpsd, "rwl", psd->write_lat);
	json_obj_add_le16(jpsd, "idlp", psd->idle_power);
	json_obj_add_uint(jpsd, "ips", psd->idle_scale);
	json_obj_add_le16(jpsd, "actp", psd->active_power);
	json_obj_add_uint(jpsd, "apw", psd->active_work_scale);
	json_obj_add_uint(jpsd, "aps", psd->active_power_scale);

	json_end(jpsd);
}

struct json_object *nvme_id_ctrl_to_json(struct nvme_id_ctrl *ctrl, unsigned long flags)
{
	int i;

	json_start(jctrl);
	json_obj_add_le16(jctrl, "vid", ctrl->vid);
	json_obj_add_le16(jctrl, "ssvid", ctrl->ssvid);
	json_obj_add_str(jctrl, "sn", ctrl->sn);
	json_obj_add_str(jctrl, "mn", ctrl->mn);
	json_obj_add_str(jctrl, "fr", ctrl->fr);
	json_obj_add_uint(jctrl, "rab", ctrl->rab);
	json_obj_add_bytes(jctrl, ieee, ctrl, hex_array_to_str);
	json_obj_add_uint(jctrl, "cmic", ctrl->cmic);
	json_obj_add_uint(jctrl, "mdts", ctrl->mdts);
	json_obj_add_le16(jctrl, "cntlid", ctrl->cntlid);
	json_obj_add_le32(jctrl, "ver", ctrl->ver);
	json_obj_add_le32(jctrl, "rtd3r", ctrl->rtd3r);
	json_obj_add_le32(jctrl, "rtd3e", ctrl->rtd3e);
	json_obj_add_le32(jctrl, "oaes", ctrl->oaes);
	json_obj_add_le32(jctrl, "ctratt", ctrl->ctratt);
	json_obj_add_le16(jctrl, "rrls", ctrl->rrls);
	json_obj_add_le16(jctrl, "cntrltype", ctrl->cntrltype);
	json_obj_add_bytes(jctrl, fguid, ctrl, hex_array_to_str);
	json_obj_add_le16(jctrl, "crdt1", ctrl->crdt1);
	json_obj_add_le16(jctrl, "crdt2", ctrl->crdt2);
	json_obj_add_le16(jctrl, "crdt3", ctrl->crdt3);
	json_obj_add_le16(jctrl, "oacs", ctrl->oacs);
	json_obj_add_uint(jctrl, "acl", ctrl->acl);
	json_obj_add_uint(jctrl, "aerl", ctrl->aerl);
	json_obj_add_uint(jctrl, "frmw", ctrl->frmw);
	json_obj_add_uint(jctrl, "lpa", ctrl->lpa);
	json_obj_add_uint(jctrl, "elpe", ctrl->elpe);
	json_obj_add_uint(jctrl, "npss", ctrl->npss);
	json_obj_add_uint(jctrl, "avscc", ctrl->avscc);
	json_obj_add_uint(jctrl, "apsta", ctrl->apsta);
	json_obj_add_le16(jctrl, "wctemp", ctrl->wctemp);
	json_obj_add_le16(jctrl, "cctemp", ctrl->cctemp);
	json_obj_add_le16(jctrl, "mtfa", ctrl->mtfa);
	json_obj_add_le32(jctrl, "hmpre", ctrl->hmpre);
	json_obj_add_le32(jctrl, "hmmin", ctrl->hmmin);
	json_obj_add_bytes(jctrl, tnvmcap, ctrl, int128_to_str);
	json_obj_add_bytes(jctrl, unvmcap, ctrl, int128_to_str);
	json_obj_add_le32(jctrl, "rpmbs", ctrl->rpmbs);
	json_obj_add_le16(jctrl, "edstt", ctrl->edstt);
	json_obj_add_uint(jctrl, "dsto", ctrl->dsto);
	json_obj_add_uint(jctrl, "fwug", ctrl->fwug);
	json_obj_add_le16(jctrl, "kas", ctrl->kas);
	json_obj_add_le16(jctrl, "hctma", ctrl->hctma);
	json_obj_add_le16(jctrl, "mntmt", ctrl->mntmt);
	json_obj_add_le16(jctrl, "mxtmt", ctrl->mxtmt);
	json_obj_add_le32(jctrl, "sanicap", ctrl->sanicap);
	json_obj_add_le32(jctrl, "hmminds", ctrl->hmminds);
	json_obj_add_le16(jctrl, "hmmaxd", ctrl->hmmaxd);
	json_obj_add_le16(jctrl, "nsetidmax", ctrl->nsetidmax);
	json_obj_add_uint(jctrl, "anatt",ctrl->anatt);
	json_obj_add_uint(jctrl, "anacap", ctrl->anacap);
	json_obj_add_le32(jctrl, "anagrpmax", ctrl->anagrpmax);
	json_obj_add_le32(jctrl, "nanagrpid", ctrl->nanagrpid);
	json_obj_add_uint(jctrl, "sqes", ctrl->sqes);
	json_obj_add_uint(jctrl, "cqes", ctrl->cqes);
	json_obj_add_le16(jctrl, "maxcmd", ctrl->maxcmd);
	json_obj_add_le32(jctrl, "nn", ctrl->nn);
	json_obj_add_le16(jctrl, "oncs", ctrl->oncs);
	json_obj_add_le16(jctrl, "fuses", ctrl->fuses);
	json_obj_add_uint(jctrl, "fna", ctrl->fna);
	json_obj_add_uint(jctrl, "vwc", ctrl->vwc);
	json_obj_add_le16(jctrl, "awun", ctrl->awun);
	json_obj_add_le16(jctrl, "awupf", ctrl->awupf);
	json_obj_add_uint(jctrl, "nvscc", ctrl->nvscc);
	json_obj_add_uint(jctrl, "nwpc", ctrl->nwpc);
	json_obj_add_le16(jctrl, "acwu", ctrl->acwu);
	json_obj_add_le32(jctrl, "sgls", ctrl->sgls);
	json_obj_add_str(jctrl, "subnqn", ctrl->subnqn);
	json_obj_add_le32(jctrl, "ioccsz", ctrl->ioccsz);
	json_obj_add_le32(jctrl, "iorcsz", ctrl->iorcsz);
	json_obj_add_le16(jctrl, "icdoff", ctrl->icdoff);
	json_obj_add_uint(jctrl, "ctrattr", ctrl->ctrattr);
	json_obj_add_uint(jctrl, "msdbd", ctrl->msdbd);
	json_obj_add_array(jctrl, psds, nvme_id_ctrl_psd_to_json, ctrl->psd,
			   i, ctrl->npss + 1, NULL);
	json_end(jctrl);
}

static json_object *nvme_nvmset_attr_to_json(struct nvme_nvmset_attr_entry *attr)
{
	json_start(jattr);
	json_obj_add_le16(jattr, "nvmset_id", attr->id);
	json_obj_add_le16(jattr, "endurance_group_id", attr->endurance_group_id);
	json_obj_add_le32(jattr, "random_4k_read_typical", attr->random_4k_read_typical);
	json_obj_add_le32(jattr, "optimal_write_size", attr->opt_write_size);
	json_obj_add_bytes(jattr, total_nvmset_cap, attr, int128_to_str);
	json_obj_add_bytes(jattr, unalloc_nvmset_cap, attr, int128_to_str);
	json_end(jattr);
}

struct json_object *nvme_nvmset_list_to_json(struct nvme_id_nvmset *nvmset, unsigned long flags)
{
	int i;

	json_start(jnvmset);
	json_obj_add_uint(jnvmset, "nid", nvmset->nid);
	json_obj_add_array(jnvmset, nvmset, nvme_nvmset_attr_to_json,
			   nvmset->ent, i, nvmset->nid, NULL);
	json_end(jnvmset);
}
