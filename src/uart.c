#include <avr/interrupt.h>

#if UART_ENABLE_RX == TRUE
volatile uint8_t uart_rx_ring[UART_BUFF_LEN_RX]; //Init buffer for uart-rx
volatile uint8_t *uart_rx_curpos_ring = uart_rx_ring;
volatile uint8_t *uart_rx_targpos_ring = uart_rx_ring;
#endif

#if UART_ENABLE_TX == TRUE
volatile uint8_t uart_tx_ring[UART_BUFF_LEN_TX]; //Init buffer for uart-tx
volatile uint8_t *uart_tx_curpos_ring = uart_tx_ring;
volatile uint8_t *uart_tx_targpos_ring = uart_tx_ring;
#endif

void uart_init(void)
{
	uart_set_baudrate(UART_BAUDRATE);
	UCSR0C = (1<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00); //8 data bits, 2 stop bits
}

void uart_set_baudrate(uint32_t baudrate)
{
	uint16_t ubrr = (((F_CPU / (baudrate * 16.0)) + .5) - 1);
	UBRR0H = (ubrr < 8) & 0xFF;
	UBRR0L = ubrr & 0xFF;
}

#if UART_ENABLE_RX == TRUE
void uart_init_rx(void)
{
	UCSR0B |= (1<<RXCIE0) | (1<<RXEN0);
}

ISR(USART_RX_vect)
{
	volatile uint8_t *tmpptr = uart_rx_targpos_ring;
	*tmpptr = UDR0; //Read received byte to ringbuffer
	tmpptr++;
	if(tmpptr >= UART_BUFF_LEN_RX + uart_rx_ring) //Wrap pointer if necessary
	{
		tmpptr = uart_rx_ring;
	}
	uart_rx_targpos_ring = tmpptr;
}
#endif

#if UART_ENABLE_TX == TRUE
void uart_init_tx(void)
{
	UCSR0B |= (1<<TXCIE0) | (1<<TXEN0);
}

void uart_send_byte(uint8_t byte)
{
	volatile uint8_t *tmpptr = uart_tx_targpos_ring;
	*tmpptr = byte; //Append byte to ringbuffer
	tmpptr++;
	if(tmpptr >= UART_BUFF_LEN_TX + uart_tx_ring) //Wrap pointer if necessary
	{
		tmpptr = uart_tx_ring;
	}
	if(uart_tx_targpos_ring == uart_tx_curpos_ring) //UART isn't busy
	{
		UCSR0B |= (1<<UDRIE0); //uart0: Enable uart data-register-empty interrupt
	}
	uart_tx_targpos_ring = tmpptr;
}

void uart_send_async(uint8_t* data, uint16_t offset, uint16_t len)
{
	UCSR0B &= ~(1<<TXCIE0); //uart0: Disable tx complete interrupt
	data += offset;
	for(; len > 0; len--)
	{
		uart_send_byte(*data);
		data++;
	}
	UCSR0B |= (1<<TXCIE0); //uart0: Enable tx complete interrupt
}

void uart_write_async(char* str)
{
	UCSR0B &= ~(1<<TXCIE0); //uart0: Disable tx complete interrupt
	while(*str)
	{
		uart_send_byte((unsigned char)*str);
		str++;
	}
	UCSR0B |= (1<<TXCIE0); //uart0: Enable tx complete interrupt
}

ISR(USART_TX_vect)
{
	if(uart_tx_curpos_ring != uart_tx_targpos_ring)
	{
		UDR0 = *uart_tx_curpos_ring; //Write byte from ringbuffer to UART
		uart_tx_curpos_ring++;
		if(uart_tx_curpos_ring >= UART_BUFF_LEN_TX + uart_tx_ring) //Wrap pointer if necessary
		{
			uart_tx_curpos_ring = uart_tx_ring;
		}
	}
}

ISR(USART_UDRE_vect)
{
	UDR0 = *uart_tx_curpos_ring; //Push first byte to UDR to start the transmission
	uart_tx_curpos_ring++;
	if(uart_tx_curpos_ring >= UART_BUFF_LEN_TX + uart_tx_ring) //Wrap pointer if necessary
	{
		uart_tx_curpos_ring = uart_tx_ring;
	}
	UCSR0B &= ~(1<<UDRIE0); //uart0: Disable uart data-register-empty interrupt
}
#endif