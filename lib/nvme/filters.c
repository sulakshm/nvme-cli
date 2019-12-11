#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "filters.h"

/* global, used for controller specific namespace filter */
static int current_index;

const char *dev = "/dev/";
const char *sys_nvme = "/sys/class/nvme";
const char *subsys_dir = "/sys/class/nvme-subsystem/";

static int nvme_scan_dev_filter(const struct dirent *d)
{
	int ctrl, ns, part;

	if (d->d_name[0] == '.')
		return 0;

	if (strstr(d->d_name, "nvme")) {
		if (sscanf(d->d_name, "nvme%dn%dp%d", &ctrl, &ns, &part) == 3)
			return 0;
		if (sscanf(d->d_name, "nvme%dn%d", &ctrl, &ns) == 2)
			return ctrl == current_index;
	}
	return 0;
}

int nvme_scan_dev_instance_namespaces(struct dirent ***d, int instance)
{
	current_index = instance;
	return scandir(dev, d, nvme_scan_dev_filter, alphasort);
}

int nvme_scan_namespace_filter(const struct dirent *d)
{
	int i, n;

	if (d->d_name[0] == '.')
		return 0;

	if (strstr(d->d_name, "nvme")) {
		if (sscanf(d->d_name, "nvme%dn%d", &i, &n) == 2)
			return 1;
	}

	return 0;
}

int nvme_scan_paths_filter(const struct dirent *d)
{
	int id, cntlid, nsid;

	if (d->d_name[0] == '.')
		return 0;

	if (strstr(d->d_name, "nvme")) {
		if (sscanf(d->d_name, "nvme%dc%dn%d", &id, &cntlid, &nsid) == 3)
			return 1;
		if (sscanf(d->d_name, "nvme%dn%d", &id, &nsid) == 2)
			return 1;
	}

	return 0;
}

int nvme_scan_ctrls_filter(const struct dirent *d)
{
	int id, nsid;

	if (d->d_name[0] == '.')
		return 0;

	if (strstr(d->d_name, "nvme")) {
		if (sscanf(d->d_name, "nvme%dn%d", &id, &nsid) == 2)
			return 0;
		if (sscanf(d->d_name, "nvme%d", &id) == 1)
			return 1;
		return 0;
	}

	return 0;
}

int nvme_scan_subsys_filter(const struct dirent *d)
{
	int id;

	if (d->d_name[0] == '.')
		return 0;

	if (strstr(d->d_name, "nvme-subsys")) {
		if (sscanf(d->d_name, "nvme-subsys%d", &id) != 1)
			return 0;
		return 1;
	}

	return 0;
}
