/**
  ******************************************************************************
  * @file    SPI/SPI_FullDuplex_AdvComIT/Master/Src/main.c
  * @author  MCD Application Team
  * @version   V1.2.4
  * @date      13-November-2015
  * @brief   This sample code shows how to use STM32F4xx SPI HAL API to transmit
  *          and receive a data buffer with a communication process based on
  *          IT transfer.
  *          The communication is done using 2 boards.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup SPI_FullDuplex_AdvCom_IT
  * @{
  */

/** @addtogroup Master
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SPI_ACK_BYTES             0xA5A5
#define SPI_NACK_BYTES            0xDEAD
#define SPI_TIMEOUT_MAX           0x1000
#define SPI_SLAVE_SYNBYTE         0x53
#define SPI_MASTER_SYNBYTE        0xAC

/* Defines used for transfer communication */
#define ADDRCMD_MASTER_READ                         ((uint16_t)0x1234)
#define ADDRCMD_MASTER_WRITE                        ((uint16_t)0x5678)
#define CMD_LENGTH                                  ((uint16_t)0x0004)
#define DATA_LENGTH                                 ((uint16_t)0x0020)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;
FlagStatus TestReady = RESET;

/* Buffer used for transmission */
uint8_t aTxMasterBuffer[] = "SPI - MASTER - Transmit message";
uint8_t aTxSlaveBuffer[]  = "SPI - SLAVE - Transmit message ";
/* Buffer used for reception */
uint8_t aRxBuffer[DATA_LENGTH];

/* Private function prototypes -----------------------------------------------*/
static void Master_Synchro(void);
static void SystemClock_Config(void);
static void Error_Handler(void);
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint8_t addrcmd[CMD_LENGTH] = {0};
  uint16_t ackbytes = 0x0000;

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();
  
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  
  /* Configure LED3, LED4, LED5 and LED6 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);
    
  /*##-1- Configure the SPI peripheral #######################################*/
  /* Set the SPI parameters */
  SpiHandle.Instance               = SPIx;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.Mode              = SPI_MODE_MASTER;  
  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Configure USER Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /* Wait for user Button press before starting the communication. Toggles LED3 until then */
  while(TestReady != SET)
  {
    BSP_LED_Toggle(LED3);
    HAL_Delay(40);
  }
  
  /* Turn Off LED3 */
  BSP_LED_Off(LED3);
  
  /* Infinite loop */  
  while(1)
  {
    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Receive Data from the Slave ###########################################*/ 
    addrcmd[0] = (uint8_t) (ADDRCMD_MASTER_READ >> 8);
    addrcmd[1] = (uint8_t) ADDRCMD_MASTER_READ;
    addrcmd[2] = (uint8_t) (DATA_LENGTH >> 8);
    addrcmd[3] = (uint8_t) DATA_LENGTH;
    /* Send Master READ command to slave */
    if(HAL_SPI_Transmit_IT(&SpiHandle, addrcmd, CMD_LENGTH) != HAL_OK)
    {
      Error_Handler();
    }
     /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {}
    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Receive ACK from the Slave */
    ackbytes = 0;
    if(HAL_SPI_Receive_IT(&SpiHandle, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {}
    /* Check the received ACK */
    if(ackbytes == SPI_ACK_BYTES)
    {
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Receive the requested data from the slave */
      if(HAL_SPI_Receive_IT(&SpiHandle, aRxBuffer, DATA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
      {}
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Send ACK to the Slave */
      ackbytes = SPI_ACK_BYTES;
      if(HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
      {}
    }    
    else
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
    
    /* Compare received buffer with one expected from slave */
    if(Buffercmp((uint8_t*)aTxSlaveBuffer, (uint8_t*)aRxBuffer, CMD_LENGTH))
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
    else
    {
      /* Turn LED6 on: Reception is correct */
      BSP_LED_Toggle(LED6);
    }

    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Transmit Data To Slave ################################################*/
    addrcmd[0] = (uint8_t) (ADDRCMD_MASTER_WRITE >> 8);
    addrcmd[1] = (uint8_t) ADDRCMD_MASTER_WRITE;
    addrcmd[2] = (uint8_t) (DATA_LENGTH >> 8);
    addrcmd[3] = (uint8_t) DATA_LENGTH;
    /* Send Master WRITE command to the slave */
    if(HAL_SPI_Transmit_IT(&SpiHandle, addrcmd, CMD_LENGTH) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {}
    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Receive ACK from the Slave */
    ackbytes = 0;
    if(HAL_SPI_Receive_IT(&SpiHandle, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {}
    /* Check the received ACK */
    if(ackbytes == SPI_ACK_BYTES)
    {
      /* Synchronization between Master and Slave */
      Master_Synchro();
      /* Send the requested data from the slave */
      if(HAL_SPI_Transmit_IT(&SpiHandle, aTxMasterBuffer, DATA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
      {}
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Receive ACK from the Slave */
      ackbytes = 0;
      if(HAL_SPI_Receive_IT(&SpiHandle, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
      {}
    }    
    else
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
   
    /* Flush Rx buffer for next transmission */
    Flush_Buffer(aRxBuffer, DATA_LENGTH);
    
    /* Toggle LED4 */
    BSP_LED_Toggle(LED4);
    
    /* This delay permit to user to see LED4 toggling*/
    HAL_Delay(100);
  }
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
 if(KEY_BUTTON_PIN == GPIO_Pin)
 {
   TestReady = SET;
 }
}

/**
  * @brief  Master Synchronization with Slave.
  * @param  None
  * @retval None
  */
static void Master_Synchro(void)
{
  uint8_t txackbytes = SPI_MASTER_SYNBYTE, rxackbytes = 0x00;
  do
  {
    /* Call SPI write function to send command to slave */
    if(HAL_SPI_TransmitReceive_IT(&SpiHandle, (uint8_t *)&txackbytes, (uint8_t *)&rxackbytes, 1) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY)
    {}
  }while(rxackbytes != SPI_SLAVE_SYNBYTE);
}

/**
  * @brief  SPI error callbacks
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  /* call error handler */
  Error_Handler();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
   */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED5 on */
  BSP_LED_On(LED5);
  while(1)
  {
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if((*pBuffer1) != *pBuffer2)
    {
      return BufferLength;
    }
    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}

/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    *pBuffer = 0;

    pBuffer++;
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
