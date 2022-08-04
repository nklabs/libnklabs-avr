// Copyright 2020 NK Labs, LLC

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <string.h>
#include "nksched.h"
#include "nkuart.h"

// Select UART if there are more than one
#ifndef RXC

// Probably ATmega328pb with 16 MHz clock
#define F_CPU 16000000UL
#define BAUD 9600

#define UCSRA UCSR0A
#define RXC RXC0

#define UCSRB UCSR0B
#define RXEN RXEN0
#define TXEN TXEN0
#define UDRE UDRE0
#define RXCIE RXCIE0

#define UCSRC UCSR0C
#define USBS USBS0
#define UCSZ0 UCSZ00

#define UDR UDR0

#define UBRRL UBRR0L
#define UBRRH UBRR0H

#define USART_RXC_vect USART_RX_vect

#else

// Probably ATmega32 on STK500 with 3.68 MHz clock
#define F_CPU 3686400UL
#define BAUD 115200


#endif

// Console UART Rx buffer

static unsigned char rx_buf[NK_UART_RXBUF_SIZE];
static uint32_t rx_buf_rd;
static volatile uint32_t rx_buf_wr;
static int tty_mode;

nk_spinlock_t console_lock = SPIN_LOCK_UNLOCKED;

// Task to submit when rx data available
int waiting_rx_tid;
void (*waiting_rx_task)(void *data);
void *waiting_rx_task_data;

void nk_set_uart_callback(int tid, void (*func)(void *data), void *data)
{
        nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
	if (rx_buf_rd != rx_buf_wr) {
		// Data is available now
		nk_sched(tid, func, data, 0, "UART ISR");
		waiting_rx_task = 0;
	} else {
		waiting_rx_tid = tid;
		waiting_rx_task = func;
		waiting_rx_task_data = data;
	}
	nk_irq_unlock(&console_lock, irq_flag);
}

int nk_get_uart_mode()
{
	return tty_mode;
}

int nk_set_uart_mode(int new_mode)
{
	int old_mode = tty_mode;
	tty_mode = new_mode;
	return old_mode;
}

static void wait_for_space()
{
    while (!(UCSRA & (1 << UDRE))) ;
}

void nk_putc(char ch)
{
        nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
	if (!tty_mode && ch == '\n') {
		wait_for_space();
		UDR = '\r';
	}
	wait_for_space();
        UDR = ch;
	nk_irq_unlock(&console_lock, irq_flag);
}

void nk_puts(const char *s)
{
        nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
	while (*s) {
		nk_putc(*s++);
	}
	nk_irq_unlock(&console_lock, irq_flag);
}

void nk_uart_write(const char *s, int len)
{
	nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
	while (len--) {
		nk_putc(*s++);
	}
	nk_irq_unlock(&console_lock, irq_flag);
}

// Transfer any available characters from UART FIFO to input buffer

static void rx_chars(void)
{
	while (UCSRA & (1 << RXC)) {
		uint8_t c = UDR;
		if (rx_buf_wr - rx_buf_rd != sizeof(rx_buf)) {
			rx_buf[rx_buf_wr++ & (sizeof(rx_buf) - 1)] = c;
		}
	}
}

// Interrupt if characters are available: but Apollo gives interrupt on 4 byte units, not single byte, so
// we still have to poll.

ISR(USART_RXC_vect)
{
	rx_chars();
	if (waiting_rx_task) {
		nk_sched(waiting_rx_tid, waiting_rx_task, waiting_rx_task_data, 0, "UART ISR");
		waiting_rx_task = 0;
	}
}

int nk_getc()
{
	int ch = -1;
        nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
        rx_chars();
	if (rx_buf_rd != rx_buf_wr) {
		ch = rx_buf[rx_buf_rd++ & (sizeof(rx_buf) - 1)];
	}
	nk_irq_unlock(&console_lock, irq_flag);
	return ch;
}

int nk_kbhit()
{
        nk_irq_flag_t irq_flag = nk_irq_lock(&console_lock);
	if (rx_buf_rd != rx_buf_wr) {
		nk_irq_unlock(&console_lock, irq_flag);
		return 1;
	} else {
		nk_irq_unlock(&console_lock, irq_flag);
		return 0;
	}
}

int nk_uart_read(char *s, int len, nk_time_t timeout)
{
	int l = 0;
	int need_time = 1;
	nk_time_t old_time;
	nk_time_t clocks = timeout * NK_TIME_COUNTS_PER_USECOND;
	while (l != len) {
		int c = nk_getc();
		if (c != -1) {
			s[l++] = (char)c;
			need_time = 1;
		} else {
			if (need_time) {
				old_time = nk_get_time();
				need_time = 0;
			}
			if ((nk_get_time() - old_time) >= clocks)
				break;
		}
	}
	return l;
}

// UART configuration settings.

#define UBRR_VALUE ((F_CPU / (BAUD * 16UL)) - 1)

void nk_init_uart()
{
   // Set up UART
   // Set baud
   UBRRH = (UBRR_VALUE >> 8);
   UBRRL = (UBRR_VALUE);
   // Enable Tx, Rx and Rx interrupts
   UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

   // Set format
   UCSRC =
     0
#ifdef URSEL
     | (1 << URSEL) // Select UCSRC regsiter on ATmega32
#endif
     | (1 << USBS) // Two stop bits
     | (3 << UCSZ0); // 8 data bits
     ;
   sei();
}
