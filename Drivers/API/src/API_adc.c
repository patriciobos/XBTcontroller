/*
 * API_adc.c
 *
 *  Created on: 25 mar. 2022
 *      Author: patricio
 */

/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "API_adc.h"
#include "API_uart.h"
#include "API_delay.h"


/** @addtogroup STM32F4xx_HAL_Examples
 * @{
 */

/** @addtogroup ADC_RegularConversion_DMA
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
typedef enum {ACQUIRING, PROCESSING} FSM_ADC_state_t;



/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

#define ADC_TIMEOUT 0xFFFF

#define COUNT2AVERAGE 10

/* Private variables ---------------------------------------------------------*/

/* FSM state variable declaration*/
FSM_ADC_state_t fsm_adc_state;

/* ADC handler declaration */
ADC_HandleTypeDef AdcHandle;

/* Variable used to get converted value */
__IO uint16_t uhADCxConvertedValue = 0;

static uint16_t values2average[COUNT2AVERAGE];
//static const uint8_t count2average = sizeof(values2average)/sizeof(uint16_t);

static bool onEntry = true;

static delay_t ADCdelay;

/* Private function prototypes -----------------------------------------------*/

static void Error_Handler(void);
/* Private functions ---------------------------------------------------------*/

void FSM_ADC_Init(){

	fsm_adc_state = ACQUIRING;

	uint8_t i;

	//ADC_init();

	for(i=0;i<COUNT2AVERAGE;i++){
		values2average[i] = 0;
	}

	delayInit(&ADCdelay, 1000);

}


void FSM_ADC_Handler(){

	uint8_t i, index;
	uint32_t sum, avg_value;
	//char buffer[255];

	switch (fsm_adc_state){

	case ACQUIRING:
		if(onEntry){
			onEntry = false;
			index = 0;
			ADC_start();
		}

		if (HAL_ADC_PollForConversion(&AdcHandle, ADC_TIMEOUT) == HAL_OK) {
			values2average[index] = HAL_ADC_GetValue(&AdcHandle);
			index++;

		}

		if (index >= COUNT2AVERAGE){
			//index = 0;
			fsm_adc_state = PROCESSING;
			onEntry = true;
			ADC_stop();
		}

		break;

	case PROCESSING:
		if(onEntry) {

			onEntry = false;

			sum = values2average[0];

			for(i=1;i<COUNT2AVERAGE;i++){
				sum = sum + values2average[i];
			}

			avg_value = sum / COUNT2AVERAGE;


			printf("avg_value = %d\n\r", avg_value);


		}

		if (delayRead(&ADCdelay)){
			fsm_adc_state = ACQUIRING;
			onEntry = true;
			BSP_LED_Toggle(LED1);


		}

		break;

	default:
		break;

	}
}

void ADC_Init(){

	GPIO_InitTypeDef          GPIO_InitStruct;
	//static DMA_HandleTypeDef  hdma_adc;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* ADC1 Periph clock enable */
	ADCx_CLK_ENABLE();
	/* Enable GPIO clock ****************************************/
	ADCx_CHANNEL_GPIO_CLK_ENABLE();
	/* Enable DMA2 clock */
	//	DMAx_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* ADC Channel GPIO pin configuration */
	GPIO_InitStruct.Pin = ADCx_CHANNEL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADCx_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

	ADC_ChannelConfTypeDef sConfig;

	/*##-1- Configure the ADC peripheral #######################################*/
	AdcHandle.Instance                   = ADCx;
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV8;
	AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
	AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	AdcHandle.Init.ContinuousConvMode    = ENABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.NbrOfDiscConversion   = 0;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Conversion start trigged at each external event */
	AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion       = 1;
	AdcHandle.Init.DMAContinuousRequests = DISABLE;
	AdcHandle.Init.EOCSelection          = DISABLE;

	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		/* ADC initialization Error */
		Error_Handler();
	}

	/*##-2- Configure ADC regular channel ######################################*/
	sConfig.Channel      = ADC_CHANNEL_10;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.Offset       = 0;

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		Error_Handler();
	}

	printf("ADC init [OK]\n\r");

}

/**
 * @brief  Start the conversion process.
 * @param  None
 * @retval None
 * @note: Considering IT occurring after each number of ADC conversions
 *       (IT by DMA end of transfer), select sampling time and ADC clock
 *       with sufficient duration to not create an overhead situation in
 *        IRQHandler.
 */

/* I/O operation functions ******************************************************/
/*HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef* hadc);
 */


HAL_StatusTypeDef ADC_getValue(uint32_t *adcValue){

	HAL_StatusTypeDef returnValue;

	returnValue = HAL_ADC_PollForConversion(&AdcHandle, ADC_TIMEOUT);

	if(returnValue == HAL_OK)
		*adcValue = HAL_ADC_GetValue(&AdcHandle);

	return returnValue;
}

void ADC_start(){

	if(HAL_ADC_Start(&AdcHandle) != HAL_OK)
	{
		/* Start Conversation Error */
		Error_Handler();
	}
}

/**
 * @brief  Stop the conversion process.
 * @param  None
 * @retval None
 */

void ADC_stop(){

	if(HAL_ADC_Stop(&AdcHandle) != HAL_OK)
	{
		/* Stop Conversation Error */
		Error_Handler();
	}

}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void)
{
	/* Turn LED2 on */
	BSP_LED_On(LED2);
	while (1)
	{
	}
}


/**
 * @brief  Conversion complete callback in non blocking mode
 * @param  AdcHandle : AdcHandle handle
 * @note   This example shows a simple way to report end of conversion, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	/* Turn LED1 on: Transfer process is correct */
	//BSP_LED_On(LED1);
	/* Get the converted value of regular channel */
	uhADCxConvertedValue = HAL_ADC_GetValue(AdcHandle);
	//values2average[index%10] = HAL_ADC_GetValue(AdcHandle);

}
