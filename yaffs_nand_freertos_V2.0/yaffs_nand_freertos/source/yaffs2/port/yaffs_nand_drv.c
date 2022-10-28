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
 * This is an interface module for handling NAND in yaffs2 mode.
 */

/* This code calls a driver that accesses "generic" NAND. In the simulator
 * this is direceted at a file-backed NAND simulator. The NAND access functions
 * should also work with real NAND.
 *
 * This driver is designed for use in yaffs2 mode with a 2k page size and
 * 64 bytes of spare.
 *
 * The spare ares is used as follows:
 * offset 0: 2 bytes bad block marker.
 * offset 2: 8x3 bytes of ECC over the data.
 * offset 26: rest available to store Yaffs tags etc.
 */

#include "yaffs_nand_drv.h"
#include "yportenv.h"
#include "yaffs_trace.h"

#include "nand_store.h"
#include "yaffs_flashif.h"
#include "yaffs_guts.h"
#include "nanddrv.h"
#include "yaffs_ecc.h"

#include "bsp_nand.h"
#include "fsl_semc_nand_flash.h"
#include "fsl_debug_console.h"

static nand_handle_t nandHandle;

struct nand_context {
	struct nand_chip *chip;
	u8 *buffer;
};


static inline struct nand_chip *dev_to_chip(struct yaffs_dev *dev)
{
	struct nand_context *ctxt =
		(struct nand_context *)(dev->driver_context);
	return ctxt->chip;
}

static inline u8 *dev_to_buffer(struct yaffs_dev *dev)
{
	struct nand_context *ctxt =
		(struct nand_context *)(dev->driver_context);
	return ctxt->buffer;
}

static int yaffs_nand_drv_WriteChunk(struct yaffs_dev *dev, int nand_chunk,
				   const u8 *data, int data_len,
				   const u8 *oob, int oob_len)
{
	struct nand_chip *chip = dev_to_chip(dev);
	u8 *buffer = dev_to_buffer(dev);
	u8 *e;
//	struct nanddrv_transfer tr[2];
	int i;
    status_t status;

	if(!data || !oob)
		return YAFFS_FAIL;


	/* Calc ECC and marshall the oob bytes into the buffer */

	memset(buffer, 0xff, chip->spare_bytes_per_page);

	for(i = 0, e = buffer + 2; i < chip->data_bytes_per_page; i+=256, e+=3)
		yaffs_ecc_calc(data + i, e);

	memcpy(buffer + 26, oob, oob_len);

//	/* Set up and execute transfer */
//
//	tr[0].buffer = (u8 *)data;
//	tr[0].offset = 0;
//	tr[0].nbytes = data_len;
//
//	tr[1].buffer = buffer;
//	tr[1].offset = chip->data_bytes_per_page;
//	tr[1].nbytes = chip->spare_bytes_per_page;
//
//	if(nanddrv_write_tr(chip, nand_chunk, tr, 2) == 0)
//		return YAFFS_OK;
//	else
//		return YAFFS_FAIL;

    if((NULL != data) && (data_len > 0))
    {
        status = Nand_Flash_Page_Program(&nandHandle, nand_chunk, data, data_len);
        if (status != kStatus_Success)
        {
        	printf("\r\nNand_Flash_Page_Program写入失败 nand_chunk %d!\r\n",nand_chunk);
        	//ErrorHandler();
        	return YAFFS_FAIL;
        }
    }

    if((NULL != oob) && (oob_len > 0))
    {
        status = Nand_Flash_Page_Program_Partial(&nandHandle, nand_chunk, chip->data_bytes_per_page, buffer, chip->spare_bytes_per_page);
        if (status != kStatus_Success)
        {
        	printf("\r\nNand_Flash_Page_Program_Partial写入失败 nand_chunk %d!\r\n",nand_chunk);
        	//ErrorHandler();
        	return YAFFS_FAIL;
        }
    }

	return YAFFS_OK;
}

static int yaffs_nand_drv_ReadChunk(struct yaffs_dev *dev, int nand_chunk,
				   u8 *data, int data_len,
				   u8 *oob, int oob_len,
				   enum yaffs_ecc_result *ecc_result_out)
{
	struct nand_chip *chip = dev_to_chip(dev);
	u8 *buffer = dev_to_buffer(dev);
    status_t status;

	
//	struct nanddrv_transfer tr[2];
//	struct nanddrv_transfer *trp = tr;
//	int n_tr = 0;
	int ret;
	enum yaffs_ecc_result ecc_result;
	int i;
	u8 *e;
	u8 read_ecc[3];

//	if(data) {
//		trp->buffer = data;
//		trp->offset = 0;
//		trp->nbytes = data_len;
//		n_tr++;
//		trp++;
//	}
//
//
//	trp->buffer = buffer;
//	trp->offset = chip->data_bytes_per_page;
//	trp->nbytes = chip->spare_bytes_per_page;
//	n_tr++;
//	trp++;


//	ret = nanddrv_read_tr(chip, nand_chunk, tr, n_tr);
    if((NULL != data) && (data_len > 0))
    {
    	status = Nand_Flash_Read_Page(&nandHandle, nand_chunk, data, data_len);
    	if (status != kStatus_Success)
    	{
    		printf("\r\nNand_Flash_Read_Page读取页失败 nand_chunk: %d !\r\n",nand_chunk);
    		//ErrorHandler();
    		if (ecc_result_out)
    			*ecc_result_out = YAFFS_ECC_RESULT_UNKNOWN;
    		return YAFFS_FAIL;
    	}
	}
    if((NULL != oob) && (oob_len > 0))
    {
    	status = Nand_Flash_Read_Page_Partial(&nandHandle, nand_chunk, chip->data_bytes_per_page, buffer, chip->spare_bytes_per_page);
    	if (status != kStatus_Success)
    	{
    		printf("\r\nNand_Flash_Read_Page_Partial读取页失败 nand_chunk: %d !\r\n",nand_chunk);

    		//ErrorHandler();
    		if (ecc_result_out)
    			*ecc_result_out = YAFFS_ECC_RESULT_UNKNOWN;
    		return YAFFS_FAIL;
    	}
    
    
    	/* Do ECC and marshalling */
    	if(oob)
    		memcpy(oob, buffer + 26, oob_len);
	}

	ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	if(data) {
		for(i = 0, e = buffer + 2; i < chip->data_bytes_per_page; i+=256, e+=3) {
			yaffs_ecc_calc(data + i, read_ecc);
			ret = yaffs_ecc_correct(data + i, e, read_ecc);
			if (ret < 0)
				ecc_result = YAFFS_ECC_RESULT_UNFIXED;
			else if( ret > 0 && ecc_result == YAFFS_ECC_RESULT_NO_ERROR)
				ecc_result = YAFFS_ECC_RESULT_FIXED;
		}
	}

	if (ecc_result_out)
		*ecc_result_out = ecc_result;

	return YAFFS_OK;
}

static int yaffs_nand_drv_EraseBlock(struct yaffs_dev *dev, int block_no)
{
	struct nand_chip *chip = dev_to_chip(dev);
    status_t status;
	//if(nanddrv_erase(chip, block_no) == 0)

    status = Nand_Flash_Erase_Block(&nandHandle, block_no);
    if (status != kStatus_Success)
    {
        printf("\r\nNAND Flash擦除块失败 Block no %d!\r\n",block_no);
        //ErrorHandler();
        return YAFFS_FAIL;
    }
    else
    {
		return YAFFS_OK;
    }

		
}
/*block的第一个page，extra区域写入*/
static int yaffs_nand_drv_MarkBad(struct yaffs_dev *dev, int block_no)
{
	struct nand_chip *chip = dev_to_chip(dev);
	u8 *buffer = dev_to_buffer(dev);
	int nand_chunk = block_no * chip->pages_per_block;
//	struct nanddrv_transfer tr[1];
    status_t status;

	memset(buffer, 0xff, chip->spare_bytes_per_page);
	buffer[0] = 'Y';
	buffer[1] = 'Y';

//	tr[0].buffer = buffer;
//	tr[0].offset = chip->data_bytes_per_page;
//	tr[0].nbytes = chip->spare_bytes_per_page;
//
//	if(nanddrv_write_tr(chip, nand_chunk, tr, 1) == 0)
//		return YAFFS_OK;
//	else
//		return YAFFS_FAIL;

    status = Nand_Flash_Page_Program_Partial(&nandHandle, nand_chunk, chip->data_bytes_per_page, buffer, chip->spare_bytes_per_page);
    if (status != kStatus_Success)
    {
    	printf("\r\nyaffs_nand_drv_MarkBad block No %d!\r\n",block_no);
    	//ErrorHandler();
    	return YAFFS_FAIL;
    }
    
	return YAFFS_OK;
}

static int yaffs_nand_drv_CheckBad(struct yaffs_dev *dev, int block_no)
{
	struct nand_chip *chip = dev_to_chip(dev);
	u8 *buffer = dev_to_buffer(dev);
	int nand_chunk = block_no * chip->pages_per_block;
//	struct nanddrv_transfer tr[1];
    status_t status;

	memset(buffer, 0, chip->spare_bytes_per_page);

//	tr[0].buffer = buffer;
//	tr[0].offset = chip->data_bytes_per_page;
//	tr[0].nbytes = chip->spare_bytes_per_page;
//
//	nanddrv_read_tr(chip, nand_chunk, tr, 1);

	status = Nand_Flash_Read_Page_Partial(&nandHandle, nand_chunk, chip->data_bytes_per_page, buffer, chip->spare_bytes_per_page);
	if (status != kStatus_Success)
	{
		printf("\r\nNand_Flash_Read_Page_Partial 读取失败!\r\n");
		//ErrorHandler();
		return YAFFS_FAIL;
	}

	/* Check that bad block marker is not set */
	if(yaffs_hweight8(buffer[0]) + yaffs_hweight8(buffer[1]) < 14)
		return YAFFS_FAIL;
	else
		return YAFFS_OK;

}

static int yaffs_nand_drv_Initialise(struct yaffs_dev *dev)
{
	struct nand_chip *chip = dev_to_chip(dev);
    status_t status;
    char Manufacturer[12];
	(void)chip;
	
    printf("\r\nNAND Flash初始化!\r\n");
    status = Nand_Flash_Init(&nandConfig, &nandHandle);
    if (status != kStatus_Success)
    {
        printf("\r\nNAND Flash初始化失败!\r\n");
        printf("\r\n error code: %d \r\n", status);
        ErrorHandler();
    }
    printf("\r\nNAND Flash初始化成功!\r\n");
    
	print_nand_info(&nandHandle);

	return YAFFS_OK;
}

static int yaffs_nand_drv_Deinitialise(struct yaffs_dev *dev)
{
	struct nand_chip *chip = dev_to_chip(dev);

	(void) chip;
	return YAFFS_OK;
}


int yaffs_nand_install_drv(struct yaffs_dev *dev, struct nand_chip *chip)
{
	struct yaffs_driver *drv = &dev->drv;
	u8 *buffer = NULL;
	struct nand_context *ctxt = NULL;

	//ctxt = malloc(sizeof(struct nand_context));
	//buffer = malloc(chip->spare_bytes_per_page);

	ctxt = yaffsfs_malloc(sizeof(struct nand_context));
	buffer = yaffsfs_malloc(chip->spare_bytes_per_page);

	if(!buffer || !ctxt)
		goto fail;

	drv->drv_write_chunk_fn = yaffs_nand_drv_WriteChunk;
	drv->drv_read_chunk_fn = yaffs_nand_drv_ReadChunk;
	drv->drv_erase_fn = yaffs_nand_drv_EraseBlock;
	drv->drv_mark_bad_fn = yaffs_nand_drv_MarkBad;
	drv->drv_check_bad_fn = yaffs_nand_drv_CheckBad;
	drv->drv_initialise_fn = yaffs_nand_drv_Initialise;
	drv->drv_deinitialise_fn = yaffs_nand_drv_Deinitialise;

	ctxt->chip = chip;
	ctxt->buffer = buffer;
	dev->driver_context = (void *) ctxt;
	return YAFFS_OK;

fail:
	//free(ctxt);
	//free(buffer);
	yaffsfs_free(ctxt);
	yaffsfs_free(buffer);
	return YAFFS_FAIL;
}
