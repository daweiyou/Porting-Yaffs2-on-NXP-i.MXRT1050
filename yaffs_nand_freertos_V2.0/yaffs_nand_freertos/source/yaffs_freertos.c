/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdarg.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "board.h"
#include "sdmmc_config.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_common.h"

#include "yportenv.h"

#include "private_lpuart_freertos.h"
#include "fsl_lpuart.h"
#include "fsl_lpuart_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "lpuart_freertos_cfg.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Task stack size. */
#define DEFAULT_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define DEFAULT_TASK_PRIORITY (configMAX_PRIORITIES - 1U)

#define cli_task_PRIORITY (configMAX_PRIORITIES - 1)

#define PRIVATE_CONSOLE_SCANF_MAX_LOG_LEN (20U)
#define PRIVATE_CONSOLE_PRINTF_MAX_LOG_LEN (512U)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Yaffs2BasicTestTask(void *pvParameters);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief semaphore  */
/*
static SemaphoreHandle_t s_fileAccessSemaphore = NULL;
static SemaphoreHandle_t s_CardDetectSemaphore = NULL;
*/
uint8_t background_buffer[32];
//uint8_t recv_buffer[4];
uint32_t recv_buffer_len;
lpuart_rtos_handle_t lpuart_rtos_handle;
struct _lpuart_handle t_handle;

lpuart_rtos_config_t lpuart_config = {
    .baudrate    = 115200,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

AT_NONCACHEABLE_SECTION_INIT(uint8_t g_txBuffer[UART_TX_BUFFER_LENGTH]) = {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t g_rxBuffer[UART_RX_BUFFER_LENGTH]) = {0};

extern xComPortHandle* pxPort ;

/*******************************************************************************
 * Code
 ******************************************************************************/
//return lpuart_rtos_handle, if return 0, fail init
xComPortHandle* xSerialPortInitMinimal(unsigned int baudrate, unsigned int queuelen)
{
	xComPortHandle* port = NULL;

	lpuart_config.srcclk = DEMO_LPUART_CLK_FREQ;
	lpuart_config.base	 = DEMO_LPUART;
	lpuart_config.baudrate = baudrate;

	if (kStatus_Success != LPUART_RTOS_Init_Private(&lpuart_rtos_handle, &t_handle, &lpuart_config))
	{
        port = NULL;
	}
	else
	{
        port = &lpuart_rtos_handle;
	}

	return port;
}

BaseType_t xSerialGetChar(xComPortHandle* pxPort, uint8_t* p_rx_char, unsigned int delay)
{
	int error;
	size_t n = 0;

	error = LPUART_RTOS_Receive_Private(pxPort, g_rxBuffer, UART_RX_BUFFER_LENGTH, &n);
	if (error == kStatus_LPUART_Error)
	{
		return pdFAIL;
	}

    assert(n>0);
    
    if(n>0)
	    *p_rx_char = g_rxBuffer[0];

	return pdPASS;
}

void xSerialPutChar( xComPortHandle* pxPort, uint8_t tx_char, unsigned int delay )
{
    //use non cache buffer for DMA operation
    g_txBuffer[0] = tx_char;
	if (kStatus_Success != LPUART_RTOS_Send_Private(pxPort, g_txBuffer, 1))
	{
		;
	}
}
void vSerialPutString(xComPortHandle* pxPort, uint8_t *tx_buffer, short tx_buffer_len)
{
    short tx_len = 0;
    
    tx_len = (tx_buffer_len < UART_TX_BUFFER_LENGTH)?tx_buffer_len:UART_TX_BUFFER_LENGTH;

        
    //use non cache buffer for DMA operation
    memcpy(g_txBuffer,tx_buffer,tx_len);
	if (kStatus_Success != LPUART_RTOS_Send_Private(pxPort, g_txBuffer, tx_len))
	{
		;
	}
}
static void PrintCallback_private(char *buf, int32_t *indicator, char dbgVal, int len)
{
    int i = 0;

    for (i = 0; i < len; i++)
    {
        if (((uint32_t)*indicator + 1UL) >= PRIVATE_CONSOLE_PRINTF_MAX_LOG_LEN)
        {
            //(void)DbgConsole_SendDataReliable((uint8_t *)buf, (uint32_t)(*indicator));
            vSerialPutString(pxPort,(uint8_t *)buf, (uint32_t)(*indicator));
            *indicator = 0;
        }

        buf[*indicator] = dbgVal;
        (*indicator)++;
    }
}

/* See fsl_debug_console.h for documentation of this function. */
int printf_private(const char *formatString, ...)
{
    va_list ap;
    int logLength = 0, dbgResult = 0;
    char printBuf[PRIVATE_CONSOLE_PRINTF_MAX_LOG_LEN] = {'\0'};


    {
        va_start(ap, formatString);
        /* format print log first */
        logLength = StrFormatPrintf(formatString, ap, printBuf, PrintCallback_private);
        /* print log */
        //dbgResult = DbgConsole_SendDataReliable((uint8_t *)printBuf, (size_t)logLength);
        vSerialPutString(pxPort,(uint8_t *)printBuf, (size_t)logLength);

        va_end(ap);
    }
    return dbgResult;
}

/* See fsl_debug_console.h for documentation of this function. */
int Putchar_private(int ch)
{
    /* print char */
    //return DbgConsole_SendDataReliable((uint8_t *)&ch, 1U);
    xSerialPutChar(pxPort,ch, 1U);
    
    return 0;
}


static void Yaffs2BasicTestTask(void *pvParameters)
{
    #if 0
    nand_flash_test();
    #endif
    
	yaffs_start_up();
    
    //yaffs_test_main(NXP_YAFFS_NAME);
    //printf("Format first\r\n");
    //yaffs_format(NXP_YAFFS_NAME,0,0,0);
    dtest_main(NULL,NULL);

    vTaskSuspend(NULL);
}

/*!
 * @brief Main function
 */

int main(void)
{
    lpuart_config_t lpuartConfig;
    edma_config_t config;
    lpuart_transfer_t xfer;
    uint8_t printBuf;
    
    
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
//  BOARD_InitDebugConsole();

    LED_GPIO_Config();
    
	BOARD_InitNand();

//    printf("\r\n");
//    printf("*****Board clock information*****\r\n");
//    printf("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
//    printf("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
//    printf("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
//    printf("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
//    printf("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
//    printf("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
//    printf("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
//    printf("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));	

//    if (pdPASS !=
//        xTaskCreate(Yaffs2BasicTestTask, "Yaffs2BasicTestTask", DEFAULT_TASK_STACK_SIZE, NULL, DEFAULT_TASK_PRIORITY, NULL))
//    {
//        return -1;
//    }

	vUARTCommandConsoleStart(configMINIMAL_STACK_SIZE + 500, cli_task_PRIORITY);
	
	/* Register commands with the FreeRTOS+CLI command interpreter. */
	vRegisterBasicCLICommands();
	
	/*Register Yaffs2 FS operation commands*/
    vRegisterFileSystemCLICommands();

    yaffs_start_up();
    
    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* Scheduler should never reach this point. */
    while (true)
    {
    }
}



