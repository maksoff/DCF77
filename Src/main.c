
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

/*
 * change this code in hal_rtc
 * Set updated time in decreasing counter by number of days elapsed
 * counter_time -= (days_elapsed * 24U * 3600U - 1);
 */
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
microrl_t mcrl;
microrl_t * p_mcrl = &mcrl;

bool CDC_is_ready = false;
bool rtc_sec_irq_armed = false;
uint32_t tick_delay;
bool show_time = true;
bool show_date = false;
bool show_simple_time = true;
bool led_tack = false;
bool led_bypass_dcf77 = true;
bool led_display_dcf77 = false;
bool secf_flag = false;
bool first_minute = true;
bool time_synced = false;
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
	rtc_sec_irq_armed = true;
	tick_delay = HAL_GetTick();
}

/*
 * seconds interrupt comes for the second change! RM0008 page 486 fig 180
 * so, we make 1 ms delay to be sure the new second is here
 */

bool poll_second_update (void)
{
		  if (rtc_sec_irq_armed && (HAL_GetTick() - tick_delay > 1))
		  {
			  rtc_sec_irq_armed = false;
			  return true;
		  }
		  return false;
}

void process_cdc_input_data(uint8_t* Buf, uint32_t *Len)
{
	for (uint32_t i = 0; i < (*Len); i++)
		fifo_push((buff_t) Buf[i]);
}

void print (const char * str)
{
	if (!CDC_is_ready)
		return;
	uint16_t len = 0;
	while (str[++len] != 0);
	uint32_t timeout = HAL_GetTick();
	while (((USBD_CDC_HandleTypeDef*)(hUsbDeviceFS.pClassData))->TxState!=0)
		if (HAL_GetTick() - timeout >= 5)
			break;
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

/*
 * @param pointer char str[9]
 */
void time_to_string(char * str)
{
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	str[0] = (((time.Hours)&0xF0)>>4) + '0';
	str[1] = (((time.Hours)&0x0F)>>0) + '0';
	str[2] = ':';
	str[3] = (((time.Minutes)&0xF0)>>4) + '0';
	str[4] = (((time.Minutes)&0x0F)>>0) + '0';
	str[5] = ':';
	str[6] = (((time.Seconds)&0xF0)>>4) + '0';
	str[7] = (((time.Seconds)&0x0F)>>0) + '0';
	str[8] = '\0';

}

int print_time (int argc, const char * const * argv)
{
	if (argc != 11) // TODO dirty hack
	{
		show_time = false;
		show_date = false;
	}
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
		print_color("on", C_L_GREEN);
	print(ENDL);
	return 0;
}
int led_on 			(int argc, const char * const * argv)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
	led_show(argc, argv);
	return 0;
}
int led_off 		(int argc, const char * const * argv)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
	led_show(argc, argv);
	return 0;
}
int led_toggle 		(int argc, const char * const * argv)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	led_show(argc, argv);
	return 0;
}


int led_tick 		(int argc, const char * const * argv)
{
	led_tack ^= 1;
	return 0;
}


int led_dcf77 		(int argc, const char * const * argv)
{
	if (argc > 2)
	{
		led_tack = false;
		led_display_dcf77 = true;
		return 0;
	}
	led_tack = false;
	led_bypass_dcf77 ^= 1;
	return 0;
}


int time_show 		(int argc, const char * const * argv)
{
	show_simple_time = ((argc > 2) && (strcmp(argv[2], "simple") == 0));
	print_color("Ctrl+C or 'time' to terminate", C_L_RED);
	print(ENDL);
	show_time = true;
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
	RTC_TimeTypeDef sTime;
	if (argc != 3)
	{
		print (COLOR_RED);
		print ("enter time in hh:mm:ss format, ex: 'time set 18:03:22'");
		print (ENDL);
		print ("time updated on enter");
		print (COLOR_NC);
		print (ENDL);
		return 1;
	}
	sTime.Hours   = ((argv[2][0] - '0') << 4) | (argv[2][1] - '0');
	sTime.Minutes = ((argv[2][3] - '0') << 4) | (argv[2][4] - '0');
	sTime.Seconds = ((argv[2][6] - '0') << 4) | (argv[2][7] - '0');
	if (sTime.Hours > 0x23)
		sTime.Hours = 0x23;
	if (sTime.Minutes > 0x59)
		sTime.Minutes = 0x59;
	if (sTime.Seconds > 0x59)
		sTime.Seconds = 0x59;
	HAL_RTC_SetTime (&hrtc,  &sTime, RTC_FORMAT_BCD);
	print_time(argc, argv);
	return 0;
}

int date_show 		(int argc, const char * const * argv)
{
	show_simple_time = ((argc > 2) && (strcmp(argv[2], "simple") == 0));
	print_color("Ctrl+C or 'time' to terminate", C_L_RED);
	print(ENDL);
	show_date = true;
	show_time = true;
	return 0;
}

int date_set 		(int argc, const char * const * argv)
{
	RTC_DateTypeDef sDate;
	if (argc != 3)
	{
		print (COLOR_RED);
		print ("enter date in YYYY.MM.DD format, ex: 'date set 2018.11.02'");
		print (COLOR_NC);
		print (ENDL);
		return 1;
	}
	sDate.Year   = ((argv[2][2] - '0') << 4) | (argv[2][3] - '0');
	sDate.Month  = ((argv[2][5] - '0') << 4) | (argv[2][6] - '0');
	sDate.Date   = ((argv[2][8] - '0') << 4) | (argv[2][9] - '0');
	if (sDate.Year > 0x99)
		sDate.Year = 0x99;
	if (sDate.Month > 0x12)
		sDate.Month = 0x12;
	if (sDate.Date > 0x31)
		sDate.Date = 0x31;
	HAL_RTC_SetDate (&hrtc, &sDate, RTC_FORMAT_BCD);
	print_date(argc, argv);
	return 0;
}

/*
 * @param pointer char str[11]
 */
void date_to_string (char * str)
{
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate (&hrtc, &sDate, RTC_FORMAT_BCD);
	str[0] = '2';
	str[1] = '0';
	str[2] = ((sDate.Year & 0xF0) >> 4) + '0';
	str[3] = (sDate.Year & 0x0F) + '0';
	str[4] = '.';
	str[5] = ((sDate.Month & 0xF0) >> 4) + '0';
	str[6] = (sDate.Month & 0x0F) + '0';
	str[7] = '.';
	str[8] = ((sDate.Date & 0xF0) >> 4) + '0';
	str[9] = (sDate.Date & 0x0F) + '0';
	str[10] = '\0';
}

int print_weekday (int argc, const char * const * argv)
{
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate (&hrtc, &sDate, RTC_FORMAT_BCD);
	char str[2];
	str[0] = (sDate.WeekDay + '0');
	str[1] = '\0';
	print (str);
	print (ENDL);
	return 0;
}

void day_to_string (char * str)
{
	RTC_DateTypeDef sDate;
	HAL_RTC_GetDate (&hrtc, &sDate, RTC_FORMAT_BCD);

}

int print_date 		(int argc, const char * const * argv)
{
	char str[10];
	date_to_string(str);
	print(COLOR_LIGHT_BLUE);
	print(str);
	print(COLOR_NC);
	print(ENDL);
	return 0;
}


int echo_toggle 	(int argc, const char * const * argv)
{
	return 0;
}

int echo_on 		(int argc, const char * const * argv)
{
	return 0;
}

int echo_off 		(int argc, const char * const * argv)
{
	return 0;
}

int echo_show 		(int argc, const char * const * argv)
{
	return 0;
}

int color_toggle 	(int argc, const char * const * argv)
{
	return 0;
}

int color_on 		(int argc, const char * const * argv)
{
	return 0;
}

int color_off 		(int argc, const char * const * argv)
{
	return 0;
}

int color_show 		(int argc, const char * const * argv)
{
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
	int synonym_level = 0;
	bool tokens_found = false;
	for (int i = 0; i < argc; i++)
	{
		for (int n = last_main_synonym; n < microrl_actions_length; n++)
		{
			tokens_found = false;
			int current_level = microrl_actions[n].level;
			// next higher level command found, break;
			if (current_level != -1)
				synonym_level = current_level; // save the synonym level
			if ((current_level != -1) && (current_level < i))
				break;
			if (current_level == i)
				last_main_synonym = n;
			if ((strcmp(argv[i], microrl_actions[n].cmd) == 0) &&
					(i == synonym_level))
			{
				tokens_found = true;
				func = microrl_actions[last_main_synonym++].func;
				break;
			}
		}
		if (!tokens_found)	// nothing found, nothing to do here
			break;
	}

	if (func != NULL)
	{
		return func(argc, argv); // function found
	} else if (tokens_found)
	{
		print_color ("command: '", C_L_RED);
		print_color ((char*)argv[0], C_L_RED);
		print_color ("' needs additional arguments", C_L_RED);
		print(ENDL);
		print_color ("use '", C_NC);
		print_color ("?", C_GREEN);
		print_color ("' for help", C_NC);
		print (ENDL);
		return 1;
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

	/*
	 * print cmd and synonyms with level == argc-1.
	 * if argc == 0 print without synonyms
	 */

	if (argc == 0)
	{
		// if there is no token in cmdline, just print all available token
		for (int i = 0; i < microrl_actions_length; i++) {
			if (microrl_actions[i].level == 0)
			compl_word[j++] = (char*) microrl_actions [i].cmd;
		}
	} else {
		// get last entered token
		char * bit = (char*)argv [argc-1];
		// iterate through our available token and match it
		// based on previous tokens in the line, find the correct one shift
		int last_main_synonym = 0;
		int synonym_level = 0;
		bool tokens_found = false;
		for (int i = 0; i < argc; i++)
		{
			for (int n = last_main_synonym; n < microrl_actions_length; n++)
			{
				tokens_found = false;
				int current_level = microrl_actions[n].level;
				// next higher level command found, break;
				if (current_level != -1)
					synonym_level = current_level; // save the synonym level
				if ((current_level != -1) && (current_level < i))
					break;
				if (current_level == i)
					last_main_synonym = n;
				if ((i == argc - 1) && (strstr(microrl_actions [n].cmd, bit) == microrl_actions [n].cmd) &&
										(i == synonym_level))
				{
					tokens_found = true;
					compl_word [j++] =(char*) microrl_actions [n].cmd;
				}
				else if ((strcmp(argv[i], microrl_actions[n].cmd) == 0) && (i == synonym_level))
				{
					last_main_synonym++;
					tokens_found = true;
					break;
				}
			}
			if (!tokens_found)	// nothing found, nothing to do here
				break;
		}
	}

	// note! last ptr in array always must be NULL!!!
	compl_word [j] = NULL;
	// return set of variants
	return compl_word;
}
#endif


void sigint (void)
{
	show_time = false;
	show_date = false;
	show_simple_time = false;
	led_display_dcf77 = false;
	print (ENDL);
	print ("^C catched!");
	int i = 0;
	while (ENTER[i])
		microrl_insert_char(p_mcrl, ENTER[i++]);
}

uint32_t count_bits_in_u32(uint32_t i){
	i -= ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void print_u32 (uint32_t dig)
{
	char str [6];
	for (int i = 0; i < 5; i++)
	{
		str[4 - i] = dig % 10 + '0';
		dig /= 10;
	}
	str[5] = '\0';
	print (str);
}
void print_x8 (uint8_t dig)
{
	char str [5];
	str[0] = '0';
	str[1] = 'x';
	str[4] = '\0';
	str[3] = (dig & 0x0F) + '0';
	if (str[3] > '9')
		str[3] = str[3] - '0' - 10 + 'A';
	dig >>= 4;
	str[2] = (dig & 0x0F) + '0';
	if (str[2] > '9')
		str[2] = str[2] - '0' - 10 + 'A';
	print(str);
}
void calculate_time(bool * time_array)
{
	RTC_TimeTypeDef sTime;
	sTime.Hours = 0;
	sTime.Minutes = 0;
	sTime.Seconds = 0;
	for (int i = 0; i < 6; i++)
	{
		sTime.Hours   |= (time_array[29 + i] << i);
	}
	print(" H: ");
	print_x8(sTime.Hours);
	print("; ");
	if ((sTime.Hours > 0x23) || ((sTime.Hours & 0x0F) > 0x9))
		return;

	for (int i = 0; i < 7; i++)
	{
		sTime.Minutes   |= (time_array[21 + i] << i);
	}
	print(" m: ");
	print_x8(sTime.Minutes);
	print("; ");

	if ((sTime.Minutes > 0x59) || ((sTime.Minutes & 0x0F) > 0x9))
		return;

	RTC_DateTypeDef sDate;
	sDate.Year = 0;
	sDate.Month = 0;
	sDate.Date = 0;

	for (int i = 0; i < 8; i++)
	{
		sDate.Year   |= (time_array[50 + i] << i);
	}
	print(" Y: ");
	print_x8(sDate.Year);
	print("; ");

	for (int i = 0; i < 5; i++)
	{
		sDate.Month   |= (time_array[45 + i] << i);
	}
	print(" M: ");
	print_x8(sDate.Month);
	print("; ");

	for (int i = 0; i < 6; i++)
	{
		sDate.Date   |= (time_array[36 + i] << i);
	}
	print(" d: ");
	print_x8(sDate.Date);
	print("; ");

	if ((sDate.Year  > 0x99) || ((sDate.Year  & 0x0F) > 0x09))
		return;
	if ((sDate.Month > 0x12) || ((sDate.Month & 0x0F) > 0x09))
		return;
	if ((sDate.Date  > 0x31) || ((sDate.Date  & 0x0F) > 0x09))
		return;

//	bool check_minutes;
//	for (int i = 0; i < 7; i++)
//	{
//
//	}
	HAL_RTC_SetTime (&hrtc,  &sTime, RTC_FORMAT_BCD);
	HAL_RTC_SetDate (&hrtc, &sDate, RTC_FORMAT_BCD);
	print_color(" sync", C_YELLOW);
}

void process_time (void)
{
	static uint32_t time_delay = 0;
	static uint32_t zero_cnt = 0, one_cnt = 0;
	static uint32_t pos_cnt = 0;
	static bool time_array[60];
	static bool bad_minute = false;
	static bool first_minute = true;
	bool time_bit = HAL_GPIO_ReadPin(DCF77_GPIO_Port, DCF77_Pin);
	if ((HAL_GetTick() - time_delay >= 10) || (time_bit && (zero_cnt > 170)))
		  {
			  time_delay = HAL_GetTick();
			  if (time_bit)
			  {
//				  if (zero_cnt)
//				  {
//					  print(" ");
//					  print_time(0,0);
////					  print ("    0: ");
////					  print_u32(zero_cnt);
////					  print (ENDL);
//				  }
				  if ((170 < zero_cnt) && (zero_cnt < 200))
				  {
					  print(" POS: ");
					  print_u32(pos_cnt);
					  print("; ");
					  if (bad_minute)
						  print_color(" BAD; ", C_RED);
					  if ((!bad_minute) && (pos_cnt >= 58))
						  calculate_time(time_array);
					  bad_minute = false;
					  pos_cnt = 0;
					  print(ENDL);
					  char time[9];
					  char date[11];
					  print(COLOR_BROWN);
					  time_to_string (time);
					  date_to_string(date);
					  print(date);
					  print(" ");
					  print(time);
					  print(COLOR_NC);
					  print(ENDL);
					  print(ENDL);
				  } else if ((70 <= zero_cnt) && (zero_cnt <= 94) && (!bad_minute))
				  {
					  if (pos_cnt >= 59)
						  bad_minute = true;
					  else
						  pos_cnt++;
				  } else if (zero_cnt)
				  {
					  bad_minute = true;
				  }
				  one_cnt++;
				  zero_cnt = 0;
			  } else {
				  if (one_cnt)
				  {
					  print_time(11, 0);
					  print(" ");
				  }
				  if ((7 <= one_cnt) && (one_cnt <= 14))
				  {
					  time_array[pos_cnt] = false;
//					  print("0");
				  } else if ((16 <= one_cnt) && (one_cnt <= 24))
				  {
					  time_array[pos_cnt] = true;
//					  print("1");
				  } else if (one_cnt) {
					  print_color("-", C_RED);
					  print(COLOR_RED);
					  print_u32(one_cnt);
					  print_color("-", C_RED);
					  bad_minute = true;
				  }
				  zero_cnt++;
				  one_cnt = 0;
			  }
//
//			  if (time_bit)
//				  print("|");
//			  else
//				  print(".");
		  }
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
	  process_time();

	  while (!fifo_is_empty())
		  microrl_insert_char(p_mcrl, (int) fifo_pop());

	  if (led_bypass_dcf77)
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1^HAL_GPIO_ReadPin(DCF77_GPIO_Port, DCF77_Pin));

	  if (poll_second_update ())
	  {
		  if (show_time)
		  {
			  if (!show_simple_time)
				  print("\r ");
			  else
				  print (ENDL);
			  char time[9];
			  char date[11];
			  time_to_string (time);
			  if(show_date)
				  date_to_string(date);
			  if (!show_simple_time)
				  print(COLOR_PURPLE);
			  if (show_date)
			  {
				  print(date);
				  print(" ");
			  }
			  print(time);
			  if (!show_simple_time)
			  {
				  print(COLOR_NC);
				  print("\r");
			  }
		  }

		  if (led_tack)
		  {
			  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			  HAL_Delay(100);
			  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		  }
	  }
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
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x18;

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
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

  /*Configure GPIO pin : DCF77_Pin */
  GPIO_InitStruct.Pin = DCF77_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DCF77_GPIO_Port, &GPIO_InitStruct);

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
