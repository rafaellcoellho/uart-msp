#include <msp430f5529.h>
#include <stdio.h>

#include "uca0.h"

char *uca0Receiver;
bool uca0DataReady = false;

void uca0Init(void){

	//D� um reset no perif�rico serial do MSP e deixa em modo de configura��o
	UCA0CTL1 = UCSWRST;

	/*
	UCSSEL0 -> Seleciona a fonte de clock para ACLK = 32768Hz
	UCA0BR0 -> Valor tabelado no datasheet para a baudrate de 9600, ver p�gina 953
	UCA0BR1 -> Valor tabelado no datasheet para a baudrate de 9600, ver p�gina 953
	UCBRS_3 -> Tabelado no datasheet, ver p�gina 953
	 */
	UCA0CTL1 |= UCSSEL0;
	UCA0BR0 = 3;
	UCA0BR1 = 0;
	UCA0MCTL =  UCBRS_3;

	//Seleciona a fun��o dos i/o para serial, sendo P3.3 -> TX e P3.4 -> RX
	P3DIR |= BIT3;
	P3SEL |= BIT3 + BIT4;

	//Termina modo de configura��o do perif�rico
	UCA0CTL1 = UCA0CTL1 & ~UCSWRST;

	//Habilita a interrup��o do RX
	UCA0IE |= UCRXIE;
}

void uca0WriteByte(const char byte){

	//Enquanto o perif�rico estiver ocupado, espera
	while (UCA0STAT & UCBUSY != 0);
	UCA0TXBUF = byte;
}

void uca0WriteString(char *str){

	do{
		uca0WriteByte(*str);
	} while (*++str);
}

char uca0ReadByte(void){

	//Tempo emp�rico de timeout, cerca de 50000 ciclos
	unsigned int i = 50000;

	do{
		//Se a interrup��o de recep��o estiver ligada, retorna o byte recebido
		if ( UCA0IFG & 0x01 ){
			return UCA0RXBUF;
		}
	}while(--i);

	//Caso i tenha chegado a 0 antes de algum byte chegar e ligar a interrup��o da serial, retorna \0 como terminador da string recebida
	return '\0';
}

char *uca0ReadString(void){

	static char buffer[RX_BUFF];
	char *temp;
	temp = buffer;

	//Ao notar que a rotina sempre acabava perdendo o primeiro byte, a primeira captura � feita
	*temp = UCA0RXBUF;
	temp++;

	//Enquanto receber bytes != do terminador '\0', armazena no buffer e passa pra pr�xima posi��o
	while( (*temp = uca0ReadByte() ) != '\0'){
		++temp;
	}

	//Retorna um ponteiro para o array com a string
	return buffer;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void uartA0(void){

	switch (UCA0IV)
	{
		case 2:
			//L� o que chegou no buffer de recep��o
			uca0Receiver = uca0ReadString();

			//Indica que a recep��o j� foi lida
			uca0DataReady = true;
			break;
		case 4:

			break;
	}
}
