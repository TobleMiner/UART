#ifndef __UART_LIB_H__
#define __UART_LIB_H__

	#ifndef TRUE
		#define TRUE 1
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif

	//Config
	#include "config/config.h"
	
	//Libs
	#include <stdint.h>
	
	//Functions
	extern void		uart_init(void);
	extern void		uart_set_baudrate(uint32_t baudrate);
	#if UART_ENABLE_TX == TRUE
	extern void		uart_init_tx(void);
	extern void		uart_send_byte(uint8_t byte);
	extern void		uart_send_async(uint8_t* data, uint16_t offset, uint16_t len);
	extern void		uart_send_sync(uint8_t* data, uint16_t offset, uint16_t len);
	extern void		uart_write_sync(char* str);
	extern void		uart_write_async(char* str);
	#endif
	#if UART_ENABLE_RX == TRUE
	extern void		uart_init_rx(void);
	extern uint8_t	uart_receive_byte();
	extern void		uart_receive_sync(uint8_t* data, uint16_t len);
	#endif
	
	#include "uart.c"
#endif
