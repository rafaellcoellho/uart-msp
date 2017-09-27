# Basic UART routines in MSP430 microcontroller (MSP430x5xx)

Example of configuration of UART in C. Simple routines for better understanding of the peripheral. There are several similar ones on the internet, but the more the better :D.

## Getting Started

Clone or download this repository on your computer. In the docs folder there is the x5xx manual (that this code will probably work). In the src folder is the header and the implementation of the functions.

### Prerequisites

When writing this code I was using MSP-EXP430F5529LP, so I guarantee that this code works perfectly well with it. Other variations of the x5xx series may not work the same, but I believe they are similar and this code is a good start.

I used the 32768 Hz XT2 oscillator, if there is not one with the same value attached to your MSP, you will need to adjust some things. Any questions just look in the manual.

When include in your code, to send bytes, no restriction. But byte reception only happens if interrupts are enabled. At CCS this is how we enable peripheral interruptions:

```
_enable_interrupts();
```

If you can send but not receive anything, the interrupts are probably disabled for some reason.

## Built With

* IDE: Code Composer Studio v6.1.1.00022
* Compiler: TI v4.4.5

I do not know if previous versions have problems. Just in case you make sure nothing has changed much in your compiler.

## How to use

```c
void uca0Init(void);
void uca0WriteByte(const char byte);
void uca0WriteString(char *str);
char uca0ReadByte(void);
char *uca0ReadString(void);
```
The name of the functions are very self explanatory, so there is not much to explain. Include this library in the code you want to use, and call the above functions just like any library.

```c
#include "uca0.h"
```

### How change baudrate

Read the registers settings of the following code, and replace according to the table on page 953 of the microcontroller manual.

```c
UCA0CTL1 |= UCSSEL0;
UCA0BR0 = 3;
UCA0BR1 = 0;
UCA0MCTL =  UCBRS_3;
```

### Input buffer size

Change this line.

```c
#define RX_BUFF 		128
```
### How does the code know that the input bytes sequence has ended?

I solved this problem in the following way: there is a timeout for the arrival of a new byte, when this limit is exceeded the function of reading the stream of bytes is warned by inserting a \0 at the end.

```c
unsigned int i = 50000;

do{
    if ( UCA0IFG & 0x01 ){
        return UCA0RXBUF;
    }
}while(--i);

return '\0';
```

Timeout is calculated with 50000 iterations of a loop. I empirically calculated that the data I was receiving with a baudrate of 9600, that value works. It's a somewhat unpredictable method in the matter of time, but I did not want to use a timer because I found it unnecessary complexity.

I do not check if the buffer has reached the end, this can be dangerous. If you use this code on something, think about this possibility and change the routines as needed.

## Authors

* **Rafael Coelho Silva** - [rafaellcoellho](https://github.com/rafaellcoellho)

## License

This project is licensed under the Beerware licence. For more information:

https://en.wikipedia.org/wiki/Beerware