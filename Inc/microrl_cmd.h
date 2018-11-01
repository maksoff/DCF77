/*
 * microrl_cmd.h
 *
 *  Created on: Oct 28, 2018
 *      Author: makso
 */

#ifndef MICRORL_CMD_H_
#define MICRORL_CMD_H_

#define _VER "DCF77 ver 0.1"

int print_help 		(int argc, const char * const * argv);
int clear_screen 	(int argc, const char * const * argv);
int led_on 			(int argc, const char * const * argv);
int led_off 		(int argc, const char * const * argv);
int led_toggle 		(int argc, const char * const * argv);
int led_show 		(int argc, const char * const * argv);
int time_show 		(int argc, const char * const * argv);
int time_show_simple(int argc, const char * const * argv);
int time_set 		(int argc, const char * const * argv);
int print_time 		(int argc, const char * const * argv);

#define EMPTY_CMD_HELP "[]"

#define MICRORL_CMD_LENGTH (10)
#define MICRORL_HELP_MSG_LENGTH (44)

typedef struct{
	int level;									// 0: top, 1: next, 2: next next; -1: same functions as above
	char cmd	  [MICRORL_CMD_LENGTH];			// command name
	char help_msg [MICRORL_HELP_MSG_LENGTH];	// help message
	int (*func)   (int argc, const char * const * argv ); // pointer to function
} microrl_action_t;

/*
 * Ex. Menu Structure:						should be formated in this way:
 * help 	-- help message					{0, 	"help", "help message", print_help},
 * h		-- -//-							{-1, 	"h", 	"", 			NULL},
 * clr		-- clear screen					{0,		"clr",  "clear screen",	clear_scr},
 * led  	-- led toggle					{0, 	"led",  "led toggle,	led_toggle},
 * lamp  	-- -//-							{-1,	"lamp",	"",				NULL},
 *   on 	-- turn led on					{1, 	"on",	"turn led on",	led_on},
 *   off 	-- turn led off					{1, 	"off",	"turn led off",	led_off},
 * time		-- show time once				{0,		"time",	"show time once", print_time},
 *   show   -- autoupdate time				{1,		"show",	"autoupdate time", print_time_auto},
 *   auto	-- -//-							{-1,	"auto", "",				NULL},
 *     simple -- autoupdate without esc		{2,		"simple", "autoupdate without esc", print_time_no_esc}
 *
 * !      -//- == synonym for function above
 * !!!    order of lines is important! the alternative names and sublevel commands are referenced for function above.
 */

const microrl_action_t microrl_actions [] =
{
		{ 0, 		"help", 	"this message", 	print_help},
		{-1,		"h", 		"", 				NULL},
		{-1,		"?", 		"", 				NULL},
		{ 0,		"clear", 	"clear screen", 	clear_screen},
		{-1,		"clr", 		"", 				NULL},
		{-1,		"clrscr",	"", 				NULL},
		{ 0,		"led",		"toggle led",		led_toggle},
		{   1,		"on",		"turn on",			led_on},
		{   1,		"off",		"turn off",			led_off},
		{   1,		"show", 	"show led",			led_show},
		{     2,	"simple", 	"-no ESC-", 			led_show},
		{ 0,		"time",		"print time",		print_time},
		{   1,		"show", 	"auto update",		time_show},
		{  -1,		"auto",		"",					NULL},
		{     2,	"simple", 	"auto for logging",	time_show_simple},
		{ 	1,		"set",		"time set 'hh:mm:ss'",time_set},
		{  -1,		"enter",	"",					NULL}
};

#define microrl_actions_length (sizeof(microrl_actions)/sizeof(microrl_action_t))

// array for completion
char * compl_word [microrl_actions_length + 1];

#define COLOR_CODE_LENGTH		(9)

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

typedef enum {
	C_NC,
	C_WHITE,
	C_BLACK,
	C_BLUE,
	C_L_BLUE,
	C_GREEN,
	C_L_GREEN,
	C_CYAN,
	C_L_CYAN,
	C_RED,
	C_L_RED,
	C_PURPLE,
	C_L_PURPLE,
	C_BROWN,
	C_YELLOW,
	C_GRAY,
	C_L_GRAY
} microrl_color_e;

typedef struct {
	microrl_color_e name;
	char code[10];
} microrl_color_t;

const microrl_color_t microrl_color_lookup [] =
{
		{C_NC,		COLOR_NC},
		{C_WHITE,	COLOR_WHITE},
		{C_BLACK,	COLOR_BLACK},
		{C_BLUE,	COLOR_BLUE},
		{C_L_BLUE,	COLOR_LIGHT_BLUE},
		{C_GREEN,	COLOR_GREEN},
		{C_L_GREEN,	COLOR_LIGHT_GREEN},
		{C_CYAN,	COLOR_CYAN},
		{C_L_CYAN,	COLOR_LIGHT_CYAN},
		{C_RED,		COLOR_RED},
		{C_L_RED,	COLOR_LIGHT_RED},
		{C_PURPLE,	COLOR_PURPLE},
		{C_L_PURPLE,COLOR_LIGHT_PURPLE},
		{C_BROWN,	COLOR_BROWN},
		{C_YELLOW,	COLOR_YELLOW},
		{C_GRAY,	COLOR_GRAY},
		{C_L_GRAY,	COLOR_LIGHT_GRAY}
};

#define microrl_color_lookup_length (sizeof(microrl_color_lookup)/sizeof(microrl_color_t))

const microrl_color_e microrl_help_color [] =
{
		C_GREEN,
		C_L_GREEN,
		C_PURPLE,
		C_L_PURPLE
};

#define microrl_help_color_lenght (sizeof(microrl_help_color)/sizeof(microrl_color_e))

#endif /* MICRORL_CMD_H_ */
