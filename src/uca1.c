#include <msp430f5529.h>

#include "uca1.h"

#ifdef DEBUG_SERIAL_LINE
	#include <stdio.h>
#endif

unsigned char *uca1_receiver;
bool uca1_data_ready = false;

void uca1_init(void){

	#ifdef DEBUG_SERIAL_LINE
		printf("-->Attencion, DEBUG_SERIAL_LINE is active!\n");
		#ifdef STUFF_BYTES
			printf("-->STUFF_BYTE is active!\n\n");
		#else
			printf("-->STUFF_BSPI YTE not active!\n\n");
		#endif
	#endif

	//Dá um reset no periférico serial do MSP e deixa em modo de configuração
	UCA1CTL1 = UCSWRST;

	/*
	UCSSEL0 -> Seleciona a fonte de clock para ACLK = 32768Hz
	UCA1BR0 -> Valor tabelado no datasheet para a baudrate de 9600, ver página 953
	UCA1BR1 -> Valor tabelado no datasheet para a baudrate de 9600, ver página 953
	UCBRS_3 -> Tabelado no datasheet, ver página 953
	 */
	UCA1CTL1 |= UCSSEL__ACLK;
	UCA1BR0 = 3;
	UCA1BR1 = 0;
	UCA1MCTL =  UCBRS_3;

	//Seleciona a função dos i/o para serial, sendo P4.4 -> TX e P4.5 -> RX
	P4DIR |= BIT4;
	P4SEL |= BIT4 + BIT5;

	//Termina modo de configuração do periférico
	UCA1CTL1 = UCA1CTL1 & ~UCSWRST;

	//Habilita a interrupção do RX
	UCA1IE |= UCRXIE;
}

void uca1_change_baudrate(void){
	//Dá um reset no periférico serial do MSP e deixa em modo de configuração
	UCA1CTL1 = UCSWRST;

	/*
	UCSSEL_SMCLK -> Seleciona a fonte de clock para SMCLK = 4 MHz
	UCA1BR0 -> Valor tabelado no datasheet para a baudrate de 115200, ver página 953
	UCA1BR1 -> Valor tabelado no datasheet para a baudrate de 115200, ver página 953
	UCBRS_3 -> Tabelado no datasheet, ver página 953
	 */
	UCA1CTL1 |= UCSSEL__SMCLK;
	UCA1BR0 = 34;
	UCA1BR1 = 0;
	UCA1MCTL =  UCBRS_6;

	//Termina modo de configuração do periférico
	UCA1CTL1 = UCA1CTL1 & ~UCSWRST;

	//Habilita a interrupção do RX
	UCA1IE |= UCRXIE;
}

void uca1_write_byte(const unsigned char byte){

	//Enquanto o periférico estiver ocupado, espera
	while (UCA1STAT & UCBUSY != 0);
	UCA1TXBUF = byte;
}

void uca1_write_bytes(const unsigned char *array, const unsigned int size){

	unsigned int i,j;

	#ifdef DEBUG_SERIAL_LINE
		printf("Transmitting: ");
		for(i=0, j=size ; j > 0; i++, j--){
			printf("0x%x ", array[i] );
		}
		printf("\n");
	#endif

	#ifdef STUFF_BYTES
		// Envia o start byte
		uca1_write_byte( array[0] );

		// Faz uma iteração para enviar todos os bytes, enquanto faz um stuffing dos bytes
		for(i=1, j=(size-1) ; j > 0 ; i++, j--){

			// Se for <STX> ou <XON> ou <XOFF>, envia por exemplo <DLE><STX+1>
			if( (array[i] == 0x02) | (array[i] == 0x11) | (array[i] == 0x13) ){
				uca1_write_byte(0x10);
				uca1_write_byte( array[i] + 1 );
			}else if(array[i] == 0x10){
				// Se for <DLE>, envia <DLE><DLE>
				uca1_write_byte(0x10);
				uca1_write_byte(0x10);
			}else{
				//Se não for nenhum desses, simplesmente envia
				uca1_write_byte( array[i] );
			}
		}

		// Envia CRC
		uca1_write_byte( array[i++] );
		uca1_write_byte( array[i] );
	#else
		//Simplesmente faz uma iteração
		for(i=0, j=size; j > 0; i++, j--){
			uca1_write_byte( array[i] );
		}
	#endif
}

bool uca1_read_byte(unsigned char *byte){

	//Tempo empírico de timeout, cerca de 50000 ciclos
	unsigned int i = 50000;

	do{
		//Se a interrupção de recepção estiver ligada, passa o byte recebido por referência e retorna que ainda está havendo transmissão
		if ( UCA1IFG & 0x01 ){
			*byte = UCA1RXBUF;
			return true;
		}
	}while(--i);

	//Caso i tenha chegado a 0 antes de algum byte chegar e ligar a interrupção da serial, retorna falso encerrar o stream de bytes
	return false;
}

unsigned char *uca1_read_bytes(void){

	static unsigned char buffer[RX_BUFF];
	unsigned char aux;
	unsigned char *temp;
	temp = buffer;

	//Ao notar que a rotina sempre acabava perdendo o primeiro byte, a primeira captura é feita
	*temp = UCA1RXBUF;
	temp++;

	#ifdef STUFF_BYTES
		bool stuff_next_byte = false;
		//Enquanto não houver timeout, lê o byte e coloca no buffer
		while( uca1_read_byte(&aux) ){

			//Se o byte anterior foi 0x10
			if(stuff_next_byte == true){
				//Se o byte é <DLE>
				if(aux == 0x10){
					//Simplesmente coloca no vetor
					*temp = aux;
				}else{
					//Subtrai pra encontrar o byte correto
					*temp = aux-1;
				}
				temp++;
				stuff_next_byte = false;
			}else if(aux != 0x10){
				//Se for um byte qualquer, simplemente armazena
				*temp = aux;
				temp++;
			}else{
				//Se for <DLE>, o proximo byte é pra ser fuito o stuffing
				stuff_next_byte = true;
			}
		}
	#else
		while( uca1_read_byte(&aux) ){
			*temp = aux;
			temp++;
		}
	#endif

	//Zona de DEBUG
	#ifdef DEBUG_SERIAL_LINE
		unsigned char *pt_aux = buffer;

		printf("Received: ");
		while(pt_aux != temp){
			printf("0x%x ", *pt_aux);
			pt_aux++;
		}
		printf("\n");
	#endif

	//Retorna um ponteiro para o array com a string
	return buffer;
}

void uca1_break(void){

	#ifdef DEBUG_SERIAL_LINE
		printf("Sending a BREAK for 100mS\n");
	#endif
	// Seleciona o P4.4 como io
	P4SEL &= ~BIT4;

	//BREAK é forçar o RX a estado lógico 0 por um determinado tempo
	P4OUT &= ~BIT4;

	//Envia o BREAK por 100mS
	__delay_cycles(2500000);

	// Seleciona P4.4 como RX novamente
	P4SEL |= BIT4;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void uartA1(void){

	switch (UCA1IV)
	{
		case 2:
			//Lê o que chegou no buffer de recepção
			uca1_receiver = uca1_read_bytes();

			//Indica que a recepção já foi lida
			uca1_data_ready = true;
			break;
		case 4:

			break;
	}
}
