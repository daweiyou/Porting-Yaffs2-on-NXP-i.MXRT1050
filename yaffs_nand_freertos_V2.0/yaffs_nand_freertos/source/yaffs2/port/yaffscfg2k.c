/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg2k.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include "yaffs_osglue.h"
#include <errno.h>

#if (defined(ENABLE_YAFFS_TRACE)) && (ENABLE_YAFFS_TRACE == 1)
unsigned yaffs_trace_mask =

	YAFFS_TRACE_SCAN |
	YAFFS_TRACE_GC |
	YAFFS_TRACE_ERASE |
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_TRACING |
	YAFFS_TRACE_ALLOCATE |
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_VERIFY |
	0;
#else
unsigned yaffs_trace_mask = 0; //no yaffs log
#endif


/* Configure the devices that will be used */

#include "yaffs_flashif2.h"
//include "yaffs_m18_drv.h"
//#include "yaffs_nor_drv.h"
#include "yaffs_nand_drv.h"

/*
W29N01HV: 1G Bytes有效数据字节
138,412,032 bytes,
1024  blocks, each block = 135168 bytes = 64*(2048+64)
Each block consists of 64 programmable pages of 2,112-bytes(2048+64)

这个数据即可以手动配置，也可以在nand init的时候从nand参数表中读取
*/
#define BLOCKS_PER_NAND_FLASH 1024
#define PAGES_PER_BLOCK 64
#define DATA_BYTES_PER_PAGE 2048
#define SPARE_BYTES_PER_PAGE 64

struct nand_chip *nand_chip_init()
{
    //struct nand_chip *chip = malloc(sizeof(struct nand_chip));
    struct nand_chip *chip = yaffsfs_malloc(sizeof(struct nand_chip));
    
    memset(chip, 0, sizeof(struct nand_chip));;
    
    chip->private_data = 0;
    /*
    chip->set_ale = NULL;//nand_set_ale;
    chip->set_cle = NULL;//nand_set_cle;
    chip->read_cycle = NULL;//nand_read_cycle;
    chip->write_cycle = NULL;//nand_write_cycle;
    chip->check_busy = NULL;//nand_check_busy;
    chip->idle_fn = NULL;//nand_idle_fn;
    */
    chip->bus_width_shift = 0;
    
    chip->blocks = BLOCKS_PER_NAND_FLASH;
    chip->pages_per_block = PAGES_PER_BLOCK;
    chip->data_bytes_per_page = DATA_BYTES_PER_PAGE;
    chip->spare_bytes_per_page = SPARE_BYTES_PER_PAGE;
    
    return chip;
}
struct yaffs_dev *yaffs_nand_port_install_drv(const char *dev_name,
					const char *backing_file_name,
					int n_blocks)
{
	struct yaffs_dev *dev;
	char *name_copy = NULL;
	struct yaffs_param *param;
	struct nand_chip *chip = NULL;

	//dev = malloc(sizeof(struct yaffs_dev));
	dev = yaffsfs_malloc(sizeof(struct yaffs_dev));
	
	//name_copy = strdup(dev_name);//内部malloc分配并复制一个字符串
	name_copy = yaffsfs_malloc(strlen(dev_name)+1);
	memcpy(name_copy,dev_name,strlen(dev_name)+1);

	if(!dev || !name_copy)
		goto fail;

	memset(dev, 0, sizeof(*dev));
    
//	chip = nandsim_file_init(backing_file_name, n_blocks, 64, 2048, 64, 0);
//	if(!chip)
//		goto fail;
    
    chip = nand_chip_init();
    
	param = &dev->param;

	param->name = name_copy;

	param->total_bytes_per_chunk = chip->data_bytes_per_page;
	param->chunks_per_block = chip->pages_per_block;
	param->n_reserved_blocks = 5;
	param->start_block = 0; // First block
	param->end_block = n_blocks - 1; // Last block
	param->is_yaffs2 = 1;
	param->use_nand_ecc = 1;
	param->n_caches = 10;
	param->stored_endian = 2;

	if(yaffs_nand_install_drv(dev, chip) != YAFFS_OK)
		goto fail;

	/* The yaffs device has been configured, install it into yaffs */
	yaffs_add_device(dev);

	return dev;

fail:
	//free(dev);
	//free(name_copy);
	yaffsfs_free(dev);
	yaffsfs_free(name_copy);
	return NULL;
}

int yaffs_start_up(void)
{
	static int start_up_called = 0;

	if(start_up_called)
		return 0;
	start_up_called = 1;

	/* Call the OS initialisation (eg. set up lock semaphore */
	yaffsfs_OSInitialisation();

	/* Install the various devices and their device drivers */
	//yflash2_install_drv("yflash2");
	//yaffs_m18_install_drv("M18-1");
	//yaffs_nor_install_drv("nor");
	//yaffs_nandsim_install_drv("nand", "emfile-nand", 256);
    

    yaffs_nand_port_install_drv(NXP_YAFFS_NAME, "yaffs_file-nand", BLOCKS_PER_NAND_FLASH);
    
    
	return 0;
}



