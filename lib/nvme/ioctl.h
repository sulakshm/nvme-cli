/*
 * Definitions for the NVM Express interface
 * Copyright (c) 2019, Keith Busch <kbusch@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef _LINUX_NVME_IOCTL
#define _LINUX_NVME_IOCTL

#include <stdbool.h>
#include <linux/types.h>
#include <sys/ioctl.h>

#include "nvme.h"

/* 
 * We can not always count on the kernel UAPI being installed. Use the same
 * 'ifdef' guard to avoid double definitions.
 * */
#ifndef _UAPI_LINUX_NVME_IOCTL_H
#define _UAPI_LINUX_NVME_IOCTL_H

struct nvme_user_io {
	__u8	opcode;
	__u8	flags;
	__u16	control;
	__u16	nblocks;
	__u16	rsvd;
	__u64	metadata;
	__u64	addr;
	__u64	slba;
	__u32	dsmgmt;
	__u32	reftag;
	__u16	apptag;
	__u16	appmask;
};

struct nvme_passthru_cmd {
	__u8	opcode;
	__u8	flags;
	__u16	rsvd1;
	__u32	nsid;
	__u32	cdw2;
	__u32	cdw3;
	__u64	metadata;
	__u64	addr;
	__u32	metadata_len;
	__u32	data_len;
	__u32	cdw10;
	__u32	cdw11;
	__u32	cdw12;
	__u32	cdw13;
	__u32	cdw14;
	__u32	cdw15;
	__u32	timeout_ms;
	__u32	result;
};

#define nvme_admin_cmd nvme_passthru_cmd

#define NVME_IOCTL_ID		_IO('N', 0x40)
#define NVME_IOCTL_ADMIN_CMD	_IOWR('N', 0x41, struct nvme_admin_cmd)
#define NVME_IOCTL_SUBMIT_IO	_IOW('N', 0x42, struct nvme_user_io)
#define NVME_IOCTL_IO_CMD	_IOWR('N', 0x43, struct nvme_passthru_cmd)
#define NVME_IOCTL_RESET	_IO('N', 0x44)
#define NVME_IOCTL_SUBSYS_RESET	_IO('N', 0x45)
#define NVME_IOCTL_RESCAN	_IO('N', 0x46)

#endif /* _UAPI_LINUX_NVME_IOCTL_H */

/**
 * nvme_submit_passthru() - Submit an nvme passthrough command
 * @fd:		File descriptor of nvme device
 * @ioctl_cmd:	The passthrough ioctl type, either NVME_IOCTL_ADMIN_CMD or
 * 		NVME_IOCTL_IO_CMD
 * @cmd:	The nvme command to send
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_submit_passthru(int fd, unsigned long ioctl_cmd,
			 struct nvme_passthru_cmd *cmd);

/**
 * nvme_submit_admin_passthru() - Submit an nvme passthrough admin command
 * @fd:		File descriptor of nvme device
 * @cmd:	The nvme admin command to send
 *
 * Calls nvme_submit_passthru() with NVME_IOCTL_ADMIN_CMD for the ioctl_cmd.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_submit_admin_passthru(int fd, struct nvme_passthru_cmd *cmd);

/**
 * nvme_submit_passthru() - Submit an nvme passthrough command
 * @fd:		File descriptor of nvme device
 * @cmd:	The nvme io command to send
 *
 * Calls nvme_submit_passthru() with NVME_IOCTL_IO_CMD for the ioctl_cmd.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_submit_io_passthru(int fd, struct nvme_passthru_cmd *cmd);

/**
 * nvme_passthru() - Submit an nvme passthrough command
 * @fd:		File descriptor of nvme device
 * @ioctl_cmd:	The passthrough ioctl type, either NVME_IOCTL_ADMIN_CMD or
 * 		NVME_IOCTL_IO_CMD
 * @opcode:	The nvme io command to send
 * @flags:	NVMe command flags (not used)
 * @rsvd:	Reserevd for future use
 * @nsid:	Namespace identifier
 * @cdw2:	Command dword 2
 * @cdw3:	Command dword 3
 * @cdw10:	Command dword 10
 * @cdw11:	Command dword 11
 * @cdw12:	Command dword 12
 * @cdw13:	Command dword 13
 * @cdw14:	Command dword 14
 * @cdw15:	Command dword 15
 * data_len:	Length of the data transfered in this command in bytes
 * @data:	Pointer to user address of the data buffer
 * @metadata_len:Length of metadata transfered in this command
 * @metadata:	Pointer to user address of the metadata buffer
 * @timeout:	How long the kernel waits for the command to complete
 * @result:	Optional field to return the result from the CQE dword 0
 *
 * Parameterized from of nvme_submit_passthru() that sets up a struct
 * nvme_passthru.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_passthru(int fd, unsigned long ioctl_cmd, __u8 opcode, __u8 flags,
		  __u16 rsvd, __u32 nsid, __u32 cdw2, __u32 cdw3,
		  __u32 cdw10, __u32 cdw11, __u32 cdw12,
		  __u32 cdw13, __u32 cdw14, __u32 cdw15,
		  __u32 data_len, void *data, __u32 metadata_len,
		  void *metadata, __u32 timeout_ms, __u32 *result);

/**
 * nvme_io() - Submit an nvme user io command
 * @fd:		File descriptor of nvme device
 * @opcode:	Read, write or compare
 * @slba:	Starting logical block
 * @nblocks:	Number of logical blocks to send (0's based value)
 * @control:	Command control flags
 * @dsmgmt:	Data set management attributes
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 * @data:	Pointer to user address of the data buffer
 * @metadata:	Pointer to user address of the metadata buffer
 *
 * Creates a struct nvme_user_io from the parameters and issues
 * NVME_IOCTL_SUBMIT_IO to the device. The kernel will infer the transfer
 * length based on the nblocks parameter.
 *
 * Deprecated: use nvme_submit_io_passthru() instead as this interface can only
 * support read/write/compare io functions in the kernel.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_io(int fd, __u8 opcode, __u64 slba, __u16 nblocks, __u16 control,
	      __u32 dsmgmt, __u32 reftag, __u16 apptag,
	      __u16 appmask, void *data, void *metadata);

/**
 * nvme_read() - Submit an nvme user read command
 * @fd:		File descriptor of nvme device
 * @slba:	Starting logical block
 * @nblocks:	Number of logical blocks to send (0's based value)
 * @control:	Command control flags
 * @dsmgmt:	Data set management attributes
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 * @data:	Pointer to user address of the data buffer
 * @metadata:	Pointer to user address of the metadata buffer
 *
 * Calls nvme_io() with nvme_cmd_read for the opcode.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_read(int fd, __u64 slba, __u16 nblocks, __u16 control,
	      __u32 dsmgmt, __u32 reftag, __u16 apptag,
	      __u16 appmask, void *data, void *metadata);

/**
 * nvme_write() - Submit an nvme user write command
 * @fd:		File descriptor of nvme device
 * @slba:	Starting logical block
 * @nblocks:	Number of logical blocks to send (0's based value)
 * @control:	Command control flags
 * @dsmgmt:	Data set management attributes
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 * @data:	Pointer to user address of the data buffer
 * @metadata:	Pointer to user address of the metadata buffer
 *
 * Calls nvme_io() with nvme_cmd_write for the opcode.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_write(int fd, __u64 slba, __u16 nblocks, __u16 control,
	       __u32 dsmgmt, __u32 reftag, __u16 apptag,
	       __u16 appmask, void *data, void *metadata);

/**
 * nvme_read() - Submit an nvme user compare command
 * @fd:		File descriptor of nvme device
 * @slba:	Starting logical block
 * @nblocks:	Number of logical blocks to send (0's based value)
 * @control:	Command control flags
 * @dsmgmt:	Data set management attributes
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 * @data:	Pointer to user address of the data buffer
 * @metadata:	Pointer to user address of the metadata buffer
 *
 * Calls nvme_io() with nvme_cmd_compare for the opcode.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_compare(int fd, __u64 slba, __u16 nblocks, __u16 control,
		 __u32 dsmgmt, __u32 reftag, __u16 apptag,
		 __u16 appmask, void *data, void *metadata);


/**
 * nvme_write_zeros() - Submit an nvme write zeroes command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @slba:	Starting logical block
 * @nlb:	Number of logical blocks to clear (0's based value)
 * @control:	Command control flags
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 *
 * The Write Zeroes command is used to set a range of logical blocks to zero.
 * After successful completion of this command, the value returned by
 * subsequent reads of logical blocks in this range shall be all bytes cleared
 * to 0h until a write occurs to this LBA range.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_write_zeros(int fd, __u32 nsid, __u64 slba, __u16 nlb,
		     __u16 control, __u32 reftag, __u16 apptag, __u16 appmask);

/**
 * nvme_write_uncorrectable() - Submit an nvme write uncorrectable command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @slba:	Starting logical block
 * @nlb:	Number of logical blocks to invalidate (0's based value)
 *
 * The Write Uncorrectable command is used to mark a range of logical blocks as
 * invalid. When the specified logical block(s) are read after this operation,
 * a failure is returned with Unrecovered Read Error status. To clear the
 * invalid logical block status, a write operation on those logical blocks is
 * required.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_write_uncorrectable(int fd, __u32 nsid, __u64 slba, __u16 nlb);

/**
 * nvme_verify() - Send an nvme verify command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @slba:	Starting logical block
 * @nlb:	Number of logical blocks to verify (0's based value)
 * @control:	Command control flags
 * @reftag:	This field specifies the Initial Logical Block Reference Tag
 * 		expected value. Used only if the namespace is formatted to use
 * 		end-to-end protection information.
 * @apptag:	This field specifies the Application Tag Mask expected value.
 * 		Used only if the namespace is formatted to use end-to-end
 * 		protection information.
 * @appmask:	This field specifies the Application Tag expected value. Used
 * 		only if the namespace is formatted to use end-to-end protection
 * 		information.
 *
 * The Verify command verifies integrity of stored information by reading data
 * and metadata, if applicable, for the LBAs indicated without transferring any
 * data or metadata to the host.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_verify(int fd, __u32 nsid, __u64 slba, __u16 nlb, __u16 control,
		__u32 reftag, __u16 apptag, __u16 appmask);

/**
 * nvme_flush() - Send an nvme flush command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 *
 * The Flush command is used to request that the contents of volatile write
 * cache be made non-volatile.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_flush(int fd, __u32 nsid);

/**
 * nvme_dsm() - Send an nvme data set management command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @dsm:	The data set management attributes
 * &nr_ranges:	Number of block ranges in the data set management attributes
 *
 * The Dataset Management command is used by the host to indicate attributes
 * for ranges of logical blocks. This includes attributes like frequency that
 * data is read or written, access size, and other information that may be used
 * to optimize performance and reliability, and may be used to
 * deallocate/unmap/trim those logical blocks.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_dsm(int fd, __u32 nsid, __u32 cdw11, struct nvme_dsm_range *dsm,
	     __u16 nr_ranges);

/**
 * nvme_dsm_range () - Constructs a data set range structure
 * @ctx_attrs:	Array of context attributes
 * @llbas:	Array of length in logical blocks
 * @slbas:	Array of starting logical blocks
 * @nr_ranges:	The size of the dsm arrays
 *
 * Each array must be the same size of size 'nr_ranges'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
struct nvme_dsm_range *nvme_setup_dsm_range(__u32 *ctx_attrs,
					    __u32 *llbas, __u64 *slbas,
					    __u16 nr_ranges);

/**
 * nvme_resv_acquire() - Send an nvme reservation acquire
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @rtype:	The type of reservation to be created
 * @racqa:	The action that is performed by the command
 * @iekey:	Set to ignore the existing key
 * @crkey:	The current reservation key associated with the host
 * @nrkey:	The reservation key to be unregistered from the namespace if
 * 		the action is preempt
 *
 * The Reservation Acquire command is used to acquire a reservation on a
 * namespace, preempt a reservation held on a namespace, and abort a
 * reservation held on a namespace.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_resv_acquire(int fd, __u32 nsid, __u8 rtype, __u8 racqa,
		      bool iekey, __u64 crkey, __u64 nrkey);

/**
 * nvme_resv_register() - Send an nvme reservation register
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @rrega:	The registration action
 * @cptpl:	Change persist through power loss
 * @iekey:	Set to ignore the existing key
 * @crkey:	The current reservation key associated with the host
 * @nrkey:	The new reservation key to be register if action is register or
 * 		replace
 *
 * The Reservation Register command is used to register, unregister, or replace
 * a reservation key.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_resv_register(int fd, __u32 nsid, __u8 rrega, __u8 cptpl,
		       bool iekey, __u64 crkey, __u64 nrkey);

/**
 * nvme_resv_release() - Send an nvme reservation release
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @rtype:	The type of reservation to release
 * @rrela:	Reservation releast action
 * @iekey:	Set to ignore the existing key
 * @crkey:	The current reservation key to release
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_resv_release(int fd, __u32 nsid, __u8 rtype, __u8 rrela,
		      bool iekey, __u64 crkey);

/**
 * nvme_resv_report() - Send an nvme reservation report
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier
 * @numd:	Number of dwords to transfer (0's based value)
 * @cdw11:	Command dword 11 values
 * @report:	The user space destination address to store the reservation report
 *
 * Returns a Reservation Status data structure to memory that describes the
 * registration and reservation status of a namespace. See
 * 'struct nvme_reservation_status'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_resv_report(int fd, __u32 nsid, __u32 numd, __u32 cdw11,
	struct nvme_reservation_status *report);

/**
 * nvme_identify13() - Send the NVMe 1.3 version of identify
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier, if applicable
 * @cdw10:	The CNS value, and if applicable, CNTID
 * @cdw11:	The NVMe Set ID if CNS is 04h
 * @data:	User space destination address to transfer the data
 *
 * The Identify command returns a data buffer that describes information about
 * the NVM subsystem, the controller or the namespace(s).
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify13(int fd, __u32 nsid, __u32 cdw10, __u32 cdw11, void *data);

/**
 * nvme_identify() - Send the original identify command, sans cdw11 option
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier, if applicable
 * @cdw10:	The CNS value, and if applicable, CNTID
 * @data:	User space destination address to transfer the data
 *
 * The Identify command returns a data buffer that describes information about
 * the NVM subsystem, the controller(s) or the namespace(s).
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify(int fd, __u32 nsid, __u32 cdw10, void *data);

/**
 * nvme_identify_ctrl() - Retrieves nvme identify controller
 * @fd:		File descriptor of nvme device
 * id:		User space destination address to transfer the data
 *
 * Sends nvme identify with CNS value 01h.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ctrl(int fd, struct nvme_id_ctrl *id);

/**
 * nvme_identify_ns() - Retrieves nvme identify namespace
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace to identify
 * @present:	Set to return only if namespace is allocated, clear for all
 * 		namespces
 * @id:		User space destination address to transfer the data
 *
 * If the Namespace Identifier (NSID) field specifies an active NSID, then the
 * Identify Namespace data structure is returned to the host for that specified
 * namespace.
 *
 * If the controller supports the Namespace Management capability and the NSID
 * field is set to FFFFFFFFh, then the controller returns an Identify Namespace
 * data structure that specifies capabilities that are common across namespaces
 * for this controller.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ns(int fd, __u32 nsid, bool present, struct nvme_id_ns *ns);

/**
 * nvme_identify_ns_list() - Retrieves identify namespaces list
 * @fd:		File descriptor of nvme device
 * @nsid:	Return namespaces greater than this identifer
 * @all:	Clear all to return only active namespaces; set to include
 * 		inactive namespaces
 * @ns_list:	User space destination address to transfer the data
 *
 * A list of 1024 namespace IDs is returned to the host containing NSIDs in
 * increasing order that are greater than the value specified in the Namespace
 * Identifier (nsid) field of the command.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ns_list(int fd, __u32 nsid, bool all,
			  struct nvme_ns_list *ns_list);

/**
 * nvme_identify_ctrl_list() - Retrieves identify controller list
 * @fd:		File descriptor of nvme device
 * @nsid:	Return controllers that are attached to this nsid
 * @cntlid:	Starting CNTLID to return in the list
 * cntlist:	User space destination address to transfer the data
 *
 * Up to 2047 controller identifiers is returned containing a controller
 * identifier greater than or equal to the value specified in the Controller
 * Identifier (cndtil).
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ctrl_list(int fd, __u32 nsid, __u16 cntid,
			    struct nvme_controller_list *cntlist);

/**
 * nvme_identify_ns_descs() - Retrieves namespace descriptor list
 * @fd:		File descriptor of nvme device
 * @nsid:	The namespace id to retrieve destriptors
 * @descs:	User space destination address to transfer the data
 *
 * A list of Namespace Identification Descriptor structures is returned to the
 * host for the namespace specified in the Namespace Identifier (NSID) field if
 * it is an active NSID.
 *
 * The data returned is in the form of an arrray of 'struct nvme_ns_id_desc'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ns_descs(int fd, __u32 nsid, struct nvme_ns_id_desc *descs);

/**
 * nvme_identify_nvmset() - Retrieves NVM Set List
 * @fd:		File descriptor of nvme device
 * @nvmeset_id:	NVM Set Identifier
 * @nvmset:	User space destination address to transfer the data
 *
 * Retrieves an NVM Set List, struct nvme_id_nvmset. The data structure is an
 * ordered list by NVM Set Identifier, starting with the first NVM Set
 * Identifier supported by the NVM subsystem that is equal to or greater than
 * the NVM Set Identifier
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_nvmset(int fd, __u16 nvmset_id, struct nvme_id_nvmset *nvmset);

/**
 * nvme_identify_uuid() - Retrieves device's UUIDs
 * @fd:		File descriptor of nvme device
 * @uuid_list:	User space destination address to transfer the data
 *
 * Each UUID List entry is either 0h, the NVMe Invalid UUID, or a valid UUID.
 * Valid UUIDs are those which are non-zero and are not the NVMe Invalid UUID.
 * The data transfered is in the form of a 'struct nvme_id_uuid_list'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_uuid(int fd, struct nvme_id_uuid_list *uuid_list);

/**
 * nvme_identify_secondary_ctrl_list() - Retrieves secondary controller list
 * @fd:		File descriptor of nvme device
 * @nsid:	Return controllers attached to this namespace
 * @cntid:	Return controllers starting at this identifier
 * @sc_list:	User space destination address to transfer the data
 *
 * A Secondary Controller List is returned to the host for up to 127 secondary
 * controllers associated with the primary controller processing this command.
 * The list contains entries for controller identifiers greater than or equal
 * to the value specified in the Controller Identifier (cntid).
 *
 * Data is returned in the form of 'struct nvme_secondary_controllers_list'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_secondary_ctrl_list(int fd, __u32 nsid, __u16 cntid,
			struct nvme_secondary_controllers_list *sc_list);

/**
 * nvme_identify_ns_granularity() - Retrieves namespace granularity
 * 				    identifaction
 * @fd:		File descriptor of nvme device
 * @gr_list:	User space destination address to transfer the data
 *
 * If the controller supports reporting of Namespace Granularity, then a
 * Namespace Granularity List is returned to the host for up to sixteen
 * namespace granularity descriptors
 *
 * Data is returned in the form of 'struct nvme_id_ns_granularity_list'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_identify_ns_granularity(int fd,
	struct nvme_id_ns_granularity_list *gr_list);

/**
 * nvme_get_log() - Original nvme get log, fewer optional parameters
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier applicable to some logs
 * @log_id:	Log page identifier
 * @rae:	Retain asynchronous events
 * @data_len:	Length of provided user buffer to hold the log data in bytes
 * @data:	User space destination address to transfer the data
 *
 * The Get Log Page command returns a data buffer containing the log page
 * requested.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_log(int fd, __u32 nsid, __u8 log_id, bool rae,
		 __u32 data_len, void *data);

/**
 * nvme_get_log14() - NVMe 1.4 version of get log
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier applicable to some logs
 * @log_id:	Log page identifier
 * @lsp:	Log specific field
 * @lpo:	Log page offset for partial log transfers
 * @lsi:	Endurance group information
 * @rae:	Retain asynchronous events
 * @uuid_ix:	UUID selection, if supported
 * @data_len:	Length of provided user buffer to hold the log data in bytes
 * @data:	User space destination address to transfer the data
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_log14(int fd, __u32 nsid, __u8 log_id, __u8 lsp, __u64 lpo,
		   __u16 lsi, bool rae, __u8 uuid_ix, __u32 data_len,
		   void *data);

/**
 * nvme_get_log13() - The NVMe 1.3 version of get log
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace identifier applicable to some logs
 * @log_id:	Log page identifier
 * @lsp:	Log specific field
 * @lip:	Log specific information
 * @lpo:	Log page offset for partial log transfers
 * @rae:	Retain asynchronous events
 * @data_len:	Length of provided user buffer to hold the log data in bytes
 * @data:	User space destination address to transfer the data
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_log13(int fd, __u32 nsid, __u8 log_id, __u8 lsp, __u64 lpo,
		   __u16 lsi, bool rae, __u32 data_len, void *data);

/**
 * nvme_get_telemetry_log() - Retrieves a telemetry log
 * @fd:		File descriptor of nvme device
 * @gen_report:	Set to capture log for host initiated
 * @ctrl_init:	Set to use controller initiated data
 * @offset:	Offset into the telemetry data
 * @data_len:	Length of provided user buffer to hold the log data in bytes
 * @data:	User address for log page data
 *
 * Telemetry enables manufacturers to collect internal data logs to improve the
 * functionality and reliability of products. The telemetry data collection may
 * be initiated by the host or by the controller. The data is returned in the
 * Telemetry Host-Initiated log page or the Telemetry Controller-Initiated log
 * page. The data captured is vendor specific. The telemetry feature defines
 * the mechanism to collect the vendor specific data.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_telemetry_log(int fd, bool gen_report, bool ctrl_init,
			   __u64 offset, size_t data_len, void *data);

/**
 * nvme_fw_log() - Retrieves a controller firmware log
 * @fd:		File descriptor of nvme device
 * @fw_log:	User address to store the log page
 *
 *
 * This log page is used to describe the firmware revision stored in each
 * firmware slot supported. The firmware revision is indicated as an ASCII
 * string. The log page also indicates the active slot number.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_fw_log(int fd, struct nvme_firmware_log_page *fw_log);

/**
 * nvme_changed_ns_list_log() - Retrieve namespace changed list
 * @fd:		File descriptor of nvme device
 * @ns_list:	User address to store the log page
 *
 * This log page is used to describe namespaces attached to this controller
 * that have changed since the last time the namespace was identified, been
 * added, or deleted.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_changed_ns_list_log(int fd, struct nvme_changed_ns_list_log *ns_list);

/**
 * nvme_error_log() - Retrieve nvme error log
 * @fd:		File descriptor of nvme device
 * @entries:	Number of error log entries allocated
 * @err_log:	Array of error logs of size 'entries'
 *
 * This log page is used to describe extended error information for a command
 * that completed with error or report an error that is not specific to a
 * particular command.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_error_log(int fd, int entries, struct nvme_error_log_page *err_log);

/**
 * nvme_smart_log() - Retrieve nvme smart log
 * @fd:		File descriptor of nvme device
 * @nsid:	Optional namespace identifier
 * @smart_log:	User address to store the smart log
 *
 * This log page is used to provide SMART and general health information. The
 * information provided is over the life of the controller and is retained
 * across power cycles. To request the controller log page, the namespace
 * identifier specified is FFFFFFFFh. The controller may also support
 * requesting the log page on a per namespace basis, as indicated by bit 0 of
 * the LPA field in the Identify Controller data structure.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_smart_log(int fd, __u32 nsid, struct nvme_smart_log *smart_log);

/**
 * nvme_ana_log() - Retrieve nvme asymmetric namespace access log
 * @fd:		File descriptor of nvme device
 * @rgo:	Set to Return Groups Only without Namespace IDs
 * @log_len:	The allocated length of the log page
 * @ana_log	User address to store the ana log
 *
 * This log consists of a header describing the log and descriptors containing
 * the asymmetric namespace access information for ANA Groups that contain
 * namespaces that are attached to the controller processing the command.
 *
 * Returns a variable length log page with header 'struct nvme_ana_rsp_hdr',
 * and entries 'struct nvme_ana_group_desc'.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_ana_log(int fd, int rgo, size_t log_len,
	struct nvme_ana_group_desc *ana_log);

/**
 * nvme_effects_log() - Retrieve nvme command effects log
 * @fd:		File descriptor of nvme device
 * @effects_log:User address to store the effects log
 *
 * This log page is used to describe the commands that the controller supports
 * and the effects of those commands on the state of the NVM subsystem.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_effects_log(int fd, struct nvme_effects_log_page *effects_log);

/**
 * nvme_discovery_log() - Retrieve the discovery log page
 * @fd:		File descriptor of nvme device
 * @log:	User address to store the discovery log
 * @size:	The allocated size of the log
 *
 * Supported only by fabrics discovery controllers, returning discovery
 * records.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_discovery_log(int fd, struct nvmf_disc_rsp_page_hdr *log, __u32 size);

/**
 * nvme_sanitize_log() - Retrieve sanitize status
 * @fd:		File descriptor of nvme device
 * @log:	User address to store the sanitize log
 *
 * The Sanitize Status log page is used to report sanitize operation time
 * estimates and information about the most recent sanitize operation.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_sanitize_log(int fd, struct nvme_sanitize_log_page *log);

/**
 * nvme_endurance_log() - Retrieve endurance group log entries
 * @fd:		File descriptor of nvme device
 * @group_id:	Starting group identifier to return in the list
 * @log:	User address to store the endurance log
 *
 * This log page indicates if an Endurance Group Event has occurred for a
 * particular Endurance Group. If an Endurance Group Event has occurred, the
 * details of the particular event are included in the Endurance Group
 * Information log page for that Endurance Group. An asynchronous event is
 * generated when an entry for an Endurance Group is newly added to this log
 * page.
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_endurance_log(int fd, __u16 group_id,
		       struct nvme_endurance_group_log *log);

/**
 * nvme_set_feature() - Set a feature attribute
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID, if applicable
 * @fid:	Feature identifier
 * @value:	Value to set the feature to
 * @cdw12:	Feature specific command dword12 field
 * @save:	Save value across power states
 * @data_len:	Length of feature data, if applicable, in bytes
 * @data:	User address of feature data, if applicable
 * @result:	The command completion result from CQE dword0
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_set_feature(int fd, __u32 nsid, __u8 fid, __u32 value, __u32 cdw12,
		     bool save, __u32 data_len, void *data, __u32 *result);

/**
 * nvme_get_feature() - Retrieve a feature attribute
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID, if applicable
 * @fid:	Feature identifier
 * @sel:	Select which type of attribute to return
 * @cdw11:	Feature specific command dword11 field
 * @data_len:	Length of feature data, if applicable, in bytes
 * @data:	User address of feature data, if applicable
 * @result:	The command completion result from CQE dword0
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_feature(int fd, __u32 nsid, __u8 fid, __u8 sel,
		     __u32 cdw11, __u32 data_len, void *data, __u32 *result);

/**
 * nvme_format() - Format nvme namespace(s)
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to format
 * @lbaf:	Logical block address format
 * @ses:	Secure erase settings
 * @pi:		Protection infomration type
 * @pil:	Protection information location (beginning or end)
 * @ms:		Metadata settings (extended or separated)
 * @timeout:	Set to override default timeout to this value in milliseconds;
 * 		useful for long running formats, or clear to 0 for the default
 *
 * The Format NVM command is used to low level format the NVM media. This
 * command is used by the host to change the LBA data size and/or metadata
 * size. A low level format may destroy all data and metadata associated with
 * all namespaces or only the specific namespace associated with the command
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_format(int fd, __u32 nsid, __u8 lbaf, __u8 ses, __u8 pi,
		__u8 pil, __u8 ms, __u32 timeout);

/**
 * nvme_ns_create() - Create a namespace
 * @fd:		File descriptor of nvme device
 * @nsze:	Namespace size
 * @ncap:	Namespace capacity
 * @flbas:	Format with thes logical block settings
 * @dps:	Data protection settings
 * @nmic:	Namespace mulit-pathing capable
 * @anagrpid:	Asynchronous namespace group identifier
 * @nvmesetid: 	NVM Set identifier
 * @timeout:	Override the default timeout to this value in milliseconds;
 * 		clear to 0 to use the default
 * @result:	On success, the new namespace id that was created
 *
 * On successful creation, the namespace exists in the subsystem, but is not
 * attached to any controller. Use the nvme_ns_attach_ctrls() to assign the
 * namespace to one or more controllers.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_ns_create(int fd, __u64 nsze, __u64 ncap, __u8 flbas, __u8 dps,
		   __u8 nmic, __u32 anagrpid, __u16 nvmsetid,  __u32 timeout,
		   __u32 *result);

/**
 * nvme_ns_delete() - Delete namespace from subsystem
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to delete
 * @timeout:	Overide the default timeout to this value in milliseconds;
 * 		clear to 0 to use the default
 *
 * It is recommended that a namespace being deleted is not attached to any
 * controller. Use the nvme_ns_detach_ctrls() first if the namespace is still
 * attached.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_ns_delete(int fd, __u32 nsid, __u32 timeout);


/**
 * nvme_ns_attach_ctrls() - Attach namespace to controller(s)
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to attach
 * @num_ctrls:	Number of controllers in ctrlist
 * @ctrlist:	List of controller IDs to perform the attach action
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_ns_attach_ctrls(int fd, __u32 nsid, __u16 num_ctrls, __u16 *ctrlist);

/**
 * nvme_ns_detach_ctrls() - Detach namespace from controller(s)
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to detach
 * @num_ctrls:	Number of controllers in ctrlist
 * @ctrlist:	List of controller IDs to perform the detach action
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_ns_detach_ctrls(int fd, __u32 nsid, __u16 num_ctrls, __u16 *ctrlist);

/**
 * nvme_fw_download() - Download firmware to the controller
 * @fd:		File descriptor of nvme device
 * @offset:	Offset in the firmware data
 * @data_len:	Length of data in this command in bytes
 * @data:	Userspace address of the firmware data
 *
 * The Firmware Image Download command is used to download all or a portion of
 * an image for a future update to the controller. The Firmware Image Download
 * command downloads a new image (in whole or in part) to the controller.
 *
 * The image may be constructed of multiple pieces that are individually
 * downloaded with separate Firmware Image Download commands. Each Firmware
 * Image Download command includes a Dword Offset and Number of Dwords that
 * specify a dword range.
 *
 * The new firmware image is not activated as part of the Firmware Image
 * Download command. Use the nvme_fw_commit() to activate a newly downloaded
 * image.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_fw_download(int fd, __u32 offset, __u32 data_len, void *data);

/**
 * nvme_fw_commit() - Commit firmware using the specified action
 * @fd:		File descriptor of nvme device
 * @slot:	Firmware slot to commit the downloaded image
 * @action:	Action to use for the firmware image
 * @bpid:	Boot partition ID, if applicable
 *
 * The Firmware Commit command is used to modify the firmware image or Boot
 * Partitions.
 *
 * Refer to the NVMe Specification for valid commit actions.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise. The command status response may specify additional reset
 * 	   actions required to complete the commit process.
 */
int nvme_fw_commit(int fd, __u8 slot, __u8 action, __u8 bpid);

/**
 * nvme_sec_send() -
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to issue security command on
 * @nssf:	NVMe Security Specific field
 * @spsp:	Security Protocol Specific field
 * @secp:	Security Protocol
 * @tl:		Protocol specific transfer length
 * @data_len:	Data length of the payload in bytes
 * @data:	Security data payload to send
 * @result:	The command completion result from CQE dword0
 *
 * The Security Send command is used to transfer security protocol data to the
 * controller. The data structure transferred to the controller as part of this
 * command contains security protocol specific commands to be performed by the
 * controller. The data structure transferred may also contain data or
 * parameters associated with the security protocol commands.
 *
 * The security data is protocol specific and is not defined by the NVMe
 * specification.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_sec_send(int fd, __u32 nsid, __u8 nssf, __u16 spsp,
		  __u8 secp, __u32 tl, __u32 data_len, void *data,
		  __u32 *result);

/**
 * nvme_sec_recv() -
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to issue security command on
 * @nssf:	NVMe Security Specific field
 * @spsp:	Security Protocol Specific field
 * @secp:	Security Protocol
 * @al:		Protocol specific allocation length
 * @data_len:	Data length of the payload in bytes
 * @data:	Security data payload to recieve
 * @result:	The command completion result from CQE dword0
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_sec_recv(int fd, __u32 nsid, __u8 nssf, __u16 spsp,
		  __u8 secp, __u32 al, __u32 data_len, void *data,
		  __u32 *result);

/**
 * nvme_get_lba_status() - Retrieve information on possibly unrecoverable LBAs
 * @fd:		File descriptor of nvme device
 * @slba:	Starting logical block address to check statuses
 * @mndw:	Maximum number of dwords to return
 * @atype:	Action type mechanism to determine LBA status desctriptors to
 * 		return
 * @rl:		Range length from slba to perform the action
 * @lbas:	Data payload to return status descriptors
 *
 * The Get LBA Status command requests information about Potentially
 * Unrecoverable LBAs. Refer to the specification for action type descriptions.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_lba_status(int fd, __u64 slba, __u32 mndw, __u8 atype, __u16 rl,
			struct nvme_lba_status *lbas);

/**
 * nvme_dir_send() - Send directive command
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID, if applicable
 * @dspec:	Directive specific field
 * @dtype:	Directive type
 * @doper:	Directive operation
 * @dw12:	Directive specific command dword12
 * @data_len:	Length of data payload in bytes
 * @data:	Usespace address of data payload
 * @result:	If successful, the CQE dword0 value
 *
 * Directives is a mechanism to enable host and NVM subsystem or controller
 * information exchange. The Directive Send command is used to transfer data
 * related to a specific Directive Type from the host to the controller.
 *
 * See the NVMe specification for more information.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_dir_send(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
		  __u32 dw12, __u32 data_len, void *data, __u32 *result);

/**
 * nvme_dir_recv() - Receive directive specific data
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID, if applicable
 * @dspec:	Directive specific field
 * @dtype:	Directive type
 * @doper:	Directive operation
 * @dw12:	Directive specific command dword12
 * @data_len:	Length of data payload
 * @data:	Usespace address of data payload in bytes
 * @result:	If successful, the CQE dword0 value
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_dir_recv(int fd, __u32 nsid, __u16 dspec, __u8 dtype, __u8 doper,
		  __u32 dw12, __u32 data_len, void *data, __u32 *result);

/**
 * nvme_set_property() - Set controller property
 * @fd:		File descriptor of nvme device
 * @offset:	Property offset from the base to set
 * @value:	The value to set the property
 *
 * This is an NVMe-over-Fabrics specific command, not applicable to PCIe. These
 * properties align to the PCI MMIO controller registers.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_set_property(int fd, int offset, uint64_t value);

/**
 * nvme_get_property() - Get a controller property
 * @fd:		File descriptor of nvme device
 * @offset:	Property offset from the base to retrieve
 * @value:	Where the property's value will be stored on success
 *
 * This is an NVMe-over-Fabrics specific command, not applicable to PCIe. These
 * properties align to the PCI MMIO controller registers.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_get_property(int fd, int offset, uint64_t *value);

/**
 * nvme_sanitize() - Start a sanitize operation
 * @fd:		File descriptor of nvme device
 * @sanact:	Sanitize action
 * @ause:	Allow unrestriced sanitize exit
 * @owpass:	Overwrite pass count
 * @oipbp:	Overwrite invert pattern between passes
 * @no_dealloc:	If true, don't deallocate blocks after sanitizing
 * @ovrpat:	Overwrite pattern
 *
 * A sanitize operation alters all user data in the NVM subsystem such that
 * recovery of any previous user data from any cache, the non-volatile media,
 * or any Controller Memory Buffer is not possible.
 *
 * The Sanitize command is used to start a sanitize operation or to recover
 * from a previously failed sanitize operation. The sanitize operation types
 * that may be supported are Block Erase, Crypto Erase, and Overwrite. All
 * sanitize operations are processed in the background (i.e., completion of the
 * Sanitize command does not indicate completion of the sanitize operation).
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_sanitize(int fd, __u8 sanact, __u8 ause, __u8 owpass, __u8 oipbp,
		  bool no_dealloc, __u32 ovrpat);

/**
 * nvme_self_test_start() - Start or abort a selft test
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID to test
 * @stc:	Self test code
 *
 * The Device Self-test command is used to start a device self-test operation
 * or abort a device self-test operation. A device self-test operation is a
 * diagnostic testing sequence that tests the integrity and functionality of
 * the controller and may include testing of the media associated with
 * namespaces. The controller may return a response to this command immediately
 * while running the self-test in the background.
 *
 * Set the 'nsid' field to 0 to not include namepsaces in the test. Set to
 * 0xffffffff to test all namespaces. All other values tests a specific
 * namespace, if present.
 *
 * The 'stc' field provides short and extended tests, among others. See the
 * NVMe specification for additional details.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_self_test_start(int fd, __u32 nsid, __u32 stc);

/**
 * nvme_self_test_log() - Retrieve the device self test log
 * @fd:		File descriptor of nvme device
 * @nsid:	Namespace ID being tested
 * @log:	Userspace address of the log payload
 *
 * The log page is used to indicate the status of an in progress self test and
 * the percent complete of that operation, and the results of the previous 20
 * self-test operations.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_self_test_log(int fd, __u32 nsid,
		       struct nvme_self_test_log *log);

/**
 * nvme_virtual_mgmt() -
 * @fd:		File descriptor of nvme device
 * @act:	Virtual resource action
 * @rt:		Resource type to modify
 * @cntlid:	Controller id for which resources are bing modified
 * @nr:		Number of resources being allocated or assigned
 * @result:	If successful, the CQE dword0
 *
 * The Virtualization Management command is supported by primary controllers
 * that support the Virtualization Enhancements capability. This command is
 * used for several functions:
 *
 *	Modifying Flexible Resource allocation for the primary controller
 *	Assigning Flexible Resources for secondary controllers
 *	Setting the Online and Offline state for secondary controllers.
 *
 * Return: The nvme command status if a response was received or -errno
 * 	   otherwise.
 */
int nvme_virtual_mgmt(int fd, __u8 act, __u8 rt, __u16 cntlid, __u16 nr,
		      __u32 *result);

/**
 * nvme_subsystem_reset() - Initiate a subsystem reset
 * @fd:		File descriptor of nvme device
 *
 * Return: Zero if a subsystem reset was initiated or -errno otherwise.
 */
int nvme_subsystem_reset(int fd);

/**
 * nvme_reset_controller() - Initiate a controller reset
 * @fd:		File descriptor of nvme device
 *
 * Return: Zero if a reset was initiated or -errno otherwise.
 */
int nvme_reset_controller(int fd);

/**
 * nvme_ns_rescan() - Initiate a controller rescan
 * @fd:		File descriptor of nvme device
 *
 * Return: Zero if a rescan was initiated or -errno otherwise.
 */
int nvme_ns_rescan(int fd);

/**
 * nvme_get_nsid() - Retrieve the NSID from a namespace file descriptor
 * @fd:		File descriptor of nvme namespace
 *
 * Return: The namespace identifier if a succecssful or -errno otherwise.
 */
int nvme_get_nsid(int fd);

#endif /* _LINUX_NVME_IOCTL */

