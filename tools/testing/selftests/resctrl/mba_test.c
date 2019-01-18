// SPDX-License-Identifier: GPL-2.0
/*
 * Memory Bandwidth Allocation (MBA) test
 *
 * Copyright (C) 2018 Intel Corporation
 *
 * Authors:
 *    Arshiya Hayatkhan Pathan <arshiya.hayatkhan.pathan@intel.com>
 *    Sai Praneeth Prakhya <sai.praneeth.prakhya@intel.com>,
 *    Fenghua Yu <fenghua.yu@intel.com>
 */
#include "resctrl.h"

#define RESULT_FILE_NAME	"result_mba"
#define NUM_OF_RUNS		5
#define MAX_DIFF		300
#define ALLOCATION_MAX		100
#define ALLOCATION_MIN		10
#define ALLOCATION_STEP		10

/*
 * Change schemata percentage from 100 to 10%. Write schemata to specified
 * con_mon grp, mon_grp in resctrl FS.
 * For each allocation, run 5 times in order to get average values.
 */
static int mba_setup(int num, ...)
{
	static int runs_per_allocation, allocation = 100;
	struct resctrl_val_param *p;
	char allocation_str[64];
	va_list param;

	va_start(param, num);
	p = va_arg(param, struct resctrl_val_param *);
	va_end(param);

	if (runs_per_allocation >= NUM_OF_RUNS)
		runs_per_allocation = 0;

	/* Only set up schemata once every NUM_OF_RUNS of allocations */
	if (runs_per_allocation++ != 0)
		return 0;

	if (allocation < ALLOCATION_MIN || allocation > ALLOCATION_MAX)
		return -1;

	sprintf(allocation_str, "%d", allocation);

	write_schemata(p->ctrlgrp, allocation_str, p->cpu_no, p->resctrl_val);
	printf("changed schemata to : %d\n", allocation);
	allocation -= ALLOCATION_STEP;

	return 0;
}

static void show_mba_info(unsigned long *bw_imc, unsigned long *bw_resc)
{
	int allocation, failed = 0, runs;

	/* Memory bandwidth from 100% down to 10% */
	for (allocation = 0; allocation < ALLOCATION_MAX / ALLOCATION_STEP;
	     allocation++) {
		unsigned long avg_bw_imc = 0, avg_bw_resc = 0;
		unsigned long sum_bw_imc = 0, sum_bw_resc = 0;
		long  avg_diff = 0;

		/*
		 * The first run is discarded due to inaccurate value from
		 * phase transition.
		 */
		for (runs = NUM_OF_RUNS * allocation + 1;
		     runs < NUM_OF_RUNS * allocation + NUM_OF_RUNS ; runs++) {
			sum_bw_imc += bw_imc[runs];
			sum_bw_resc += bw_resc[runs];
		}

		avg_bw_imc = sum_bw_imc / (NUM_OF_RUNS - 1);
		avg_bw_resc = sum_bw_resc / (NUM_OF_RUNS - 1);
		avg_diff = avg_bw_resc - avg_bw_imc;

		printf("\nschemata percentage: %d \t",
		       ALLOCATION_MAX - ALLOCATION_STEP * allocation);
		printf("avg_bw_imc: %lu\t", avg_bw_imc);
		printf("avg_bw_resc: %lu\t", avg_bw_resc);
		printf("avg_diff: %lu\t", labs(avg_diff));
		if (labs(avg_diff) > MAX_DIFF) {
			printf("failed\n");
			failed = 1;
		} else {
			printf("passed\n");
		}
	}

	if (failed) {
		printf("\nTest for schemata change using MBA failed");
		printf("as atleast one test failed!\n");
	} else {
		printf("\nTests for changing schemata using MBA passed!\n\n");
	}
}

static int check_results(void)
{
	char *token_array[8], output[] = RESULT_FILE_NAME, temp[512];
	unsigned long bw_imc[1024], bw_resc[1024];
	int runs;
	FILE *fp;

	printf("\nchecking for pass/fail\n");
	fp = fopen(output, "r");
	if (!fp) {
		perror("Error in opening file\n");

		return errno;
	}

	runs = 0;
	while (fgets(temp, 1024, fp)) {
		char *token = strtok(temp, ":\t");
		int fields = 0;

		while (token) {
			token_array[fields++] = token;
			token = strtok(NULL, ":\t");
		}

		/* Field 3 is perf imc value */
		bw_imc[runs] = strtoul(token_array[3], NULL, 0);
		/* Field 5 is resctrl value */
		bw_resc[runs] = strtoul(token_array[5], NULL, 0);
		runs++;
	}

	fclose(fp);

	show_mba_info(bw_imc, bw_resc);

	return 0;
}

void mba_test_cleanup(void)
{
	remove(RESULT_FILE_NAME);
}

int mba_schemata_change(int cpu_no, char *bw_report, char **benchmark_cmd)
{
	struct resctrl_val_param param = {
		.resctrl_val	= "mba",
		.ctrlgrp	= "c1",
		.mongrp		= "m1",
		.cpu_no		= cpu_no,
		.mum_resctrlfs	= 1,
		.filename	= RESULT_FILE_NAME,
		.bw_report	= bw_report,
		.setup		= mba_setup
	};
	int ret;

	remove(RESULT_FILE_NAME);

	ret = validate_resctrl_feature_request("mba");
	if (ret)
		return ret;

	ret = resctrl_val(benchmark_cmd, &param);
	if (ret)
		return ret;

	ret = check_results();
	if (ret)
		return ret;

	mba_test_cleanup();

	return 0;
}
