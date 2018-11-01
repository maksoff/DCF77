
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "microrl.h"
#include "microrl_cmd.h"
#include "usbd_cdc_if.h"
#include "SEGGER_RTT.h"
#include "fifo.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
microrl_t mcrl;
microrl_t * p_mcrl = &mcrl;

RTC_TimeTypeDef time;
RTC_TimeTypeDef *p_time = &time;

bool CDC_is_ready = false;
bool tick = false;
bool show_time = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void process_cdc_input_data(uint8_t* Buf, uint32_t *Len);
void print (const char * str);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	tick = true;
}
void process_cdc_input_data(uint8_t* Buf, uint32_t *Len)
{
//	uint8_t buff[2];
//	buff[0] = Buf[0]/10+'0';
//	buff[1] = Buf[0]%10+'0';
//	CDC_Transmit_FS(buff,2);
//	return;
	for (uint32_t i = 0; i < (*Len); i++)
		fifo_push((buff_t) Buf[i]);
}

void print (const char * str)
{
	if (!CDC_is_ready)
		return;
	uint16_t len = 0;
	while (str[++len] != 0);
	while (((USBD_CDC_HandleTypeDef*)(hUsbDeviceFS.pClassData))->TxState!=0);
	CDC_Transmit_FS((uint8_t*)str, len);

#if defined (SEGGER_RTT_PRINT)
	char test_str[256];
	len = 0;
	uint16_t i = 0;
	while (str[len] != 0)
	{
		if (str[len] >= ' ')
			test_str[i++] = str[len];
		len++;
	}
	test_str[i] = '\0';
	SEGGER_RTT_WriteString(0,test_str);
#endif
}

int find_color_by_name(microrl_color_e color)
{
	for (int i = 0; i < microrl_color_lookup_length; i++)
	{
		if (microrl_color_lookup[i].name == color)
		{
			return i;
		}
	}
	return 0;
}

int print_color(const char * str, microrl_color_e color)
{
	print(microrl_color_lookup[find_color_by_name(color)].code);
	print(str);
	print(COLOR_NC);
	return 0;
}

int str_length(const char * str)
{
	int i = 0;
	while (str[i])
		i++;
	return i;
}

//bool has_friend(int i)
//{
//	assert_param(i < microrl_actions_length);
//	return microrl_actions[i].friend[0] != '\0';
//}
//bool true_friends(int cmd, int friend)
//{
//	assert_param(cmd < microrl_actions_length);
//	assert_param(friend  < microrl_actions_length);
//	return ((strcmp(microrl_actions[cmd].cmd, 	microrl_actions[friend].friend) == 0) &&
//			(strcmp(microrl_actions[cmd].parent, microrl_actions[friend].parent) == 0));
//}
//
//int find_true_friend(int friend)
//{
//	assert_param(friend < microrl_actions_length);
//	for (int i = 0; i < microrl_actions_length; i++)
//		if (true_friends(i, friend))
//			return i;
//	return friend;
//}
//
//bool true_cmd_name(int i, const char * cmd, const char * parent)
//{
//	assert_param(i < microrl_actions_length);
//	return ((strcmp(microrl_actions [i].cmd, cmd) == 0) &&
//			(strcmp(microrl_actions [i].parent, parent) == 0));
//}
//
//int find_cmd_num(const char * cmd, const char * parent)
//{
//	for (int i = 0; i < microrl_actions_length; i++)
//		if (true_cmd_name (i, cmd, parent))
//			return i;
//	return 0;
//}

int print_help(int argc, const char * const * argv)
{
	print(_VER);
	print(ENDL);
	print ("Use ");
	print_color("TAB", C_GREEN);
	print(" key for completion");
	print (ENDL);
	print ("Available commands:");
	for (int i = 0; i < microrl_actions_length; i++)
	{
		if (microrl_actions[i].level == -1) // print synonyms
		{
			assert_param(i > 0);
			if (microrl_actions[i - 1].level != -1)
				print_color(" aka ", C_L_PURPLE);
			else
				print_color("/", C_L_PURPLE);
			print_color (microrl_actions[i].cmd, C_PURPLE);
		}
		else
		{
			print(ENDL);
			for (int e = -4; e < microrl_actions[i].level; e++)
				print(" ");
			print_color(microrl_actions[i].cmd, microrl_help_color[microrl_actions[i].level]);
			for (int e = 0; e < MICRORL_CMD_LENGTH + 2 -
								microrl_actions[i].level - str_length(microrl_actions[i].cmd); e++)
				print (" ");
			switch (microrl_actions[i].level){
			case 0:
				print ("-");
				break;
			case 1:
				print ("^");
				break;
			default:
				print ("#");
				break;
			}
			print (" ");
			print (microrl_actions[i].help_msg);
		}
	}
	print(ENDL);
	return 0;
}

int clear_screen(int argc, const char * const * argv)
{
	print ("\033[2J");    // ESC seq for clear entire screen
	print ("\033[H");     // ESC seq for move cursor at left-top corner
	return 0;
}

void time_to_string(char * str)
{
	HAL_RTC_GetTime(&hrtc, p_time, RTC_FORMAT_BCD);
	str[0] = (((p_time->Hours)&0xF0)>>4) + '0';
	str[1] = (((p_time->Hours)&0x0F)>>0) + '0';
	str[2] = ':';
	str[3] = (((p_time->Minutes)&0xF0)>>4) + '0';
	str[4] = (((p_time->Minutes)&0x0F)>>0) + '0';
	str[5] = ':';
	str[6] = (((p_time->Seconds)&0xF0)>>4) + '0';
	str[7] = (((p_time->Seconds)&0x0F)>>0) + '0';
	str[8] = '\0';

}

int print_time (int argc, const char * const * argv)
{
	char str[9];
	time_to_string (str);
	print(COLOR_LIGHT_BLUE);
	print(str);
	print(COLOR_NC);
	print(ENDL);
	return 0;
}
int led_show 		(int argc, const char * const * argv)
{
	print_color("LED is ", C_L_BLUE);
	if (HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin))
		print_color("off", C_RED);
	else
		print_color("on", C_BLUE);
	print(ENDL);
	return 0;
}
int led_on 			(int argc, const char * const * argv)
{
	print("led_on");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}
int led_off 		(int argc, const char * const * argv)
{
	print("led_off");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}
int led_toggle 		(int argc, const char * const * argv)
{
	print("led_toggle");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}
int time_show 		(int argc, const char * const * argv)
{
	print("time_show");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}
int time_show_simple(int argc, const char * const * argv)
{
	print("time_show_simple");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}
int time_set 		(int argc, const char * const * argv)
{
	print("time_set");
	print(ENDL);
	for (int i = 0; i < argc; i++)
	{
		print_color(argv[i], C_L_BLUE);
		print(" ");
	}
	print(ENDL);
	return 0;
}

int execute (int argc, const char * const * argv)
{
//	print_help(argc, argv);
//	return 0;
	int (*func)   (int argc, const char * const * argv ) = NULL;

	/*
	 * iterate throw levels and synonyms - run the func from the first (main) synonym
	 * run last found functions with all parameters - functions should check or ignore additional parameters
	 * if nothing found - show err msg
	 */

	int last_main_synonym = 0;
	for (int i = 0; i < argc; i++)
	{
		for (int n = last_main_synonym; n < microrl_actions_length; n++)
		{
			int current_level = microrl_actions[n].level;
			// next higher level command found, break;
			if ((current_level != -1) && (current_level < i))
				break;
			if (current_level == i)
				last_main_synonym = n;
			if (strcmp(argv[i], microrl_actions[n].cmd) == 0)
			{
				func = microrl_actions[last_main_synonym++].func;
				break;
			}
		}
		if (func == NULL)	// nothing found, nothing to do here
			break;
	}

	if (func != NULL)
	{
		return func(argc, argv); // function found
	}
	else
	{
		print_color ("command: '", C_RED);
		print_color ((char*)argv[0], C_RED);
		print_color ("' not found", C_RED);
		print(ENDL);
		print_color ("use '", C_NC);
		print_color ("?", C_GREEN);
		print_color ("' for help", C_NC);
		print (ENDL);
		return 1;

	}
}

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet (int argc, const char * const * argv)
{
	int j = 0;

	compl_word [0] = NULL;

	/*
	 * if no parameters - print all cmd with friend =="" && father == ""
	 * if parameter == 1 search with parent == ""
	 * if parameter > 1 search with parent == (parameter-2)
	 */

	/*if (argc == 0)
	{
		// if there is no token in cmdline, just print all available token
		for (int i = 0; i < microrl_actions_length; i++) {
			if (microrl_actions[i].friend[0] == '\0' && microrl_actions[i].parent[0] == '\0')
			compl_word[j++] = (char*) microrl_actions [i].cmd;
		}
	} else {
		// get last entered token
		char * bit = (char*)argv [argc-1];
		// iterate through our available token and match it
		for (int i = 0; i < microrl_actions_length; i++) {
			// if token is matched (text is part of our token starting from 0 char)
			if (strstr(microrl_actions [i].cmd, bit) == microrl_actions [i].cmd) {

				 * parent must be argv[argc-2] ("" if argc == 1)
				 * if argv[argc-2] has friend - parent == friend
				 * correct friend should hve the same parent

//				(((argc == 1) && strcmp(microrl_actions [i].parent, "") == 0) ||
//							     (argc > 1 && strcmp(microrl_actions [i].parent, argv[argc-2]) == 0))
				// c

				char empty_parrent = '\0';
				char * mh_parent = &empty_parrent;
				int nr = i;
//				if (argc > 1)
//				{
//					mh_parent = argv[argc - 2];
//					for (int n = 0; n < microrl_actions_length; n++)
//						if ((strcmp(microrl_actions[n].cmd, mh_parent) == 0) &&
//							(has_friend(n)))
//							mh_parent = microrl_actions[n].friend;
//					if (microrl_actions[i].friend[0] != '\0') // if has a friend, use func/cmd from friend
//					{
//						for (int j = 0; j < microrl_actions_length; j++)
//							if ((strcmp (microrl_actions[n].friend, microrl_actions[j].cmd) == 0) &&
//									(strcmp (microrl_actions[n].parent, microrl_actions[j].parent) == 0))
//								nr = j;
//					}
//				}
				// add it to completion set
				if (strcmp(microrl_actions [i].parent, mh_parent) == 0)
					compl_word [j++] =(char*) microrl_actions [i].cmd;
			}
		}
	}*/

	// note! last ptr in array always must be NULL!!!
	compl_word [j] = NULL;
	// return set of variants
	return compl_word;
}
#endif


void sigint (void)
{
	show_time = false;
	print (ENDL);
	print ("^C catched!");
	print (ENDL);
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  CDC_set_receive_callback(process_cdc_input_data);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  microrl_init(p_mcrl, print);
  // set callback for execute
  microrl_set_execute_callback (p_mcrl, execute);

#ifdef _USE_COMPLETE
  // set callback for completion
  microrl_set_complete_callback (p_mcrl, complet);
#endif
  // set callback for Ctrl+C
  microrl_set_sigint_callback (p_mcrl, sigint);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  SEGGER_RTT_Init();
  HAL_GPIO_WritePin(USB_UP_GPIO_Port, USB_UP_Pin, 0);
  HAL_Delay(1);
  CDC_is_ready = true;
  HAL_RTCEx_SetSecond_IT (&hrtc);
  for (int i = 51; i > 0 ; i--)
  {
	  HAL_Delay(100);
	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  }
  while (1)
  {
	  while (!fifo_is_empty())
		  microrl_insert_char(p_mcrl, (int) fifo_pop());
	  if (tick)
	  {
		  if (show_time)
		  {
			  print("\r ");
			  char str[9];
			  time_to_string (str);
			  print(COLOR_PURPLE);
			  print(str);
			  print(COLOR_NC);
			  print("\r");
		  }
		  tick = false;
	  }
//	  HAL_Delay(1000);
//	  print("This is test");
//	  print(ENDL);
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0x18;
  sTime.Minutes = 0x3;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_OCTOBER;
  DateToUpdate.Date = 0x29;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_UP_GPIO_Port, USB_UP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_UP_Pin */
  GPIO_InitStruct.Pin = USB_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_UP_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
