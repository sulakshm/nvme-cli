#ifndef _NVME_FABRICS_H
#define _NVME_FABRICS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define BUF_SIZE		4096

struct fabrics_config {
	char *nqn;
	char *transport;
	char *traddr;
	char *trsvcid;
	char *host_traddr;
	char *hostnqn;
	char *hostid;
	char *device;
	int  nr_io_queues;
	int  nr_write_queues;
	int  nr_poll_queues;
	int  queue_size;
	int  keep_alive_tmo;
	int  reconnect_delay;
	int  ctrl_loss_tmo;
	int  tos;
	int  duplicate_connect;
	int  disable_sqflow;
	int  hdr_digest;
	int  data_digest;
};

int add_ctrl(const char *argstr);
int build_options(char **argstr, struct fabrics_config *cfg, bool discover);

#endif
