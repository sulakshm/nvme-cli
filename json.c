#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "print.h"
#include "util/json.h"
#include "common.h"

static const uint8_t zero_uuid[16] = { 0 };

void json_nvme_id_ns(struct nvme_id_ns *ns, unsigned int mode)
{
	char nguid_buf[2 * sizeof(ns->nguid) + 1],
		eui64_buf[2 * sizeof(ns->eui64) + 1];
	char *nguid = nguid_buf, *eui64 = eui64_buf;
	struct json_object *root;
	struct json_array *lbafs;
	int i;

	long double nvmcap = int128_to_double(ns->nvmcap);

	root = json_create_object();

	json_object_add_value_uint(root, "nsze", le64_to_cpu(ns->nsze));
	json_object_add_value_uint(root, "ncap", le64_to_cpu(ns->ncap));
	json_object_add_value_uint(root, "nuse", le64_to_cpu(ns->nuse));
	json_object_add_value_int(root, "nsfeat", ns->nsfeat);
	json_object_add_value_int(root, "nlbaf", ns->nlbaf);
	json_object_add_value_int(root, "flbas", ns->flbas);
	json_object_add_value_int(root, "mc", ns->mc);
	json_object_add_value_int(root, "dpc", ns->dpc);
	json_object_add_value_int(root, "dps", ns->dps);
	json_object_add_value_int(root, "nmic", ns->nmic);
	json_object_add_value_int(root, "rescap", ns->rescap);
	json_object_add_value_int(root, "fpi", ns->fpi);

	if (ns->nsfeat & NVME_NS_FEAT_NATOMIC) {
		json_object_add_value_int(root, "nawun", le16_to_cpu(ns->nawun));
		json_object_add_value_int(root, "nawupf", le16_to_cpu(ns->nawupf));
		json_object_add_value_int(root, "nacwu", le16_to_cpu(ns->nacwu));
	}

	json_object_add_value_int(root, "nabsn", le16_to_cpu(ns->nabsn));
	json_object_add_value_int(root, "nabo", le16_to_cpu(ns->nabo));
	json_object_add_value_int(root, "nabspf", le16_to_cpu(ns->nabspf));
	json_object_add_value_int(root, "noiob", le16_to_cpu(ns->noiob));
	json_object_add_value_float(root, "nvmcap", nvmcap);
	json_object_add_value_int(root, "nsattr", ns->nsattr);
	json_object_add_value_int(root, "nvmsetid", le16_to_cpu(ns->nvmsetid));

	if (ns->nsfeat & NVME_NS_FEAT_IO_OPT) {
		json_object_add_value_int(root, "npwg", le16_to_cpu(ns->npwg));
		json_object_add_value_int(root, "npwa", le16_to_cpu(ns->npwa));
		json_object_add_value_int(root, "npdg", le16_to_cpu(ns->npdg));
		json_object_add_value_int(root, "npda", le16_to_cpu(ns->npda));
		json_object_add_value_int(root, "nows", le16_to_cpu(ns->nows));
	}

	json_object_add_value_int(root, "anagrpid", le32_to_cpu(ns->anagrpid));
	json_object_add_value_int(root, "endgid", le16_to_cpu(ns->endgid));

	memset(eui64, 0, sizeof(eui64_buf));
	for (i = 0; i < sizeof(ns->eui64); i++)
		eui64 += sprintf(eui64, "%02x", ns->eui64[i]);

	memset(nguid, 0, sizeof(nguid_buf));
	for (i = 0; i < sizeof(ns->nguid); i++)
		nguid += sprintf(nguid, "%02x", ns->nguid[i]);

	json_object_add_value_string(root, "eui64", eui64_buf);
	json_object_add_value_string(root, "nguid", nguid_buf);

	lbafs = json_create_array();
	json_object_add_value_array(root, "lbafs", lbafs);

	for (i = 0; i <= ns->nlbaf; i++) {
		struct json_object *lbaf = json_create_object();

		json_object_add_value_int(lbaf, "ms",
			le16_to_cpu(ns->lbaf[i].ms));
		json_object_add_value_int(lbaf, "ds", ns->lbaf[i].ds);
		json_object_add_value_int(lbaf, "rp", ns->lbaf[i].rp);

		json_array_add_value_object(lbafs, lbaf);
	}

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_nvme_id_ctrl(struct nvme_id_ctrl *ctrl, unsigned int mode,
			void (*vs)(__u8 *vs, struct json_object *root))
{
	struct json_object *root;
	struct json_array *psds;

	long double tnvmcap = int128_to_double(ctrl->tnvmcap);
	long double unvmcap = int128_to_double(ctrl->unvmcap);

	char sn[sizeof(ctrl->sn) + 1], mn[sizeof(ctrl->mn) + 1],
		fr[sizeof(ctrl->fr) + 1], subnqn[sizeof(ctrl->subnqn) + 1];
	__u32 ieee = ctrl->ieee[2] << 16 | ctrl->ieee[1] << 8 | ctrl->ieee[0];

	int i;

	snprintf(sn, sizeof(sn), "%-.*s", (int)sizeof(ctrl->sn), ctrl->sn);
	snprintf(mn, sizeof(mn), "%-.*s", (int)sizeof(ctrl->mn), ctrl->mn);
	snprintf(fr, sizeof(fr), "%-.*s", (int)sizeof(ctrl->fr), ctrl->fr);
	snprintf(subnqn, sizeof(subnqn), "%-.*s", (int)sizeof(ctrl->subnqn), ctrl->subnqn);

	root = json_create_object();

	json_object_add_value_int(root, "vid", le16_to_cpu(ctrl->vid));
	json_object_add_value_int(root, "ssvid", le16_to_cpu(ctrl->ssvid));
	json_object_add_value_string(root, "sn", sn);
	json_object_add_value_string(root, "mn", mn);
	json_object_add_value_string(root, "fr", fr);
	json_object_add_value_int(root, "rab", ctrl->rab);
	json_object_add_value_int(root, "ieee", ieee);
	json_object_add_value_int(root, "cmic", ctrl->cmic);
	json_object_add_value_int(root, "mdts", ctrl->mdts);
	json_object_add_value_int(root, "cntlid", le16_to_cpu(ctrl->cntlid));
	json_object_add_value_uint(root, "ver", le32_to_cpu(ctrl->ver));
	json_object_add_value_uint(root, "rtd3r", le32_to_cpu(ctrl->rtd3r));
	json_object_add_value_uint(root, "rtd3e", le32_to_cpu(ctrl->rtd3e));
	json_object_add_value_uint(root, "oaes", le32_to_cpu(ctrl->oaes));
	json_object_add_value_int(root, "ctratt", le32_to_cpu(ctrl->ctratt));
	json_object_add_value_int(root, "rrls", le16_to_cpu(ctrl->rrls));
	json_object_add_value_int(root, "crdt1", le16_to_cpu(ctrl->crdt1));
	json_object_add_value_int(root, "crdt2", le16_to_cpu(ctrl->crdt2));
	json_object_add_value_int(root, "crdt3", le16_to_cpu(ctrl->crdt3));
	json_object_add_value_int(root, "oacs", le16_to_cpu(ctrl->oacs));
	json_object_add_value_int(root, "acl", ctrl->acl);
	json_object_add_value_int(root, "aerl", ctrl->aerl);
	json_object_add_value_int(root, "frmw", ctrl->frmw);
	json_object_add_value_int(root, "lpa", ctrl->lpa);
	json_object_add_value_int(root, "elpe", ctrl->elpe);
	json_object_add_value_int(root, "npss", ctrl->npss);
	json_object_add_value_int(root, "avscc", ctrl->avscc);
	json_object_add_value_int(root, "apsta", ctrl->apsta);
	json_object_add_value_int(root, "wctemp", le16_to_cpu(ctrl->wctemp));
	json_object_add_value_int(root, "cctemp", le16_to_cpu(ctrl->cctemp));
	json_object_add_value_int(root, "mtfa", le16_to_cpu(ctrl->mtfa));
	json_object_add_value_uint(root, "hmpre", le32_to_cpu(ctrl->hmpre));
	json_object_add_value_uint(root, "hmmin", le32_to_cpu(ctrl->hmmin));
	json_object_add_value_float(root, "tnvmcap", tnvmcap);
	json_object_add_value_float(root, "unvmcap", unvmcap);
	json_object_add_value_uint(root, "rpmbs", le32_to_cpu(ctrl->rpmbs));
	json_object_add_value_int(root, "edstt", le16_to_cpu(ctrl->edstt));
	json_object_add_value_int(root, "dsto", ctrl->dsto);
	json_object_add_value_int(root, "fwug", ctrl->fwug);
	json_object_add_value_int(root, "kas", le16_to_cpu(ctrl->kas));
	json_object_add_value_int(root, "hctma", le16_to_cpu(ctrl->hctma));
	json_object_add_value_int(root, "mntmt", le16_to_cpu(ctrl->mntmt));
	json_object_add_value_int(root, "mxtmt", le16_to_cpu(ctrl->mxtmt));
	json_object_add_value_int(root, "sanicap", le32_to_cpu(ctrl->sanicap));
	json_object_add_value_int(root, "hmminds", le32_to_cpu(ctrl->hmminds));
	json_object_add_value_int(root, "hmmaxd", le16_to_cpu(ctrl->hmmaxd));
	json_object_add_value_int(root, "nsetidmax",
		le16_to_cpu(ctrl->nsetidmax));

	json_object_add_value_int(root, "anatt",ctrl->anatt);
	json_object_add_value_int(root, "anacap", ctrl->anacap);
	json_object_add_value_int(root, "anagrpmax",
		le32_to_cpu(ctrl->anagrpmax));
	json_object_add_value_int(root, "nanagrpid",
		le32_to_cpu(ctrl->nanagrpid));
	json_object_add_value_int(root, "sqes", ctrl->sqes);
	json_object_add_value_int(root, "cqes", ctrl->cqes);
	json_object_add_value_int(root, "maxcmd", le16_to_cpu(ctrl->maxcmd));
	json_object_add_value_uint(root, "nn", le32_to_cpu(ctrl->nn));
	json_object_add_value_int(root, "oncs", le16_to_cpu(ctrl->oncs));
	json_object_add_value_int(root, "fuses", le16_to_cpu(ctrl->fuses));
	json_object_add_value_int(root, "fna", ctrl->fna);
	json_object_add_value_int(root, "vwc", ctrl->vwc);
	json_object_add_value_int(root, "awun", le16_to_cpu(ctrl->awun));
	json_object_add_value_int(root, "awupf", le16_to_cpu(ctrl->awupf));
	json_object_add_value_int(root, "nvscc", ctrl->nvscc);
	json_object_add_value_int(root, "nwpc", ctrl->nwpc);
	json_object_add_value_int(root, "acwu", le16_to_cpu(ctrl->acwu));
	json_object_add_value_int(root, "sgls", le32_to_cpu(ctrl->sgls));

	if (strlen(subnqn))
		json_object_add_value_string(root, "subnqn", subnqn);

	json_object_add_value_int(root, "ioccsz", le32_to_cpu(ctrl->ioccsz));
	json_object_add_value_int(root, "iorcsz", le32_to_cpu(ctrl->iorcsz));
	json_object_add_value_int(root, "icdoff", le16_to_cpu(ctrl->icdoff));
	json_object_add_value_int(root, "ctrattr", ctrl->ctrattr);
	json_object_add_value_int(root, "msdbd", ctrl->msdbd);

	psds = json_create_array();
	json_object_add_value_array(root, "psds", psds);

	for (i = 0; i <= ctrl->npss; i++) {
		struct json_object *psd = json_create_object();

		json_object_add_value_int(psd, "max_power",
			le16_to_cpu(ctrl->psd[i].max_power));
		json_object_add_value_int(psd, "flags", ctrl->psd[i].flags);
		json_object_add_value_uint(psd, "entry_lat",
			le32_to_cpu(ctrl->psd[i].entry_lat));
		json_object_add_value_uint(psd, "exit_lat",
			le32_to_cpu(ctrl->psd[i].exit_lat));
		json_object_add_value_int(psd, "read_tput",
			ctrl->psd[i].read_tput);
		json_object_add_value_int(psd, "read_lat",
			ctrl->psd[i].read_lat);
		json_object_add_value_int(psd, "write_tput",
			ctrl->psd[i].write_tput);
		json_object_add_value_int(psd, "write_lat",
			ctrl->psd[i].write_lat);
		json_object_add_value_int(psd, "idle_power",
			le16_to_cpu(ctrl->psd[i].idle_power));
		json_object_add_value_int(psd, "idle_scale",
			ctrl->psd[i].idle_scale);
		json_object_add_value_int(psd, "active_power",
			le16_to_cpu(ctrl->psd[i].active_power));
		json_object_add_value_int(psd, "active_work_scale",
			ctrl->psd[i].active_work_scale);

		json_array_add_value_object(psds, psd);
	}

	if(vs)
		vs(ctrl->vs, root);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_error_log(struct nvme_error_log_page *err_log, int entries)
{
	struct json_object *root;
	struct json_array *errors;
	int i;

	root = json_create_object();
	errors = json_create_array();
	json_object_add_value_array(root, "errors", errors);

	for (i = 0; i < entries; i++) {
		struct json_object *error = json_create_object();

		json_object_add_value_uint(error, "error_count",
			le64_to_cpu(err_log[i].error_count));
		json_object_add_value_int(error, "sqid",
			le16_to_cpu(err_log[i].sqid));
		json_object_add_value_int(error, "cmdid",
			le16_to_cpu(err_log[i].cmdid));
		json_object_add_value_int(error, "status_field",
			le16_to_cpu(err_log[i].status_field));
		json_object_add_value_int(error, "parm_error_location",
			le16_to_cpu(err_log[i].parm_error_location));
		json_object_add_value_uint(error, "lba",
			le64_to_cpu(err_log[i].lba));
		json_object_add_value_uint(error, "nsid",
			le32_to_cpu(err_log[i].nsid));
		json_object_add_value_int(error, "vs", err_log[i].vs);
		json_object_add_value_int(error, "trtype", err_log[i].trtype);
		json_object_add_value_uint(error, "cs",
			le64_to_cpu(err_log[i].cs));
		json_object_add_value_int(error, "trtype_spec_info",
			le16_to_cpu(err_log[i].trtype_spec_info));

		json_array_add_value_object(errors, error);
	}

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_nvme_resv_report(struct nvme_reservation_status *status,
				  int bytes, __u32 cdw11)
{
	struct json_object *root;
	struct json_array *rcs;
	int i, j, regctl, entries;

	regctl = status->regctl[0] | (status->regctl[1] << 8);

	root = json_create_object();

	json_object_add_value_int(root, "gen", le32_to_cpu(status->gen));
	json_object_add_value_int(root, "rtype", status->rtype);
	json_object_add_value_int(root, "regctl", regctl);
	json_object_add_value_int(root, "ptpls", status->ptpls);

	rcs = json_create_array();
	/* check Extended Data Structure bit */
	if ((cdw11 & 0x1) == 0) {
		/*
		 * if status buffer was too small, don't loop past the end of
		 * the buffer
		 */
		entries = (bytes - 24) / 24;
		if (entries < regctl)
			regctl = entries;

		json_object_add_value_array(root, "regctls", rcs);
		for (i = 0; i < regctl; i++) {
			struct json_object *rc = json_create_object();

			json_object_add_value_int(rc, "cntlid",
				le16_to_cpu(status->regctl_ds[i].cntlid));
			json_object_add_value_int(rc, "rcsts",
				status->regctl_ds[i].rcsts);
			json_object_add_value_uint(rc, "hostid",
				le64_to_cpu(status->regctl_ds[i].hostid));
			json_object_add_value_uint(rc, "rkey",
				le64_to_cpu(status->regctl_ds[i].rkey));

			json_array_add_value_object(rcs, rc);
		}
	} else {
		struct nvme_reservation_status_ext *ext_status = (struct nvme_reservation_status_ext *)status;
		char	hostid[33];

		/* if status buffer was too small, don't loop past the end of the buffer */
		entries = (bytes - 64) / 64;
		if (entries < regctl)
			regctl = entries;

		json_object_add_value_array(root, "regctlext", rcs);
		for (i = 0; i < regctl; i++) {
			struct json_object *rc = json_create_object();

			json_object_add_value_int(rc, "cntlid",
				le16_to_cpu(ext_status->regctl_eds[i].cntlid));
			json_object_add_value_int(rc, "rcsts",
				ext_status->regctl_eds[i].rcsts);
			json_object_add_value_uint(rc, "rkey",
				le64_to_cpu(ext_status->regctl_eds[i].rkey));
			for (j = 0; j < 16; j++)
				sprintf(hostid + j * 2, "%02x",
					ext_status->regctl_eds[i].hostid[j]);

			json_object_add_value_string(rc, "hostid", hostid);
			json_array_add_value_object(rcs, rc);
		}
	}

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_fw_log(struct nvme_firmware_log_page *fw_log, const char *devname)
{
	struct json_object *root;
	struct json_object *fwsi;
	char fmt[21];
	char str[32];
	int i;

	root = json_create_object();
	fwsi = json_create_object();

	json_object_add_value_int(fwsi, "Active Firmware Slot (afi)",
		fw_log->afi);
	for (i = 0; i < 7; i++) {
		if (fw_log->frs[i]) {
			snprintf(fmt, sizeof(fmt), "Firmware Rev Slot %d",
				i + 1);
			snprintf(str, sizeof(str), "%"PRIu64" (%s)",
				(uint64_t)fw_log->frs[i],
			fw_to_string(fw_log->frs[i]));
			json_object_add_value_string(fwsi, fmt, str);
		}
	}
	json_object_add_value_object(root, devname, fwsi);

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_changed_ns_list_log(struct nvme_changed_ns_list_log *log,
				     const char *devname)
{
	struct json_object *root;
	struct json_object *nsi;
	char fmt[32];
	char str[32];
	__u32 nsid;
	int i;

	if (log->log[0] == cpu_to_le32(0xffffffff))
		return;

	root = json_create_object();
	nsi = json_create_object();

	json_object_add_value_string(root, "Changed Namespace List Log",
		devname);

	for (i = 0; i < NVME_MAX_CHANGED_NAMESPACES; i++) {
		nsid = le32_to_cpu(log->log[i]);

		if (nsid == 0)
			break;

		snprintf(fmt, sizeof(fmt), "[%4u]", i + 1);
		snprintf(str, sizeof(str), "%#x", nsid);
		json_object_add_value_string(nsi, fmt, str);
	}

	json_object_add_value_object(root, devname, nsi);
	json_print_object(root, NULL);
	printf("\n");

	json_free_object(root);
}

void json_endurance_log(struct nvme_endurance_group_log *endurance_group,
			__u16 group_id)
{
	struct json_object *root;

	long double endurance_estimate =
		int128_to_double(endurance_group->endurance_estimate);
	long double data_units_read =
		int128_to_double(endurance_group->data_units_read);
	long double data_units_written =
		int128_to_double(endurance_group->data_units_written);
	long double media_units_written =
		int128_to_double(endurance_group->media_units_written);
	long double host_read_cmds =
		int128_to_double(endurance_group->host_read_cmds);
	long double host_write_cmds =
		int128_to_double(endurance_group->host_write_cmds);
	long double media_data_integrity_err =
		int128_to_double(endurance_group->media_data_integrity_err);
	long double num_err_info_log_entries =
		int128_to_double(endurance_group->num_err_info_log_entries);

	root = json_create_object();

	json_object_add_value_int(root, "critical_warning",
		endurance_group->critical_warning);
	json_object_add_value_int(root, "avl_spare",
		endurance_group->avl_spare);
	json_object_add_value_int(root, "avl_spare_threshold",
		endurance_group->avl_spare_threshold);
	json_object_add_value_int(root, "percent_used",
		endurance_group->percent_used);
	json_object_add_value_float(root, "endurance_estimate",
		endurance_estimate);
	json_object_add_value_float(root, "data_units_read", data_units_read);
	json_object_add_value_float(root, "data_units_written",
		data_units_written);
	json_object_add_value_float(root, "mediate_write_commands",
		media_units_written);
	json_object_add_value_float(root, "host_read_cmds", host_read_cmds);
	json_object_add_value_float(root, "host_write_cmds", host_write_cmds);
	json_object_add_value_float(root, "media_data_integrity_err",
		media_data_integrity_err);
	json_object_add_value_float(root, "num_err_info_log_entries",
		num_err_info_log_entries);

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_smart_log(struct nvme_smart_log *smart, unsigned int nsid)
{
	struct json_object *root;
	int c;
	char key[21];

	unsigned int temperature = ((smart->temperature[1] << 8) |
		smart->temperature[0]);

	long double data_units_read = int128_to_double(smart->data_units_read);
	long double data_units_written = int128_to_double(smart->data_units_written);
	long double host_read_commands = int128_to_double(smart->host_reads);
	long double host_write_commands = int128_to_double(smart->host_writes);
	long double controller_busy_time = int128_to_double(smart->ctrl_busy_time);
	long double power_cycles = int128_to_double(smart->power_cycles);
	long double power_on_hours = int128_to_double(smart->power_on_hours);
	long double unsafe_shutdowns = int128_to_double(smart->unsafe_shutdowns);
	long double media_errors = int128_to_double(smart->media_errors);
	long double num_err_log_entries = int128_to_double(smart->num_err_log_entries);

	root = json_create_object();

	json_object_add_value_int(root, "critical_warning",
		smart->critical_warning);
	json_object_add_value_int(root, "temperature", temperature);
	json_object_add_value_int(root, "avail_spare", smart->avail_spare);
	json_object_add_value_int(root, "spare_thresh", smart->spare_thresh);
	json_object_add_value_int(root, "percent_used", smart->percent_used);
	json_object_add_value_int(root, "endurance_grp_critical_warning_summary",
		smart->endu_grp_crit_warn_sumry);
	json_object_add_value_float(root, "data_units_read", data_units_read);
	json_object_add_value_float(root, "data_units_written",
		data_units_written);
	json_object_add_value_float(root, "host_read_commands",
		host_read_commands);
	json_object_add_value_float(root, "host_write_commands",
		host_write_commands);
	json_object_add_value_float(root, "controller_busy_time",
		controller_busy_time);
	json_object_add_value_float(root, "power_cycles", power_cycles);
	json_object_add_value_float(root, "power_on_hours", power_on_hours);
	json_object_add_value_float(root, "unsafe_shutdowns", unsafe_shutdowns);
	json_object_add_value_float(root, "media_errors", media_errors);
	json_object_add_value_float(root, "num_err_log_entries",
		num_err_log_entries);
	json_object_add_value_uint(root, "warning_temp_time",
			le32_to_cpu(smart->warning_temp_time));
	json_object_add_value_uint(root, "critical_comp_time",
			le32_to_cpu(smart->critical_comp_time));

	for (c=0; c < 8; c++) {
		__s32 temp = le16_to_cpu(smart->temp_sensor[c]);

		if (temp == 0)
			continue;
		sprintf(key, "temperature_sensor_%d",c+1);
		json_object_add_value_int(root, key, temp);
	}

	json_object_add_value_uint(root, "thm_temp1_trans_count",
			le32_to_cpu(smart->thm_temp1_trans_count));
	json_object_add_value_uint(root, "thm_temp2_trans_count",
			le32_to_cpu(smart->thm_temp2_trans_count));
	json_object_add_value_uint(root, "thm_temp1_total_time",
			le32_to_cpu(smart->thm_temp1_total_time));
	json_object_add_value_uint(root, "thm_temp2_total_time",
			le32_to_cpu(smart->thm_temp2_total_time));

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_ana_log(struct nvme_ana_rsp_hdr *ana_log, const char *devname)
{
	int offset = sizeof(struct nvme_ana_rsp_hdr);
	struct nvme_ana_rsp_hdr *hdr = ana_log;
	struct nvme_ana_group_desc *ana_desc;
	struct json_array *desc_list;
	struct json_array *ns_list;
	struct json_object *desc;
	struct json_object *nsid;
	struct json_object *root;
	size_t nsid_buf_size;
	void *base = ana_log;
	__u32 nr_nsids;
	int i, j;

	root = json_create_object();
	json_object_add_value_string(root,
			"Asynchronous Namespace Access Log for NVMe device",
			devname);
	json_object_add_value_uint(root, "chgcnt",
			le64_to_cpu(hdr->chgcnt));
	json_object_add_value_uint(root, "ngrps", le16_to_cpu(hdr->ngrps));

	desc_list = json_create_array();
	for (i = 0; i < le16_to_cpu(ana_log->ngrps); i++) {
		desc = json_create_object();
		ana_desc = base + offset;
		nr_nsids = le32_to_cpu(ana_desc->nnsids);
		nsid_buf_size = nr_nsids * sizeof(__le32);

		offset += sizeof(*ana_desc);
		json_object_add_value_uint(desc, "grpid",
				le32_to_cpu(ana_desc->grpid));
		json_object_add_value_uint(desc, "nnsids",
				le32_to_cpu(ana_desc->nnsids));
		json_object_add_value_uint(desc, "chgcnt",
				le64_to_cpu(ana_desc->chgcnt));
		json_object_add_value_string(desc, "state",
				nvme_ana_state_to_string(ana_desc->state));

		ns_list = json_create_array();
		for (j = 0; j < le32_to_cpu(ana_desc->nnsids); j++) {
			nsid = json_create_object();
			json_object_add_value_uint(nsid, "nsid",
					le32_to_cpu(ana_desc->nsids[j]));
			json_array_add_value_object(ns_list, nsid);
		}
		json_object_add_value_array(desc, "NSIDS", ns_list);
		offset += nsid_buf_size;
		json_array_add_value_object(desc_list, desc);
	}

	json_object_add_value_array(root, "ANA DESC LIST", desc_list);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_self_test_log(struct nvme_self_test_log *self_test)
{
	struct json_object *valid_attrs;
	struct json_object *root;
	struct json_array *valid;
	int i;

	root = json_create_object();
	json_object_add_value_int(root, "Current Device Self-Test Operation",
		self_test->crnt_dev_selftest_oprn);
	json_object_add_value_int(root, "Current Device Self-Test Completion",
		self_test->crnt_dev_selftest_compln);
	valid = json_create_array();

	for (i = 0; i < NVME_ST_REPORTS; i++) {
		valid_attrs = json_create_object();
		json_object_add_value_int(valid_attrs, "Self test result",
			self_test->result[i].dsts & 0xf);
		if ((self_test->result[i].dsts & 0xf) == 0xf)
			goto add;
		json_object_add_value_int(valid_attrs, "Self test code",
			self_test->result[i].dsts >> 4);
		json_object_add_value_int(valid_attrs, "Segment number",
			self_test->result[i].seg);
		json_object_add_value_int(valid_attrs, "Valid Diagnostic Information",
			self_test->result[i].vdi);
		json_object_add_value_uint(valid_attrs, "Power on hours",
			le64_to_cpu(self_test->result[i].poh));
		if (self_test->result[i].vdi & NVME_ST_VALID_NSID)
			json_object_add_value_int(valid_attrs, "Namespace Identifier",
				le32_to_cpu(self_test->result[i].nsid));
		if (self_test->result[i].vdi & NVME_ST_VALID_FLBA)
			json_object_add_value_uint(valid_attrs, "Failing LBA",
				le64_to_cpu(self_test->result[i].flba));
		if (self_test->result[i].vdi & NVME_ST_VALID_SCT)
			json_object_add_value_int(valid_attrs, "Status Code Type",
				self_test->result[i].sct);
		if (self_test->result[i].vdi & NVME_ST_VALID_SC)
			json_object_add_value_int(valid_attrs, "Status Code",
				self_test->result[i].sc);
		json_object_add_value_int(valid_attrs, "Vendor Specific",
			(self_test->result[i].vs[1] << 8) |
			(self_test->result[i].vs[0]));
add:
		json_array_add_value_object(valid, valid_attrs);
	}
	json_object_add_value_array(root, "List of Valid Reports", valid);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_effects_log(struct nvme_effects_log_page *effects_log)
{
	struct json_object *root;
	unsigned int opcode;
	char key[128];
	__u32 effect;

	root = json_create_object();

	for (opcode = 0; opcode < 256; opcode++) {
		sprintf(key, "ACS%d (%s)", opcode,
			nvme_cmd_to_string(1, opcode));
		effect = le32_to_cpu(effects_log->acs[opcode]);
		json_object_add_value_uint(root, key, effect);
	}

	for (opcode = 0; opcode < 256; opcode++) {
		sprintf(key, "IOCS%d (%s)", opcode,
			nvme_cmd_to_string(0, opcode));
		effect = le32_to_cpu(effects_log->iocs[opcode]);
		json_object_add_value_uint(root, key, effect);
	}

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_sanitize_log(struct nvme_sanitize_log_page *sanitize_log,
			      const char *devname)
{
	struct json_object *root;
	struct json_object *dev;
	struct json_object *sstat;
	const char *status_str;
	char str[128];
	__u16 status = le16_to_cpu(sanitize_log->status);

	root = json_create_object();
	dev = json_create_object();
	sstat = json_create_object();

	json_object_add_value_int(dev, "sprog",
		le16_to_cpu(sanitize_log->progress));
	json_object_add_value_int(sstat, "global_erased",
		(status & NVME_SANITIZE_LOG_GLOBAL_DATA_ERASED) >> 8);
	json_object_add_value_int(sstat, "no_cmplted_passes",
		(status & NVME_SANITIZE_LOG_NUM_CMPLTED_PASS_MASK) >> 3);

	status_str = get_sanitize_log_sstat_status_str(status);
	sprintf(str, "(%d) %s", status & NVME_SANITIZE_LOG_STATUS_MASK,
		status_str);
	json_object_add_value_string(sstat, "status", str);

	json_object_add_value_object(dev, "sstat", sstat);
	json_object_add_value_uint(dev, "cdw10_info",
		le32_to_cpu(sanitize_log->cdw10_info));
	json_object_add_value_uint(dev, "time_over_write",
		le32_to_cpu(sanitize_log->est_ovrwrt_time));
	json_object_add_value_uint(dev, "time_block_erase",
		le32_to_cpu(sanitize_log->est_blk_erase_time));
	json_object_add_value_uint(dev, "time_crypto_erase",
		le32_to_cpu(sanitize_log->est_crypto_erase_time));

	json_object_add_value_uint(dev, "time_over_write_no_dealloc",
		le32_to_cpu(sanitize_log->est_ovrwrt_time_with_no_deallocate));
	json_object_add_value_uint(dev, "time_block_erase_no_dealloc",
		le32_to_cpu(sanitize_log->est_blk_erase_time_with_no_deallocate));
	json_object_add_value_uint(dev, "time_crypto_erase_no_dealloc",
		le32_to_cpu(sanitize_log->est_crypto_erase_time_with_no_deallocate));

	json_object_add_value_object(root, devname, dev);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_print_nvme_subsystem_list(struct nvme_topology *t)
{
	struct json_object *subsystem_attrs, *path_attrs;
	struct json_array *subsystems, *paths;
	struct json_object *root;
	int i, j;

	root = json_create_object();
	subsystems = json_create_array();

	for (i = 0; i < t->nr_subsystems; i++) {
		struct nvme_subsystem *s = &t->subsystems[i];

		subsystem_attrs = json_create_object();
		json_object_add_value_string(subsystem_attrs,
					     "Name", s->name);
		json_object_add_value_string(subsystem_attrs,
					     "NQN", s->subsysnqn);

		json_array_add_value_object(subsystems, subsystem_attrs);

		paths = json_create_array();
		for (j = 0; j < s->nr_ctrls; j++) {
			struct nvme_ctrl *c = &s->ctrls[j];

			path_attrs = json_create_object();
			json_object_add_value_string(path_attrs, "Name",
					c->name);
			json_object_add_value_string(path_attrs, "Transport",
					c->transport);
			json_object_add_value_string(path_attrs, "Address",
					c->address);
			json_object_add_value_string(path_attrs, "State",
					c->state);
			if (c->ana_state)
				json_object_add_value_string(path_attrs,
						"ANAState", c->ana_state);
			json_array_add_value_object(paths, path_attrs);
		}
		if (j)
			json_object_add_value_array(subsystem_attrs, "Paths",
				paths);
	}

	if (i)
		json_object_add_value_array(root, "Subsystems", subsystems);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_ctrl_registers(void *bar)
{
	uint64_t cap, asq, acq, bpmbl, cmbmsc, pmrmsc;
	uint32_t vs, intms, intmc, cc, csts, nssr, aqa, cmbsz, cmbloc,
		bpinfo, bprsel, cmbsts, pmrcap, pmrctl, pmrsts, pmrebs, pmrswtp;
	struct json_object *root;

	cap = mmio_read64(bar + NVME_REG_CAP);
	vs = mmio_read32(bar + NVME_REG_VS);
	intms = mmio_read32(bar + NVME_REG_INTMS);
	intmc = mmio_read32(bar + NVME_REG_INTMC);
	cc = mmio_read32(bar + NVME_REG_CC);
	csts = mmio_read32(bar + NVME_REG_CSTS);
	nssr = mmio_read32(bar + NVME_REG_NSSR);
	aqa = mmio_read32(bar + NVME_REG_AQA);
	asq = mmio_read64(bar + NVME_REG_ASQ);
	acq = mmio_read64(bar + NVME_REG_ACQ);
	cmbloc = mmio_read32(bar + NVME_REG_CMBLOC);
	cmbsz = mmio_read32(bar + NVME_REG_CMBSZ);
	bpinfo = mmio_read32(bar + NVME_REG_BPINFO);
	bprsel = mmio_read32(bar + NVME_REG_BPRSEL);
	bpmbl = mmio_read64(bar + NVME_REG_BPMBL);
	cmbmsc = mmio_read64(bar + NVME_REG_CMBMSC);
	cmbsts = mmio_read32(bar + NVME_REG_CMBSTS);
	pmrcap = mmio_read32(bar + NVME_REG_PMRCAP);
	pmrctl = mmio_read32(bar + NVME_REG_PMRCTL);
	pmrsts = mmio_read32(bar + NVME_REG_PMRSTS);
	pmrebs = mmio_read32(bar + NVME_REG_PMREBS);
	pmrswtp = mmio_read32(bar + NVME_REG_PMRSWTP);
	pmrmsc = mmio_read64(bar + NVME_REG_PMRMSC);

	root = json_create_object();
	json_object_add_value_uint(root, "cap", cap);
	json_object_add_value_int(root, "vs", vs);
	json_object_add_value_int(root, "intms", intms);
	json_object_add_value_int(root, "intmc", intmc);
	json_object_add_value_int(root, "cc", cc);
	json_object_add_value_int(root, "csts", csts);
	json_object_add_value_int(root, "nssr", nssr);
	json_object_add_value_int(root, "aqa", aqa);
	json_object_add_value_uint(root, "asq", asq);
	json_object_add_value_uint(root, "acq", acq);
	json_object_add_value_int(root, "cmbloc", cmbloc);
	json_object_add_value_int(root, "cmbsz", cmbsz);
	json_object_add_value_int(root, "bpinfo", bpinfo);
	json_object_add_value_int(root, "bprsel", bprsel);
	json_object_add_value_uint(root, "bpmbl", bpmbl);
	json_object_add_value_uint(root, "cmbmsc", cmbmsc);
	json_object_add_value_int(root, "cmbsts", cmbsts);
	json_object_add_value_int(root, "pmrcap", pmrcap);
	json_object_add_value_int(root, "pmrctl", pmrctl);
	json_object_add_value_int(root, "pmrsts", pmrsts);
	json_object_add_value_int(root, "pmrebs", pmrebs);
	json_object_add_value_int(root, "pmrswtp", pmrswtp);
	json_object_add_value_uint(root, "pmrmsc", pmrmsc);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_nvme_id_ns_descs(void *data)
{
	/* large enough to hold uuid str (37) or nguid str (32) + zero byte */
	char json_str[40];
	char *json_str_p;

	union {
		__u8 eui64[NVME_NIDT_EUI64_LEN];
		__u8 nguid[NVME_NIDT_NGUID_LEN];

#ifdef LIBUUID
		uuid_t uuid;
#endif
	} desc;

	struct json_object *root;
	struct json_array *json_array = NULL;

	off_t off;
	int pos, len = 0;
	int i;

	for (pos = 0; pos < NVME_IDENTIFY_DATA_SIZE; pos += len) {
		struct nvme_ns_id_desc *cur = data + pos;
		const char *nidt_name = NULL;

		if (cur->nidl == 0)
			break;

		memset(json_str, 0, sizeof(json_str));
		json_str_p = json_str;
		off = pos + sizeof(*cur);

		switch (cur->nidt) {
		case NVME_NIDT_EUI64:
			memcpy(desc.eui64, data + off, sizeof(desc.eui64));
			for (i = 0; i < sizeof(desc.eui64); i++)
				json_str_p += sprintf(json_str_p, "%02x", desc.eui64[i]);
			len += sizeof(desc.eui64);
			nidt_name = "eui64";
			break;

		case NVME_NIDT_NGUID:
			memcpy(desc.nguid, data + off, sizeof(desc.nguid));
			for (i = 0; i < sizeof(desc.nguid); i++)
				json_str_p += sprintf(json_str_p, "%02x", desc.nguid[i]);
			len += sizeof(desc.nguid);
			nidt_name = "nguid";
			break;

#ifdef LIBUUID
		case NVME_NIDT_UUID:
			memcpy(desc.uuid, data + off, sizeof(desc.uuid));
			uuid_unparse_lower(desc.uuid, json_str);
			len += sizeof(desc.uuid);
			nidt_name = "uuid";
			break;
#endif
		default:
			/* Skip unnkown types */
			len = cur->nidl;
			break;
		}

		if (nidt_name) {
			struct json_object *elem = json_create_object();

			json_object_add_value_int(elem, "loc", pos);
			json_object_add_value_int(elem, "nidt", (int)cur->nidt);
			json_object_add_value_int(elem, "nidl", (int)cur->nidl);
			json_object_add_value_string(elem, "type", nidt_name);
			json_object_add_value_string(elem, nidt_name, json_str);

			if (!json_array) {
				json_array = json_create_array();
			}
			json_array_add_value_object(json_array, elem);
		}

		len += sizeof(*cur);
	}

	root = json_create_object();

	if (json_array)
		json_object_add_value_array(root, "ns-descs", json_array);

	json_print_object(root, NULL);
	printf("\n");

	json_free_object(root);
}

void json_nvme_id_nvmset(struct nvme_id_nvmset *nvmset)
{
	__u32 nent = nvmset->nid;
	struct json_array *entries;
	struct json_object *root;
	int i;

	root = json_create_object();

	json_object_add_value_int(root, "nid", nent);

	entries = json_create_array();
	for (i = 0; i < nent; i++) {
		struct json_object *entry = json_create_object();

		json_object_add_value_int(entry, "nvmset_id",
			  le16_to_cpu(nvmset->ent[i].id));
		json_object_add_value_int(entry, "endurance_group_id",
			  le16_to_cpu(nvmset->ent[i].endurance_group_id));
		json_object_add_value_int(entry, "random_4k_read_typical",
			  le32_to_cpu(nvmset->ent[i].random_4k_read_typical));
		json_object_add_value_int(entry, "optimal_write_size",
			  le32_to_cpu(nvmset->ent[i].opt_write_size));
		json_object_add_value_float(entry, "total_nvmset_cap",
			    int128_to_double(nvmset->ent[i].total_nvmset_cap));
		json_object_add_value_float(entry, "unalloc_nvmset_cap",
			    int128_to_double(nvmset->ent[i].unalloc_nvmset_cap));
		json_array_add_value_object(entries, entry);
	}

	json_object_add_value_array(root, "NVMSet", entries);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_nvme_list_secondary_ctrl(const struct nvme_secondary_controllers_list *sc_list,
					  __u32 count)
{
	const struct nvme_secondary_controller_entry *sc_entry = &sc_list->sc_entry[0];
	__u32 nent = min(sc_list->num, count);
	struct json_array *entries;
	struct json_object *root;
	int i;

	root = json_create_object();

	json_object_add_value_int(root, "num", nent);

	entries = json_create_array();
	for (i = 0; i < nent; i++) {
		struct json_object *entry = json_create_object();

		json_object_add_value_int(entry,
			"secondary-controller-identifier",
			le16_to_cpu(sc_entry[i].scid));
		json_object_add_value_int(entry,
			"primary-controller-identifier",
			le16_to_cpu(sc_entry[i].pcid));
		json_object_add_value_int(entry, "secondary-controller-state",
					  sc_entry[i].scs);
		json_object_add_value_int(entry, "virtual-function-number",
			le16_to_cpu(sc_entry[i].vfn));
		json_object_add_value_int(entry, "num-virtual-queues",
			le16_to_cpu(sc_entry[i].nvq));
		json_object_add_value_int(entry, "num-virtual-interrupts",
			le16_to_cpu(sc_entry[i].nvi));
		json_array_add_value_object(entries, entry);
	}

	json_object_add_value_array(root, "secondary-controllers", entries);

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}


void json_nvme_id_ns_granularity_list(
	const struct nvme_id_ns_granularity_list *glist)
{
	int i;
	struct json_object *root;
	struct json_array *entries;

	root = json_create_object();

	json_object_add_value_int(root, "attributes", glist->attributes);
	json_object_add_value_int(root, "num-descriptors",
		glist->num_descriptors);

	entries = json_create_array();
	for (i = 0; i <= glist->num_descriptors; i++) {
		struct json_object *entry = json_create_object();

		json_object_add_value_uint(entry, "namespace-size-granularity",
			le64_to_cpu(glist->entry[i].namespace_size_granularity));
		json_object_add_value_uint(entry, "namespace-capacity-granularity",
			le64_to_cpu(glist->entry[i].namespace_capacity_granularity));
		json_array_add_value_object(entries, entry);
	}

	json_object_add_value_array(root, "namespace-granularity-list", entries);

	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_nvme_id_uuid_list(const struct nvme_id_uuid_list *uuid_list)
{
	struct json_object *root;
	struct json_array *entries;
	int i;

	root = json_create_object();
	entries = json_create_array();
	/* The 0th entry is reserved */
	for (i = 1; i < NVME_MAX_UUID_ENTRIES; i++) {
		uuid_t uuid;
		struct json_object *entry = json_create_object();

		/* The list is terminated by a zero UUID value */
		if (memcmp(uuid_list->entry[i].uuid, zero_uuid, sizeof(zero_uuid)) == 0)
			break;
		memcpy(&uuid, uuid_list->entry[i].uuid, sizeof(uuid));
		json_object_add_value_int(entry, "association",
			uuid_list->entry[i].header & 0x3);
		json_object_add_value_string(entry, "uuid",
			nvme_uuid_to_string(uuid));
		json_array_add_value_object(entries, entry);
	}
	json_object_add_value_array(root, "UUID-list", entries);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_detail_ns(struct nvme_namespace *n, struct json_object *ns_attrs)
{
	long long lba;
	double nsze, nuse;

	lba = 1 << n->ns.lbaf[(n->ns.flbas & 0x0f)].ds;
	nsze = le64_to_cpu(n->ns.nsze) * lba;
	nuse = le64_to_cpu(n->ns.nuse) * lba;

	json_object_add_value_string(ns_attrs, "NameSpace", n->name);
	json_object_add_value_uint(ns_attrs, "NSID", n->nsid);

	json_object_add_value_uint(ns_attrs, "UsedBytes", nuse);
	json_object_add_value_uint(ns_attrs, "MaximumLBA",
		le64_to_cpu(n->ns.nsze));
	json_object_add_value_uint(ns_attrs, "PhysicalSize", nsze);
	json_object_add_value_uint(ns_attrs, "SectorSize", lba);
}

static void format(char *formatter, size_t fmt_sz, char *tofmt, size_t tofmtsz)
{
	fmt_sz = snprintf(formatter,fmt_sz, "%-*.*s",
		 (int)tofmtsz, (int)tofmtsz, tofmt);
	/* trim() the obnoxious trailing white lines */
	while (fmt_sz) {
		if (formatter[fmt_sz - 1] != ' ' && formatter[fmt_sz - 1] != '\0') {
			formatter[fmt_sz] = '\0';
			break;
		}
		fmt_sz--;
	}
}

void json_detail_list(struct nvme_topology *t)
{
	int i, j, k;
	struct json_object *root;
	struct json_array *devices;
	char formatter[41] = { 0 };

	root = json_create_object();
	devices = json_create_array();

	for (i = 0; i < t->nr_subsystems; i++) {
		struct nvme_subsystem *s = &t->subsystems[i];
		struct json_object *subsys_attrs;
		struct json_array *namespaces, *ctrls;

		subsys_attrs = json_create_object();
		json_object_add_value_string(subsys_attrs, "Subsystem", s->name);
		json_object_add_value_string(subsys_attrs, "SubsystemNQN", s->subsysnqn);

		ctrls = json_create_array();
		json_object_add_value_array(subsys_attrs, "Controllers", ctrls);
		for (j = 0; j < s->nr_ctrls; j++) {
			struct json_object *ctrl_attrs = json_create_object();
			struct nvme_ctrl *c = &s->ctrls[j];
			struct json_array *namespaces;

			json_object_add_value_string(ctrl_attrs, "Controller", c->name);
			json_object_add_value_string(ctrl_attrs, "Transport", c->transport);
			json_object_add_value_string(ctrl_attrs, "Address", c->address);
			json_object_add_value_string(ctrl_attrs, "State", c->state);

			format(formatter, sizeof(formatter), c->id.fr, sizeof(c->id.fr));
			json_object_add_value_string(ctrl_attrs, "Firmware", formatter);

			format(formatter, sizeof(formatter), c->id.mn, sizeof(c->id.mn));
			json_object_add_value_string(ctrl_attrs, "ModelNumber", formatter);

			format(formatter, sizeof(formatter), c->id.sn, sizeof(c->id.sn));
			json_object_add_value_string(ctrl_attrs, "SerialNumber", formatter);

			namespaces = json_create_array();

			for (k = 0; k < c->nr_namespaces; k++) {
				struct json_object *ns_attrs = json_create_object();
				struct nvme_namespace *n = &c->namespaces[k];

				json_detail_ns(n, ns_attrs);
				json_array_add_value_object(namespaces, ns_attrs);
			}
			if (k)
				json_object_add_value_array(ctrl_attrs, "Namespaces", namespaces);
			else
				json_free_array(namespaces);

			json_array_add_value_object(ctrls, ctrl_attrs);
		}

		namespaces = json_create_array();
		for (k = 0; k < s->nr_namespaces; k++) {
			struct json_object *ns_attrs = json_create_object();
			struct nvme_namespace *n = &s->namespaces[k];

			json_detail_ns(n, ns_attrs);
			json_array_add_value_object(namespaces, ns_attrs);
		}
		if (k)
			json_object_add_value_array(subsys_attrs, "Namespaces", namespaces);
		else
			json_free_array(namespaces);

		json_array_add_value_object(devices, subsys_attrs);
	}

	json_object_add_value_array(root, "Devices", devices);
	json_print_object(root, NULL);
	printf("\n");
	json_free_object(root);
}

void json_simple_ns(struct nvme_namespace *n, struct json_array *devices)
{
	struct json_object *device_attrs;
	char formatter[41] = { 0 };
	double nsze, nuse;
	int index = -1;
	long long lba;
	char *devnode;

	if (asprintf(&devnode, "/dev/%s", n->name) < 0)
		return;

	device_attrs = json_create_object();
	json_object_add_value_int(device_attrs, "NameSpace", n->nsid);

	json_object_add_value_string(device_attrs, "DevicePath", devnode);
	free(devnode);

	format(formatter, sizeof(formatter),
			   n->ctrl->id.fr,
			   sizeof(n->ctrl->id.fr));

	json_object_add_value_string(device_attrs, "Firmware", formatter);

	if (sscanf(n->ctrl->name, "nvme%d", &index) == 1)
		json_object_add_value_int(device_attrs, "Index", index);

	format(formatter, sizeof(formatter),
		       n->ctrl->id.mn,
		       sizeof(n->ctrl->id.mn));

	json_object_add_value_string(device_attrs, "ModelNumber", formatter);

	if (index >= 0) {
		char *product = nvme_product_name(index);

		json_object_add_value_string(device_attrs, "ProductName", product);
		free((void*)product);
	}

	format(formatter, sizeof(formatter),
	       n->ctrl->id.sn,
	       sizeof(n->ctrl->id.sn));

	json_object_add_value_string(device_attrs, "SerialNumber", formatter);

	lba = 1 << n->ns.lbaf[(n->ns.flbas & 0x0f)].ds;
	nsze = le64_to_cpu(n->ns.nsze) * lba;
	nuse = le64_to_cpu(n->ns.nuse) * lba;

	json_object_add_value_uint(device_attrs, "UsedBytes", nuse);
	json_object_add_value_uint(device_attrs, "MaximumLBA",
				  le64_to_cpu(n->ns.nsze));
	json_object_add_value_uint(device_attrs, "PhysicalSize", nsze);
	json_object_add_value_uint(device_attrs, "SectorSize", lba);

	json_array_add_value_object(devices, device_attrs);
}

void json_simple_list(struct nvme_topology *t)
{
	struct json_object *root;
	struct json_array *devices;
	int i, j, k;

	root = json_create_object();
	devices = json_create_array();
	for (i = 0; i < t->nr_subsystems; i++) {
		struct nvme_subsystem *s = &t->subsystems[i];

		for (j = 0; j < s->nr_ctrls; j++) {
			struct nvme_ctrl *c = &s->ctrls[j];

			for (k = 0; k < c->nr_namespaces; k++) {
				struct nvme_namespace *n = &c->namespaces[k];
				json_simple_ns(n, devices);
			}
		}

		for (j = 0; j < s->nr_namespaces; j++) {
			struct nvme_namespace *n = &s->namespaces[j];
			json_simple_ns(n, devices);
		}
	}
	json_object_add_value_array(root, "Devices", devices);
	json_print_object(root, NULL);
}

void json_print_list_items(struct nvme_topology *t,
				  enum nvme_print_flags flags)
{
	if (flags & VERBOSE)
		json_detail_list(t);
	else
		json_simple_list(t);
}
