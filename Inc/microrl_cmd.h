/*
 * microrl_cmd.h
 *
 *  Created on: Oct 28, 2018
 *      Author: makso
 */

#ifndef MICRORL_CMD_H_
#define MICRORL_CMD_H_

#define _VER "DCF77 ver 1.0"

int print_help 		(int argc, const char * const * argv);
int clear_screen 	(int argc, const char * const * argv);
int led_on 			(int argc, const char * const * argv);
int led_off 		(int argc, const char * const * argv);
int led_toggle 		(int argc, const char * const * argv);
int time_show 		(int argc, const char * const * argv);
int time_show_simple(int argc, const char * const * argv);
int time_set 		(int argc, const char * const * argv);
int print_time 		(int argc, const char * const * argv);


#define MICRORL_CMD_LENGTH (10)
#define MICRORL_HELP_MSG_LENGTH (44)

typedef struct{
	char cmd	  [MICRORL_CMD_LENGTH];
	char friend	  [MICRORL_CMD_LENGTH];
	char parent	  [MICRORL_CMD_LENGTH];
	char help_msg [MICRORL_HELP_MSG_LENGTH];
	int (*func)   (int argc, const char * const * argv );
} microrl_action_t;

const microrl_action_t microrl_actions [] =
{
		{"help", 	"", 		"", 	"this message", 	print_help},
		{"h", 		"help",		"", 	"", 				print_help},
		{"?", 		"help", 	"", 	"", 				print_help},
		{"clear", 	"",			"", 	"clear screen", 	clear_screen},
		{"h", 		"clear",	"", 	"clear screen", 	clear_screen},
		{"?", 		"clear",	"", 	"clear screen", 	clear_screen},
		{"led",		"",			"",		"switch led",		NULL},
		{ "on",		"",			"led",	"turn on",			led_on},
		{ "off",	"",			"led",	"turn off",			led_off},
		{ "",		"",			"led",	"empty - toggle",	led_toggle},
		{"time",	"",			"",		"print time",		NULL},
		{ "show", 	"",			"time",	"auto update",		time_show},
		{  "simple","",			"show",	"auto for logging",	time_show_simple},
		{ "set",	"",			"time", "time set hh:mm:ss",time_set},
		{ "",		"",			"time",	"once",				print_time}
};

#define microrl_actions_length (sizeof(microrl_actions)/sizeof(microrl_action_t))

/*// definition commands word
#define _CMD_HELP   "help"
#define _CMD_H		"h"
#define _CMD_Q		"?"
#define _CMD_CLEAR  "clear"
#define _CMD_LED	"led"
// arguments for set/clear
	#define _SCMD_ON  "on"
	#define _SCMD_OFF  "off"
#define _CMD_TIME	"time"

#define _NUM_OF_CMD 4
#define _NUM_OF_LED_SCMD 2

//available  commands
char * keyword [] = {_CMD_HELP, _CMD_H, _CMD_CLEAR, _CMD_LED, _CMD_TIME};
// 'set/clear' command argements
char * on_off_key [] = {_SCMD_ON, _SCMD_OFF};*/

// array for comletion
char * compl_word [microrl_actions_length + 1];

#define COLOR_NC				"\e[0m" 	// default
#define COLOR_WHITE				"\e[1;37m"
#define COLOR_BLACK				"\e[0;30m"
#define COLOR_BLUE				"\e[0;34m"
#define COLOR_LIGHT_BLUE		"\e[1;34m"
#define COLOR_GREEN				"\e[0;32m"
#define COLOR_LIGHT_GREEN		"\e[1;32m"
#define COLOR_CYAN				"\e[0;36m"
#define COLOR_LIGHT_CYAN		"\e[1;36m"
#define COLOR_RED				"\e[0;31m"
#define COLOR_LIGHT_RED			"\e[1;31m"
#define COLOR_PURPLE			"\e[0;35m"
#define COLOR_LIGHT_PURPLE		"\e[1;35m"
#define COLOR_BROWN				"\e[0;33m"
#define COLOR_YELLOW			"\e[1;33m"
#define COLOR_GRAY				"\e[0;30m"
#define COLOR_LIGHT_GRAY		"\e[0;37m"

#endif /* MICRORL_CMD_H_ */
