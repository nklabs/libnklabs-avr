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
#include "nkprintf.h"
#include "nkmcuflash.h"
#include "nkarch_avr.h"

// Scheduler timer- using PICORV32 timer

static volatile nk_time_t wallclock;

// Convert delay in milliseconds to number of scheduler timer clock ticks
nk_time_t nk_convert_delay(uint32_t delay)
{
	return delay * (NK_TIME_COUNTS_PER_SECOND / 1000);
}

ISR(TIMER2_COMP_vect)
{
    ++wallclock;
}

void nk_init_sched_timer()
{
   TCCR2 = (1 << WGM21) | (5 << CS20);
   OCR2 = 28;
   TIMSK = (1 << OCIE2);

   // Enable interrupts
   sei();
}

// Generate timer interrupt to wake up system

void nk_sched_wakeup(nk_time_t when)
{
}

// Get current time

extern spinlock_t sched_lock;

nk_time_t nk_get_time()
{
	unsigned long irq_flag;
	nk_time_t wall;

	nk_irq_lock(&sched_lock, irq_flag);
	wall = wallclock;
	nk_irq_unlock(&sched_lock, irq_flag);

	return wall;
}

// Busy loop delay
// Also use sched timer...

void nk_udelay(unsigned long usec)
{
	// Generic implementation
	nk_time_t old = nk_get_time();
	nk_time_t clocks = usec * (NK_TIME_COUNTS_PER_SECOND / 1000000);
	while ((nk_get_time() - old) < clocks);
}

int nk_init_mcuflash()
{
	return 0;
}

int nk_mcuflash_erase(const void *info, uint32_t address, uint32_t byte_count)
{
	return -1;
}

int nk_mcuflash_write(const void *info, uint32_t address, const uint8_t *data, uint32_t byte_count)
{
	return -1;
}

int nk_mcuflash_read(const void *info, uint32_t address, uint8_t *data, uint32_t byte_count)
{
	return -1;
}

void reboot(void)
{
};

#if 0
size_t strlen(const char *s)
{
	const char *t = s;
	while (*t)
		++t;
	return (size_t)(t - s);
}

int strcmp(const char *a, const char * b)
{
	while (*a && *b && *a == *b) {
		++a;
		++b;
	}
	if (*a > *b)
		return 1;
	else if (*a < *b)
		return -1;
	else
		return 0;
}

char *strcpy(char *d, const char *s)
{
	char *t = d;
	while ((*t++ = *s++));
	return d;
}

char *stpcpy(char *d, const char *s)
{
	while ((*d = *s)) {
		++d;
		++s;
	}
	return d;
}

void *memset(void *d, int c, size_t n)
{
	char *t = d;
	while (n--) {
		*t++ = (char)c;
	}
	return d;
}

void *memmove(void *d, const void *s, size_t n)
{
	char *t = d;
	const char *u = s;

	if (d < s) {
		while (n--) {
			*t++ = *u++;
		}
	} else if (d > s) {
		t += n;
		u += n;
		while (n--) {
			*--t = *--u;
		}
	}

	return d;
}

void *memcpy(void *d, const void *s, size_t n)
{
	char *t = d;
	const char *u = s;

	while (n--) {
		*t++ = *u++;
	}

	return d;
}

int memcmp(const void *d, const void *s, size_t n)
{
	const unsigned char *t = d;
	const unsigned char *u = s;
	while (n--) {
		if (*t > *u) return 1;
		if (*t < *u) return -1;
		++t; ++u;
	}
	return 0;
}

int atoi(const char *s)
{
	int val = 0;
	while (*s >= '0' && *s <= '9')
	{
		val = val * 10 + *s++ - '0';
	}
	return val;
}
#endif
