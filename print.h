#ifndef NVME_PRINT_H
#define NVME_PRINT_H

#include "nvme.h"
#include "util/json.h"
#include <inttypes.h>

void d(unsigned char *buf, int len, int width, int group);
void d_raw(unsigned char *buf, unsigned len);
uint64_t int48_to_long(__u8 *data);

void nvme_show_status(__u16 status);
void nvme_show_relatives(const char *name);

void __nvme_show_id_ctrl(struct nvme_id_ctrl *ctrl, unsigned int mode,
	void (*vendor_show)(__u8 *vs, struct json_object *root));
void nvme_show_id_ctrl(struct nvme_id_ctrl *ctrl, unsigned int mode);
void nvme_show_id_ns(struct nvme_id_ns *ns, unsigned int nsid,
	enum nvme_print_flags flags);
void nvme_show_resv_report(struct nvme_reservation_status *status, int bytes, __u32 cdw11,
	enum nvme_print_flags flags);
void nvme_show_lba_range(struct nvme_lba_range_type *lbrt, int nr_ranges);
void nvme_show_error_log(struct nvme_error_log_page *err_log, int entries,
	const char *devname, enum nvme_print_flags flags);
void nvme_show_smart_log(struct nvme_smart_log *smart, unsigned int nsid,
	const char *devname, enum nvme_print_flags flags);
void nvme_show_ana_log(struct nvme_ana_rsp_hdr *ana_log, const char *devname,
	enum nvme_print_flags flags, size_t len);
void nvme_show_self_test_log(struct nvme_self_test_log *self_test, const char *devname,
	enum nvme_print_flags flags);
void nvme_show_fw_log(struct nvme_firmware_log_page *fw_log, const char *devname,
	enum nvme_print_flags flags);
void nvme_show_effects_log(struct nvme_effects_log_page *effects, unsigned int flags);
void nvme_show_changed_ns_list_log(struct nvme_changed_ns_list_log *log,
	const char *devname, enum nvme_print_flags flags);
void nvme_show_endurance_log(struct nvme_endurance_group_log *endurance_log,
	__u16 group_id, const char *devname, enum nvme_print_flags flags);
void nvme_show_sanitize_log(struct nvme_sanitize_log_page *sanitize,
	const char *devname, enum nvme_print_flags flags);
void nvme_show_ctrl_registers(void *bar, bool fabrics, enum nvme_print_flags flags);
void nvme_show_single_property(int offset, uint64_t prop, int human);
void nvme_show_id_ns_descs(void *data, unsigned nsid, enum nvme_print_flags flags);
void nvme_show_lba_status(struct nvme_lba_status *list, unsigned long len,
	enum nvme_print_flags flags);
void nvme_show_list_items(struct nvme_topology *t, enum nvme_print_flags flags);
void nvme_show_subsystem_list(struct nvme_topology *t,
      enum nvme_print_flags flags);
void nvme_show_id_nvmset(struct nvme_id_nvmset *nvmset, unsigned nvmset_id,
	enum nvme_print_flags flags);
void nvme_show_list_secondary_ctrl(const struct nvme_secondary_controllers_list *sc_list,
	__u32 count, enum nvme_print_flags flags);
void nvme_show_id_ns_granularity_list(const struct nvme_id_ns_granularity_list *glist,
	enum nvme_print_flags flags);
void nvme_show_id_uuid_list(const struct nvme_id_uuid_list *uuid_list,
	enum nvme_print_flags flags);

void nvme_feature_show_fields(__u32 fid, unsigned int result, unsigned char *buf);
void nvme_directive_show(__u8 type, __u8 oper, __u16 spec, __u32 nsid, __u32 result,
	void *buf, __u32 len, enum nvme_print_flags flags);
void nvme_show_select_result(__u32 result);

const char *nvme_status_to_string(__u32 status);
const char *nvme_select_to_string(int sel);
const char *nvme_feature_to_string(int feature);
const char *nvme_register_to_string(int reg);

const char *nvme_ana_state_to_string(__u8 state);
const char *nvme_uuid_to_string(uuid_t uuid);
const char *nvme_cmd_to_string(int admin, __u8 opcode);
const char *get_sanitize_log_sstat_status_str(__u16 status);
const char *fw_to_string(__u64 fw);
long double int128_to_double(__u8 *data);

void json_nvme_id_ns(struct nvme_id_ns *ns, unsigned int mode);
void json_nvme_id_ctrl(struct nvme_id_ctrl *ctrl, unsigned int mode,
			void (*vs)(__u8 *vs, struct json_object *root));
void json_error_log(struct nvme_error_log_page *err_log, int entries);
void json_nvme_resv_report(struct nvme_reservation_status *status,
				  int bytes, __u32 cdw11);
void json_fw_log(struct nvme_firmware_log_page *fw_log, const char *devname);
void json_changed_ns_list_log(struct nvme_changed_ns_list_log *log,
				     const char *devname);
void json_endurance_log(struct nvme_endurance_group_log *endurance_group,
			__u16 group_id);
void json_smart_log(struct nvme_smart_log *smart, unsigned int nsid);
void json_ana_log(struct nvme_ana_rsp_hdr *ana_log, const char *devname);
void json_self_test_log(struct nvme_self_test_log *self_test);
void json_effects_log(struct nvme_effects_log_page *effects_log);
void json_sanitize_log(struct nvme_sanitize_log_page *sanitize_log,
			      const char *devname);
void json_print_nvme_subsystem_list(struct nvme_topology *t);
void json_ctrl_registers(void *bar);
void json_nvme_id_ns_descs(void *data);
void json_nvme_id_nvmset(struct nvme_id_nvmset *nvmset);
void json_nvme_list_secondary_ctrl(const struct nvme_secondary_controllers_list *sc_list,
					  __u32 count);
void json_nvme_id_ns_granularity_list(
	const struct nvme_id_ns_granularity_list *glist);
void json_nvme_id_uuid_list(const struct nvme_id_uuid_list *uuid_list);
void json_detail_ns(struct nvme_namespace *n, struct json_object *ns_attrs);
void json_detail_list(struct nvme_topology *t);
void json_simple_ns(struct nvme_namespace *n, struct json_array *devices);
void json_simple_list(struct nvme_topology *t);
void json_print_list_items(struct nvme_topology *t,
				  enum nvme_print_flags flags);

static inline uint32_t mmio_read32(void *addr)
{
	__le32 *p = addr;
	return le32_to_cpu(*p);
}

/* Access 64-bit registers as 2 32-bit; Some devices fail 64-bit MMIO. */
static inline __u64 mmio_read64(void *addr)
{
	__le32 *p = addr;
	return le32_to_cpu(*p) | ((uint64_t)le32_to_cpu(*(p + 1)) << 32);
}
#endif
