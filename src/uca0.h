/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, by Joerg Wunsch):
 * <rafaellcoellho@gmail.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

/*
	@file					uca0.h
	@autor					Rafael Coelho <rafaellcoellho@gmail.com>
	@brief					Biblioteca da uca0 voltada para interfacear com m�dulo bluetooth
	@details

*/

#ifndef UCA0_H_
	#define UCA0_H_

	#include <stdbool.h>
	#include <stdint.h>

	#define RX_BUFF 							128

	extern char *uca0Receiver;
	extern bool uca0DataReady;

	void uca0Init(void);
	void uca0WriteByte(const char byte);
	void uca0WriteString(char *str);
	char uca0ReadByte(void);
	char *uca0ReadString(void);


#endif /* UCA0_H_ */
