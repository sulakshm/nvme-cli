#ifndef _NVME_JSON_H
#define _NVME_JSON_H

#include <json-c/json.h>
#include "nvme.h"

enum nvme_print_flags {
	NORMAL	= 0,
	_HUMAN = 0,
	_VERBOSE,
	_TERSE,
	_JSON,
	_VS,
	_BINARY,
	_HEX,


	HUMAN	= 1 << _HUMAN,	/* print human frieldy values */
	VERBOSE	= 1 << _VERBOSE,/* verbosely decode complex values */
	TERSE	= 1 << _TERSE,	/* tersly decode complex values */
	JSON	= 1 << _JSON,	/* display in json format */
	VS	= 1 << _VS,	/* hex dump vendor specific data areas */
	BINARY	= 1 << _BINARY,	/* binary dump raw bytes */
	HEX	= 1 << _HEX,	/* binary dump raw bytes */
};

void nvme_print_json_object(struct json_object *o, int depth);

struct json_object *nvme_id_ctrl_to_json(struct nvme_id_ctrl *ctrl, unsigned long flags);
struct json_object *nvme_id_ns_to_json(struct nvme_id_ns *ns, unsigned long flags);

#endif /* _NVME_JSON_H */
