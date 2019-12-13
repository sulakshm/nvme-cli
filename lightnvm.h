/*
 * Copyright (C) 2016 CNEX Labs.  All rights reserved.
 *
 * Author: Matias Bjoerling <matias@cnexlabs.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 */

#ifndef NVME_LIGHTNVM_H_
#define NVME_LIGHTNVM_H_

#ifndef _UAPI_LINUX_LIGHTNVM_H
#define _UAPI_LINUX_LIGHTNVM_H

#include <stdio.h>
#include <sys/ioctl.h>
#define DISK_NAME_LEN 32

#include <linux/types.h>
#include <linux/ioctl.h>

#define NVM_TTYPE_NAME_MAX 48
#define NVM_TTYPE_MAX 63
#define NVM_MMTYPE_LEN 8

#define NVM_CTRL_FILE "/dev/lightnvm/control"

struct nvm_ioctl_info_tgt {
	__u32 version[3];
	__u32 reserved;
	char tgtname[NVM_TTYPE_NAME_MAX];
};

struct nvm_ioctl_info {
	__u32 version[3];	/* in/out - major, minor, patch */
	__u16 tgtsize;		/* number of targets */
	__u16 reserved16;	/* pad to 4K page */
	__u32 reserved[12];
	struct nvm_ioctl_info_tgt tgts[NVM_TTYPE_MAX];
};

enum {
	NVM_DEVICE_ACTIVE = 1 << 0,
};

struct nvm_ioctl_device_info {
	char devname[DISK_NAME_LEN];
	char bmname[NVM_TTYPE_NAME_MAX];
	__u32 bmversion[3];
	__u32 flags;
	__u32 reserved[8];
};

struct nvm_ioctl_get_devices {
	__u32 nr_devices;
	__u32 reserved[31];
	struct nvm_ioctl_device_info info[31];
};

struct nvm_ioctl_create_simple {
	__u32 lun_begin;
	__u32 lun_end;
};

struct nvm_ioctl_create_extended {
	__u16 lun_begin;
	__u16 lun_end;
	__u16 over_prov;
	__u16 rsv;
};

enum {
	NVM_CONFIG_TYPE_SIMPLE = 0,
	NVM_CONFIG_TYPE_EXTENDED = 1,
};

struct nvm_ioctl_create_conf {
	__u32 type;
	union {
		struct nvm_ioctl_create_simple s;
		struct nvm_ioctl_create_extended e;
	};
};

enum {
	NVM_TARGET_FACTORY = 1 << 0,	/* Init target in factory mode */
};

struct nvm_ioctl_create {
	char dev[DISK_NAME_LEN];		/* open-channel SSD device */
	char tgttype[NVM_TTYPE_NAME_MAX];	/* target type name */
	char tgtname[DISK_NAME_LEN];		/* dev to expose target as */

	__u32 flags;

	struct nvm_ioctl_create_conf conf;
};

struct nvm_ioctl_remove {
	char tgtname[DISK_NAME_LEN];

	__u32 flags;
};

struct nvm_ioctl_dev_init {
	char dev[DISK_NAME_LEN];		/* open-channel SSD device */
	char mmtype[NVM_MMTYPE_LEN];		/* register to media manager */

	__u32 flags;
};

enum {
	NVM_FACTORY_ERASE_ONLY_USER	= 1 << 0, /* erase only blocks used as
						   * host blks or grown blks */
	NVM_FACTORY_RESET_HOST_BLKS	= 1 << 1, /* remove host blk marks */
	NVM_FACTORY_RESET_GRWN_BBLKS	= 1 << 2, /* remove grown blk marks */
	NVM_FACTORY_NR_BITS		= 1 << 3, /* stops here */
};

struct nvm_ioctl_dev_factory {
	char dev[DISK_NAME_LEN];

	__u32 flags;
};

/* The ioctl type, 'L', 0x20 - 0x2F documented in ioctl-number.txt */
enum {
	/* top level cmds */
	NVM_INFO_CMD = 0x20,
	NVM_GET_DEVICES_CMD,

	/* device level cmds */
	NVM_DEV_CREATE_CMD,
	NVM_DEV_REMOVE_CMD,

	/* Init a device to support LightNVM media managers */
	NVM_DEV_INIT_CMD,

	/* Factory reset device */
	NVM_DEV_FACTORY_CMD,
};

#define NVM_IOCTL 'L' /* 0x4c */

#define NVM_INFO		_IOWR(NVM_IOCTL, NVM_INFO_CMD, \
						struct nvm_ioctl_info)
#define NVM_GET_DEVICES		_IOR(NVM_IOCTL, NVM_GET_DEVICES_CMD, \
						struct nvm_ioctl_get_devices)
#define NVM_DEV_CREATE		_IOW(NVM_IOCTL, NVM_DEV_CREATE_CMD, \
						struct nvm_ioctl_create)
#define NVM_DEV_REMOVE		_IOW(NVM_IOCTL, NVM_DEV_REMOVE_CMD, \
						struct nvm_ioctl_remove)
#define NVM_DEV_INIT		_IOW(NVM_IOCTL, NVM_DEV_INIT_CMD, \
						struct nvm_ioctl_dev_init)
#define NVM_DEV_FACTORY		_IOW(NVM_IOCTL, NVM_DEV_FACTORY_CMD, \
						struct nvm_ioctl_dev_factory)

#define NVM_VERSION_MAJOR	1
#define NVM_VERSION_MINOR	0
#define NVM_VERSION_PATCHLEVEL	0

#endif

enum nvme_nvm_admin_opcode {
	nvme_nvm_admin_identity		= 0xe2,
	nvme_nvm_admin_get_bb_tbl	= 0xf2,
	nvme_nvm_admin_set_bb_tbl	= 0xf1,
};

struct nvme_nvm_identity {
	__u8	opcode;
	__u8	flags;
	__le16	command_id;
	__le32	nsid;
	__le64	rsvd[2];
	__le64	prp1;
	__le64	prp2;
	__le32	chnl_off;
	__le32	rsvd11[5];
};

struct nvme_nvm_setbbtbl {
	__u8	opcode;
	__u8	flags;
	__le16	rsvd1;
	__le32	nsid;
	__le32	cdw2;
	__le32	cdw3;
	__le64	metadata;
	__u64	addr;
	__le32	metadata_len;
	__le32	data_len;
	__le64	ppa;
	__le16	nlb;
	__u8	value;
	__u8	rsvd2;
	__le32	cdw14;
	__le32	cdw15;
	__le32	timeout_ms;
	__le32	result;
};

struct nvme_nvm_getbbtbl {
	__u8	opcode;
	__u8	flags;
	__le16	rsvd1;
	__le32	nsid;
	__le32	cdw2;
	__le32	cdw3;
	__u64	metadata;
	__u64	addr;
	__u32	metadata_len;
	__u32	data_len;
	__le64	ppa;
	__le32	cdw12;
	__le32	cdw13;
	__le32	cdw14;
	__le32	cdw15;
	__le32	timeout_ms;
	__le32	result;
};

struct nvme_nvm_command {
	union {
		struct nvme_nvm_identity identity;
		struct nvme_nvm_getbbtbl get_bb;
	};
};

struct nvme_nvm_completion {
	__le64	result;		/* Used by LightNVM to return ppa completions */
	__le16	sq_head;	/* how much of this queue may be reclaimed */
	__le16	sq_id;		/* submission queue that generated this entry */
	__le16	command_id;	/* of the command which completed */
	__le16	status;		/* did the command fail, and if so, why? */
};

#define NVME_NVM_LP_MLC_PAIRS 886
struct nvme_nvm_lp_mlc {
	__le16			num_pairs;
	__u8			pairs[NVME_NVM_LP_MLC_PAIRS];
};

struct nvme_nvm_lp_tbl {
	__u8			id[8];
	struct nvme_nvm_lp_mlc	mlc;
};

struct nvme_nvm_id12_group {
	__u8			mtype;
	__u8			fmtype;
	__le16			res16;
	__u8			num_ch;
	__u8			num_lun;
	__u8			num_pln;
	__u8			rsvd1;
	__le16			num_blk;
	__le16			num_pg;
	__le16			fpg_sz;
	__le16			csecs;
	__le16			sos;
	__le16			rsvd2;
	__le32			trdt;
	__le32			trdm;
	__le32			tprt;
	__le32			tprm;
	__le32			tbet;
	__le32			tbem;
	__le32			mpos;
	__le32			mccap;
	__le16			cpar;
	__u8			reserved[10];
	struct nvme_nvm_lp_tbl lptbl;
} __attribute__((packed));

struct nvme_nvm_addr_format {
	__u8			ch_offset;
	__u8			ch_len;
	__u8			lun_offset;
	__u8			lun_len;
	__u8			pln_offset;
	__u8			pln_len;
	__u8			blk_offset;
	__u8			blk_len;
	__u8			pg_offset;
	__u8			pg_len;
	__u8			sect_offset;
	__u8			sect_len;
	__u8			res[4];
} __attribute__((packed));

enum {
	LNVM_IDFY_CAP_BAD_BLK_TBL_MGMT	= 0,
	LNVM_IDFY_CAP_HYBRID_CMD_SUPP	= 1,
	LNVM_IDFY_CAP_VCOPY		= 0,
	LNVM_IDFY_CAP_MRESETS		= 1,
	LNVM_IDFY_DOM_HYBRID_MODE	= 0,
	LNVM_IDFY_DOM_ECC_MODE		= 1,
	LNVM_IDFY_GRP_MTYPE_NAND	= 0,
	LNVM_IDFY_GRP_FMTYPE_SLC	= 0,
	LNVM_IDFY_GRP_FMTYPE_MLC	= 1,
	LNVM_IDFY_GRP_FMTYPE_TLC	= 2,
	LNVM_IDFY_GRP_MPOS_SNGL_PLN_RD	= 0,
	LNVM_IDFY_GRP_MPOS_DUAL_PLN_RD	= 1,
	LNVM_IDFY_GRP_MPOS_QUAD_PLN_RD	= 2,
	LNVM_IDFY_GRP_MPOS_SNGL_PLN_PRG	= 8,
	LNVM_IDFY_GRP_MPOS_DUAL_PLN_PRG	= 9,
	LNVM_IDFY_GRP_MPOS_QUAD_PLN_PRG	= 10,
	LNVM_IDFY_GRP_MPOS_SNGL_PLN_ERS	= 16,
	LNVM_IDFY_GRP_MPOS_DUAL_PLN_ERS	= 17,
	LNVM_IDFY_GRP_MPOS_QUAD_PLN_ERS	= 18,
	LNVM_IDFY_GRP_MCCAP_SLC		= 0,
	LNVM_IDFY_GRP_MCCAP_CMD_SUSP	= 1,
	LNVM_IDFY_GRP_MCCAP_SCRAMBLE	= 2,
	LNVM_IDFY_GRP_MCCAP_ENCRYPT	= 3,
};

struct nvme_nvm_id12 {
	__u8			ver_id;
	__u8			vmnt;
	__u8			cgrps;
	__u8			res;
	__le32			cap;
	__le32			dom;
	struct nvme_nvm_addr_format ppaf;
	__u8			resv[228];
	struct nvme_nvm_id12_group groups[4];
} __attribute__((packed));

struct nvme_nvm_id20_addrf {
	__u8			grp_len;
	__u8			pu_len;
	__u8			chk_len;
	__u8			lba_len;
	__u8			resv[4];
} __attribute__((packed));

struct nvme_nvm_id20 {
	__u8			mjr;
	__u8			mnr;
	__u8			resv[6];

	struct nvme_nvm_id20_addrf lbaf;

	__le32			mccap;
	__u8			resv2[12];

	__u8			wit;
	__u8			resv3[31];

	/* Geometry */
	__le16			num_grp;
	__le16			num_pu;
	__le32			num_chk;
	__le32			clba;
	__u8			resv4[52];

	/* Write data requirements */
	__le32			ws_min;
	__le32			ws_opt;
	__le32			mw_cunits;
	__le32			maxoc;
	__le32			maxocpu;
	__u8			resv5[44];

	/* Performance related metrics */
	__le32			trdt;
	__le32			trdm;
	__le32			twrt;
	__le32			twrm;
	__le32			tcrst;
	__le32			tcrsm;
	__u8			resv6[40];

	/* Reserved area */
	__u8			resv7[2816];

	/* Vendor specific */
	__u8			vs[1024];
} __attribute__((packed));

struct nvme_nvm_id {
	__u8			ver_id;
	__u8			resv[4095];
} __attribute__((packed));

enum {
	NVM_LID_CHUNK_INFO = 0xCA,
};

struct nvme_nvm_chunk_desc {
	__u8	cs;
	__u8	ct;
	__u8	wli;
	__u8	rsvd_7_3[5];
	__u64	slba;
	__u64	cnlb;
	__u64	wp;
};

struct nvme_nvm_bb_tbl {
	__u8	tblid[4];
	__le16	verid;
	__le16	revid;
	__le32	rvsd1;
	__le32	tblks;
	__le32	tfact;
	__le32	tgrown;
	__le32	tdresv;
	__le32	thresv;
	__le32	rsvd2[8];
	__u8	blk[0];
};

#define NVM_BLK_BITS (16)
#define NVM_PG_BITS  (16)
#define NVM_SEC_BITS (8)
#define NVM_PL_BITS  (8)
#define NVM_LUN_BITS (8)
#define NVM_CH_BITS  (7)

struct ppa_addr {
	/* Generic structure for all addresses */
	union {
		struct {
			__u64 blk	: NVM_BLK_BITS;
			__u64 pg	: NVM_PG_BITS;
			__u64 sec	: NVM_SEC_BITS;
			__u64 pl	: NVM_PL_BITS;
			__u64 lun	: NVM_LUN_BITS;
			__u64 ch	: NVM_CH_BITS;
			__u64 reserved	: 1;
		} g;

		__u64 ppa;
	};
};

static inline struct ppa_addr generic_to_dev_addr(
			struct nvme_nvm_addr_format *ppaf, struct ppa_addr r)
{
	struct ppa_addr l;

	l.ppa = ((__u64)r.g.blk) << ppaf->blk_offset;
	l.ppa |= ((__u64)r.g.pg) << ppaf->pg_offset;
	l.ppa |= ((__u64)r.g.sec) << ppaf->sect_offset;
	l.ppa |= ((__u64)r.g.pl) << ppaf->pln_offset;
	l.ppa |= ((__u64)r.g.lun) << ppaf->lun_offset;
	l.ppa |= ((__u64)r.g.ch) << ppaf->ch_offset;

	return l;
}

int lnvm_get_identity(int fd, int nsid, struct nvme_nvm_id *nvm_id);

int lnvm_do_init(char *, char *);
int lnvm_do_list_devices(void);
int lnvm_do_info(void);
int lnvm_do_create_tgt(char *, char *, char *, int, int, int, int);
int lnvm_do_remove_tgt(char *);
int lnvm_do_factory_init(char *, int, int, int);
int lnvm_do_id_ns(int, int, unsigned int);
int lnvm_do_chunk_log(int, __u32, __u32, void *, unsigned int);
int lnvm_do_get_bbtbl(int, int, int, int, unsigned int);
int lnvm_do_set_bbtbl(int, int, int, int, int, int, __u8);

#endif
