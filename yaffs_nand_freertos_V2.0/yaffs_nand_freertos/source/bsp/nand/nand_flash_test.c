/**
  ******************************************************************
  * @file    nand_flash_test.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   NAND���Գ���
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  i.MXRT1052������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************
  */

#include "board.h"
#include "bsp_nand.h"
#include "fsl_debug_console.h"
#include "fsl_nand_flash.h"
#include "fsl_semc_nand_flash.h"

uint8_t mem_writeBuffer[FLASH_PAGE_SIZE_2K];
uint8_t mem_readBuffer[FLASH_PAGE_SIZE_2K] = {0};
static nand_handle_t nandHandle;

extern const uint8_t s_nandDeviceManufacturerList[][12];
extern nand_config_t nandConfig;
extern uint32_t NAND_ReadID(void);

#undef PRINTF
#define PRINTF printf_private

/**
* @brief  ��������
* @param  ��
* @retval ��
*/
void ErrorHandler(void)
{
    PRINTF("\n\r��д������������\n\r");
    while (1)
    {
    }
}
void print_nand_info(nand_handle_t *handle)
{
    char Manufacturer[12];

	semc_mem_nand_handle_t *semcHandle = (semc_mem_nand_handle_t *)handle->deviceSpecific;

    assert(handle != NULL);
    assert(semcHandle != NULL);

    
    //��ȡID
    PRINTF("\r\nNAND FlashID:0x%x\r\n", NAND_ReadID());
    memset(Manufacturer, 0, 12);
    strncpy(Manufacturer, (const char *)s_nandDeviceManufacturerList[handle->vendorType], 8);
    PRINTF("\r\nNAND Flash  ���̣�%s\r\n", Manufacturer);
    PRINTF("\r\nNAND Flashÿҳ���������С��%d�ֽ�\r\n", handle->bytesInPageDataArea);
    PRINTF("\r\nNAND Flashÿҳ���������С��%d�ֽ�\r\n", handle->bytesInPageSpareArea);
    PRINTF("\r\nNAND Flashÿһ�������ҳ����%dҳ\r\n", handle->pagesInBlock);
    PRINTF("\r\nNAND Flashÿһ�������ҳ����%dҳ\r\n", handle->blocksInPlane);
    PRINTF("\r\nNAND Flash�豸����������%d��\r\n", handle->planesInDevice);

    PRINTF("\r\nNAND Flash columnWidth��%d bits\r\n", semcHandle->columnWidth);
    PRINTF("\r\nNAND Flash isFeatureCommandSupport��%d\r\n", semcHandle->isFeatureCommandSupport);
    PRINTF("\r\nNAND Flash eccCheckType��%s\r\n", (semcHandle->eccCheckType==kNandEccCheckType_SoftwareECC)?"SW_ecc":"Dev_ecc");
    PRINTF("\r\nNAND Flash statusCommandType��%s\r\n", (semcHandle->statusCommandType==0)?"CMD normal":"CMD Enhanced");
    PRINTF("\r\nNAND Flash ctlAccessMemAddr1��0x%x\r\n", semcHandle->ctlAccessMemAddr1);
    PRINTF("\r\nNAND Flash ctlAccessMemAddr2��0x%x\r\n", semcHandle->ctlAccessMemAddr2);

}
/**
* @brief  nand flash ��д���Գ���
* @param  ��
* @retval kStatus_Success���ɹ�
*/
void nand_flash_test(void)
{
    status_t status;
    char Manufacturer[12];
	uint32_t blockIndex = 500;
	uint32_t blockbasepageIndex;


    PRINTF("\r\nNAND Flash���Գ���!\r\n");

    PRINTF("\r\nNAND Flash��ʼ��!\r\n");
    status = Nand_Flash_Init(&nandConfig, &nandHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND Flash��ʼ��ʧ��!\r\n");
        PRINTF("\r\n error code: %d \r\n", status);
        ErrorHandler();
    }
    PRINTF("\r\nNAND Flash��ʼ���ɹ�!\r\n");

    
//    //��ȡID
//    PRINTF("\r\nNAND FlashID:0x%x\r\n", NAND_ReadID());
//    memset(Manufacturer, 0, 12);
//    strncpy(Manufacturer, (const char *)s_nandDeviceManufacturerList[nandHandle.vendorType], 8);
//    PRINTF("\r\nNAND Flash  ���̣�%s\r\n", Manufacturer);
//    PRINTF("\r\nNAND Flashÿҳ���������С��%d�ֽ�\r\n", nandHandle.bytesInPageDataArea);
//    PRINTF("\r\nNAND Flashÿҳ���������С��%d�ֽ�\r\n", nandHandle.bytesInPageSpareArea);
//    PRINTF("\r\nNAND Flashÿһ�������ҳ����%dҳ\r\n", nandHandle.pagesInBlock);
//    PRINTF("\r\nNAND Flashÿһ�������ҳ����%dҳ\r\n", nandHandle.blocksInPlane);
//    PRINTF("\r\nNAND Flash�豸����������%d��\r\n", nandHandle.planesInDevice);

	print_nand_info(&nandHandle);

    //return;
    
    //		PRINTF("\r\n***NAND Flash ID��%d\r\n",nandHandle.JEDECid);
    //    SEMC->IPCR1 = 1;

    /* Erase Block */
    PRINTF("\r\nNAND Flash Erase������һ����!\r\n");
    status = Nand_Flash_Erase_Block(&nandHandle, blockIndex);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND Flash������ʧ��!\r\n");
        ErrorHandler();
    }

    /* Read and check if it is blank. */
    PRINTF("\r\nNAND Flash����У��!\r\n");
    blockbasepageIndex = blockIndex * nandHandle.pagesInBlock;
    for (uint32_t pageIndex = 0; pageIndex < nandHandle.pagesInBlock; pageIndex++)
    {
        status = Nand_Flash_Read_Page(&nandHandle, pageIndex+blockbasepageIndex, mem_readBuffer, nandHandle.bytesInPageDataArea);
        if (status != kStatus_Success)
        {
            PRINTF("\r\nNAND Flash��ȡҳʧ��!\r\n");
            ErrorHandler();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < nandHandle.bytesInPageDataArea; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\nNAND Flash������ʧ��!\r\n");
                ErrorHandler();
            }
        }
    }

    for (uint32_t pageIndex = 0; pageIndex < nandHandle.pagesInBlock; pageIndex++)
    {
        status = Nand_Flash_Read_Page_Partial(&nandHandle, pageIndex+blockbasepageIndex, nandHandle.bytesInPageDataArea,mem_readBuffer, nandHandle.bytesInPageSpareArea);
        if (status != kStatus_Success)
        {
            PRINTF("\r\nNAND Flash��ȡҳSpareʧ��!\r\n");
            ErrorHandler();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < nandHandle.bytesInPageSpareArea; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\nNAND Flash������ʧ��!\r\n");
                ErrorHandler();
            }
        }
    }

    PRINTF("\r\nNAND Flash������ɹ�!\r\n");

    /* Program the page data. */
    PRINTF("\r\nNAND Flashҳд��!\r\n");
    /* Initialize the write buffers. */
    memset(mem_writeBuffer, 0xaa, sizeof(mem_writeBuffer));
	for (uint32_t bytesIndex = nandHandle.bytesInPageDataArea; bytesIndex < (nandHandle.bytesInPageDataArea + nandHandle.bytesInPageSpareArea); bytesIndex++)
	{
		mem_writeBuffer[bytesIndex]=0x55;
	}

    
    status = Nand_Flash_Page_Program(&nandHandle, blockbasepageIndex, mem_writeBuffer, nandHandle.bytesInPageDataArea);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND Flashҳд��ʧ��!\r\n");
        ErrorHandler();
    }
    status = Nand_Flash_Page_Program_Partial(&nandHandle, blockbasepageIndex, nandHandle.bytesInPageDataArea, &mem_writeBuffer[nandHandle.bytesInPageDataArea], nandHandle.bytesInPageSpareArea);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND Flashҳspareд��ʧ��!\r\n");
        ErrorHandler();
    }

    /* Read page data and check if the data read is equal to the data programed. */
    PRINTF("\r\nNAND Flashҳ��ȡ!\r\n");
//    status = Nand_Flash_Read_Page(&nandHandle, 0, mem_readBuffer, nandHandle.bytesInPageDataArea);
//    if (status != kStatus_Success)
//    {
//        PRINTF("\r\nNAND Flashҳ��ȡʧ��!\r\n");
//        ErrorHandler();
//    }

    status = Nand_Flash_Read_Page(&nandHandle, blockbasepageIndex, mem_readBuffer, nandHandle.bytesInPageDataArea+nandHandle.bytesInPageSpareArea);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND Flashҳ��ȡʧ��!\r\n");
        ErrorHandler();
    }
    
    if (memcmp(mem_writeBuffer, mem_readBuffer, nandHandle.bytesInPageDataArea) != 0)
    {
        PRINTF("\r\nNAND Flashҳ��ȡʧ��!\r\n");
        ErrorHandler();
    }

    /* Read page data and check if the data read is equal to the data programed. */
    PRINTF("\r\nNAND FlashҳSpare��ȡ!\r\n");
    status = Nand_Flash_Read_Page_Partial(&nandHandle, blockbasepageIndex, nandHandle.bytesInPageDataArea, mem_readBuffer, nandHandle.bytesInPageSpareArea);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNAND FlashҳSpare��ȡʧ��!\r\n");
        ErrorHandler();
    }

    if (memcmp(&mem_writeBuffer[nandHandle.bytesInPageDataArea], mem_readBuffer, nandHandle.bytesInPageSpareArea) != 0)
    {
        PRINTF("\r\nNAND Flashҳspare��ȡʧ��!\r\n");
        ErrorHandler();
    }

    PRINTF("\r\nNAND Flashҳ��д�ɹ�!\r\n");
    while(1);
}














