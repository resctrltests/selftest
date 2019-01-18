/* SPDX-License-Identifier: GPL-2.0 */
#define _GNU_SOURCE
#ifndef RESCTRL_H
#define RESCTRL_H
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/select.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/time.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/eventfd.h>

#define MB			(1024 * 1024)
#define RESCTRL_PATH		"/sys/fs/resctrl"
#define PHYS_ID_PATH		"/sys/devices/system/cpu/cpu"
#define CBM_MASK_PATH		"/sys/fs/resctrl/info"

#define PARENT_EXIT(err_msg)			\
	do {					\
		perror(err_msg);		\
		kill(ppid, SIGKILL);		\
		exit(EXIT_FAILURE);		\
	} while (0)

/*
 * resctrl_val_param:	resctrl test parameters
 * @resctrl_val:	Resctrl feature (Eg: mbm, mba.. etc)
 * @ctrlgrp:		Name of the control monitor group (con_mon grp)
 * @mongrp:		Name of the monitor group (mon grp)
 * @cpu_no:		CPU number to which the benchmark would be binded
 * @span:		Memory bytes accessed in each benchmark iteration
 * @mum_resctrlfs:	Should the resctrl FS be remounted?
 * @filename:		Name of file to which the o/p should be written
 * @bw_report:		Bandwidth report type (reads vs writes)
 * @setup:		Call back function to setup test environment
 */
struct resctrl_val_param {
	char	*resctrl_val;
	char	ctrlgrp[64];
	char	mongrp[64];
	int	cpu_no;
	unsigned long long span;
	int	mum_resctrlfs;
	char	filename[64];
	char	*bw_report;
	unsigned long mask;
	int	num_of_runs;
	int	(*setup)(int num, ...);

};

/**
 * Results of CPUID operation are stored in this structure.
 * It consists of 4x32bits IA registers: EAX, EBX, ECX and EDX.
 */
struct cpuid_out {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
};

pid_t bm_pid, ppid;
extern char cbm_mask[256];
extern unsigned long long_mask;
extern char llc_occup_path[1024];
extern int genuine_intel;
extern int authentic_amd;

int remount_resctrlfs(bool mum_resctrlfs);
int get_resource_id(int cpu_no, int *resource_id);
int umount_resctrlfs(void);
int validate_bw_report_request(char *bw_report);
int validate_resctrl_feature_request(char *resctrl_val);
int taskset_benchmark(pid_t bm_pid, int cpu_no);
void run_benchmark(int signum, siginfo_t *info, void *ucontext);
int write_schemata(char *ctrlgrp, char *schemata, int cpu_no,
		   char *resctrl_val);
int write_bm_pid_to_resctrl(pid_t bm_pid, char *ctrlgrp, char *mongrp,
			    char *resctrl_val);
int perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu,
		    int group_fd, unsigned long flags);
int run_fill_buf(unsigned long span, int malloc_and_init_memory, int memflush,
		 int op, char *resctrl_va);
int resctrl_val(char **benchmark_cmd, struct resctrl_val_param *param);
int mbm_bw_change(int span, int cpu_no, char *bw_report, char **benchmark_cmd);
void tests_cleanup(void);
void mbm_test_cleanup(void);
int mba_schemata_change(int cpu_no, char *bw_report, char **benchmark_cmd);
void mba_test_cleanup(void);
int get_cbm_mask(char *cache_type);
int get_cache_size(int cpu_no, char *cache_type, unsigned long *cache_size);
void ctrlc_handler(int signum, siginfo_t *info, void *ptr);
int cat_val(struct resctrl_val_param *param);
void cat_test_cleanup(void);
int cat_perf_miss_val(int cpu_no, int no_of_bits, char *cache_type);
int cqm_resctrl_val(int cpu_no, int n, char **benchmark_cmd);
unsigned int count_bits(unsigned long n);
void cqm_test_cleanup(void);
int get_core_sibling(int cpu_no);
int measure_cache_vals(struct resctrl_val_param *param, int bm_pid);

#endif /* RESCTRL_H */
