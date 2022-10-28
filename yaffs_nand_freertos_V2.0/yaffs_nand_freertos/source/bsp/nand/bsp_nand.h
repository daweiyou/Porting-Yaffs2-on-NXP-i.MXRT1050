#ifndef __BSP_NAND_H
#define	__BSP_NAND_H

#include "fsl_common.h"
#include "fsl_nand_flash.h"

#define EXAMPLE_SEMC SEMC
#define EXAMPLE_SEMC_START_ADDRESS (0x80000000U)
#define EXAMPLE_SEMC_CLK_FREQ CLOCK_GetFreq(kCLOCK_SemcClk)
#define EXAMPLE_SEMC_NAND_AXI_START_ADDRESS (0x9E000000U)
#define EXAMPLE_SEMC_NAND_IPG_START_ADDRESS (0x00000000U)
#define FLASH_PAGE_SIZE_4K (4096)
#define FLASH_PAGE_SIZE_2K (2112) //2048+64

extern nand_config_t nandConfig;

/*******************************************************************************
 * º¯ÊýÉùÃ÷
 ******************************************************************************/
void BOARD_InitNand(void);
#endif /* __BSP_NAND_H */
