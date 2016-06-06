/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
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
#include "stm32f1xx_hal.h"
#include "string.h"

/* USER CODE BEGIN Includes */
#define BUFFSIZE 512
#define BACKSPACE 127
#define ENTER '\r'
#define MIN(x,y) ((x)>(y)?(y):(x))
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFF); 
  return ch;
}
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFF); 
	return ch;
}

struct uart {  
    uint8_t *rear;
    uint8_t *front;
};
struct uart uart_rev; 
uint8_t aRxBuffer[BUFFSIZE];
char input[BUFFSIZE] = "";

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)  
{  
    uint8_t ret = HAL_OK;

    // last HAL_UART_Reive_IT call's storage place
    char c = *uart_rev.rear;
		printf("%c",c);
    if (c == '\r'){
        printf("\n");
    }
		
		uart_rev.rear++;   //update rear pointer
    if(uart_rev.rear >= (aRxBuffer + BUFFSIZE))  
        uart_rev.rear = aRxBuffer;  
		
		// reset received data's srorage place and data length
    do{  
        ret = HAL_UART_Receive_IT(UartHandle, uart_rev.rear, 1);  
    }while(ret != HAL_OK);
}

//get uart input from buffer into fmt, return input's length
int8_t uart_gets(uint8_t *fmt, uint16_t upperBound)  
{  
    int count = 0;	 //count is the length of bytes read
		upperBound = MIN(upperBound,BUFFSIZE); //upperBound is the reading length's upper bound
    while(count < upperBound){
        if(uart_rev.front != uart_rev.rear){
            char c = *uart_rev.front;		//read a char from front pointer
            uart_rev.front++;   //update front pointer
						if(uart_rev.front >= (aRxBuffer + BUFFSIZE))  
								uart_rev.front = aRxBuffer;  
            if (c == ENTER){ 		//ENTER means input's end
                break;
            }
            *fmt = c;
            if (c != BACKSPACE){
                fmt++;
                count++;
            }else if(count > 0){
                fmt--;
                count--;
            }
        }
		}
    *fmt = '\0';
    return count;  
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		int count = 0;
		char temp[100];
		printf("STM32> ");
		HAL_Delay(200);
		count = uart_gets((uint8_t*)input, BUFFSIZE);	//read input
		//printf("Read %d chars:%s\r\n", count, input);
		char cmd[100];
		int addr,data,paramNum;
		sscanf(input, "%s", cmd);
		if(strcmp(cmd,"peek")==0){
			paramNum = sscanf(input,"%s %x %s",cmd,&addr,temp);
			if(paramNum==2){
				printf("peek: 0x%x in addr 0x%x\r\n",*(int*)addr,addr);
			}else{
				printf("Usage:peek addr\r\n");
			}
		}else if(strcmp(cmd,"poke")==0){
			paramNum = sscanf(input,"%s %x %x %s",cmd,&addr,&data,temp);
			if(paramNum==3){
				*((int*)addr) = data;
				printf("poke: change addr 0x%x to 0x%x\r\n",addr,data);
			}else{
				printf("Usage:poke addr data\r\n");
			}
		}else if(strcmp(cmd,"tempaddr")==0){
			printf("temp array's addr is 0x%x, length:%d\r\n",(int)temp,100);
		}else{
			printf("invalid cmd!\r\n");
		}
		
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	
	if(HAL_UART_Init(&huart1) != HAL_OK){  
      printf("Uart Init Error!\r\n"); 
			while(1);
  }  

	NVIC_SetPriority(USART1_IRQn,0);  
	NVIC_EnableIRQ(USART1_IRQn);  
	uart_rev.front = aRxBuffer;  
	uart_rev.rear = aRxBuffer;
	//assign space to receive input
	if(HAL_UART_Receive_IT(&huart1,(uint8_t*)aRxBuffer,1) != HAL_OK){  
			printf("Uart Read Error!\r\n"); 
	}  
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
