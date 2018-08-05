/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "main.h"
#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "stm32_adafruit_sd.h"
#include "ff.h"
#include "private.h"
#include "rtp.h"
#include "string.h"

#define FILENAME		"audio.raw"
#define FILENAME_OUT	"alaw.raw"
#define AUDIO_BUFFER_SIZE		160

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

//static uint32_t ms_ticks; /**< 1ms timeticks counter */
static FATFS fs; /**< FatFs work area needed for each volume */
static FIL fp; /**< File object needed for each open file */
static FIL fpout;

volatile g726_state g726encode;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);

DWORD get_fattime(void) {
	return ((DWORD) (2018 - 1980) << 25 | (DWORD) 06 << 21 | (DWORD) 24 << 16);
}

static void frameToAlaw(BYTE *out, SHORT *pcm, uint32_t len) {
	for (int i = 0; i < len; i++) {
		out[i] = (BYTE) linear2alaw(pcm[i]);
	}
}

int main(void) {

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART3_UART_Init();

	RTP_Init();

//	HAL_Delay(1000);
//
//	char msg[] =
//			"HOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHOLAHHHHHHHH";
	char recvBuffer[172];
//	for (int i = 0; i < 172; ++i) {
//		recvBuffer[i] = 0x01;
//	}
//
//	while (1) {
//
//		HAL_UART_Init(&huart3);
//		HAL_StatusTypeDef res;
//		res = HAL_UART_Receive_DMA(&huart3, (uint8_t *) recvBuffer,
//				strlen(recvBuffer));
//
//		res = HAL_UART_Transmit_DMA(&huart3, (uint8_t*) msg, strlen(msg));
//
//		HAL_Delay(5000);
//		HAL_UART_DeInit(&huart3);
//
//	}

	BSP_LED_Init(LED_GREEN);

	g726_init_state(&g726encode);

	uint8_t rtpFrame[172];
	RTP_AddHeader(rtpFrame);

	if (f_mount(&fs, "", 0) != FR_OK) {
	}
	/* Create/open a file, then write a string and close it */
	if (f_open(&fp, FILENAME, FA_READ | FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {

		UINT num_read = 0;
		SHORT audio[AUDIO_BUFFER_SIZE];
		BYTE alaw[AUDIO_BUFFER_SIZE];

		do {
			f_read(&fp, audio, AUDIO_BUFFER_SIZE * 2, &num_read);
			frameToAlaw(alaw, audio, AUDIO_BUFFER_SIZE);
			RTP_AddVarHeader(rtpFrame);
			memcpy(rtpFrame + 12, alaw, AUDIO_BUFFER_SIZE);
			HAL_UART_Init(&huart3);
			HAL_StatusTypeDef res;
			res = HAL_UART_Receive_DMA(&huart3, (uint8_t *) recvBuffer,
					strlen(recvBuffer));

			res = HAL_UART_Transmit_DMA(&huart3, (uint8_t*) rtpFrame, 172);

			HAL_Delay(15);
			HAL_UART_DeInit(&huart3);

		} while (num_read == AUDIO_BUFFER_SIZE * 2);

//		SHORT buff[BUFFER_SIZE];
//		BYTE alaw[BUFFER_SIZE];
//		BYTE g726[80];
//		UINT num_read = 0;
//		UINT num_write = 0;
//
//		if (f_open(&fpout, FILENAME_OUT, FA_WRITE | FA_CREATE_ALWAYS)
//				== FR_OK) {
//
//			do {
//				f_read(&fp, buff, BUFFER_SIZE * 2, &num_read);
//				if (num_read == BUFFER_SIZE * 2) {
////					uint32_t start = HAL_GetTick();
//					frameToAlaw(alaw, buff, BUFFER_SIZE);
////					frameTog726_16(g726, buff);
////					uint8_t end = (uint8_t) (HAL_GetTick() - start);
//					f_write(&fpout, alaw, BUFFER_SIZE, &num_write);
//				}
//
//			} while (num_read == BUFFER_SIZE * 2);
//
//			f_close(&fpout);
//		}
//		f_close(&fp);

		BSP_LED_On(LED_GREEN);
	}

	while (1) {
	}

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/**Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE()
	;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV6;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
	PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	/**Configure the Systick
	 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART3 init function */
static void MX_USART3_UART_Init(void) {

	huart3.Instance = USART3;
	huart3.Init.BaudRate = 576000;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {
	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE()
	;

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

}

static void MX_GPIO_Init(void) {

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE()
	;
	__HAL_RCC_GPIOH_CLK_ENABLE()
	;
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;
	__HAL_RCC_GPIOG_CLK_ENABLE()
	;

}

void _Error_Handler(char *file, int line) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
