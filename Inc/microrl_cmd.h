/*
 * microrl_cmd.h
 *
 *  Created on: Oct 28, 2018
 *      Author: makso
 */

#ifndef MICRORL_CMD_H_
#define MICRORL_CMD_H_

#define _VER "DCF77 ver 1.0"

// definition commands word
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
char * keyword [] = {_CMD_HELP, _CMD_CLEAR, _CMD_LED, _CMD_TIME};
// 'set/clear' command argements
char * on_off_key [] = {_SCMD_ON, _SCMD_OFF};

// array for comletion
char * compl_word [_NUM_OF_CMD + 1];

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
