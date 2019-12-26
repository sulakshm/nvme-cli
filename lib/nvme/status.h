#ifndef _NVME_STATUS_H
#define _NVME_STATUS_H

#include <linux/types.h>
#include <stdbool.h>

/*
 * nvme_status_type - It gives SCT(Status Code Type) in status field in
 *                    completion queue entry.
 * @status: status field located at DW3 in completion queue entry
 */
static inline __u8 nvme_status_type(__u16 status)
{
	return (status & 0x700) >> 8;
}

/*
 * nvme_status_to_errno() - Converts nvme return status to errno
 * @status: >= 0 for nvme status field in completion queue entry,
 *          < 0 for linux internal errors
 * @fabrics: true if given status is for fabrics
 *
 * Notes: This function will convert a given status to an errno
 */
__u8 nvme_status_to_errno(int status, bool fabrics);

#endif /* _NVME_STATUS_H */
