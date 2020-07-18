
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>

#include "my_uart.h"


#define MAX_BUF 256


int main(void){
UART_init();
UART_putString((uint8_t*)
"write something, i'll repeat it\n");
uint8_t buf[MAX_BUF];

while(1) {

UART_getString(buf);
UART_putString((uint8_t*)"received\n");
UART_putString(buf);
}
}