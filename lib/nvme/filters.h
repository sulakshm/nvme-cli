#ifndef _NVME_FILTERS
#define _NVME_FILTERS

#include <dirent.h>

/**
 * char *dev: top directory of nvme device nodes (/dev/)
 */
extern const char *dev;

/**
 * char *sys_nvme: directory for nvme sysfs
 */
extern const char *sys_nvme;

/**
 * char *subsys_dir: directory for nvme subsystem sysfs
 */
extern const char *subsys_dir;

/**
 * nvme_scan_namespace_filter() - nvme namespace filter
 *
 * scandir filter callback to return device nodes that match the nvmeXnY
 * pattern.
 */
int nvme_scan_namespace_filter(const struct dirent *d);

/**
 * nvme_scan_paths_filter() - nvme paths filter
 *
 * scandir filter callback to return sysfs directories of namespace paths.
 */
int nvme_scan_paths_filter(const struct dirent *d);

/**
 * nvme_scan_ctrls_filter() - nvme controller filter
 *
 * scandir filter callback to return controller files but not namespaces.
 */
int nvme_scan_ctrls_filter(const struct dirent *d);

/**
 * nvme_scan_subsys_filter() - nvme subssytem filter
 *
 * scandir filter callback to return nvme subsystem sysfs directories.
 */
int nvme_scan_subsys_filter(const struct dirent *d);

/**
 * nvme_scan_dev_instance_namespaces() - nvme namespace filter for devices
 *
 * Finds namespaces nodes with a name prefix that mathes the given instance.
 */
int nvme_scan_dev_instance_namespaces(struct dirent ***d, int instance);

#endif /* _NVME_FILTERS */
