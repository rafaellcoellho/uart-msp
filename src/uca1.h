/*
	@file					uca1.h
	@autor					Rafael Coelho <rafaellcoellho@gmail.com>
	@data de criação		08/02/2017 15:04:00
	@data de atualização	06/04/2017 13:20:00
	@brief					Biblioteca da uca1
	@details

*/

#ifndef UCA1_H_
	#define UCA1_H_

	#include "standart_functions.h"

	#define DEBUG_SERIAL_LINE
	#define STUFF_BYTES

	#define RX_BUFF 							2053

	extern unsigned char *uca1_receiver;
	extern bool uca1_data_ready;

	void uca1_init(void);
	void uca1_change_baudrate(void);
	void uca1_write_byte(const unsigned char byte);
	void uca1_write_bytes(const unsigned char *array, const unsigned int size);
	bool uca1_read_byte(unsigned char *byte);
	unsigned char *uca1_read_bytes(void);
	void uca1_break(void);

#endif /* UCA1_H_ */
