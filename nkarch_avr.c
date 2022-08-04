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

#ifdef TCCR2
ISR(TIMER2_COMP_vect)
#else
ISR(TIMER2_COMPA_vect)
#endif
{
    ++wallclock;
}


void nk_init_sched_timer()
{
#ifdef TCCR2
    // Set for ~1000Hz from 3.68 MHz
    TCCR2 = (1 << WGM21) | (5 << CS20);
    OCR2 = 28;
    TIMSK = (1 << OCIE2);
#else
    // Set for 1000 Hz from 16 MHz
    TCCR2B = (5 << CS20); // divide by 128
    TCCR2A = (1 << WGM21); // Clear timer on match mode
    OCR2A = 124; // divide by 125
    TIMSK2 = (1 << OCIE2A);
#endif

    // Enable interrupts
    sei();
}

// Generate timer interrupt to wake up system

void nk_sched_wakeup(nk_time_t when)
{
    (void)when;
}

// Get current time

extern nk_spinlock_t sched_lock;

nk_time_t nk_get_time()
{
    nk_irq_flag_t irq_flag;
    nk_time_t wall;

    irq_flag = nk_irq_lock(&sched_lock);
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
    (void)info;
    (void)address;
    (void)byte_count;
    return -1;
}

int nk_mcuflash_write(const void *info, uint32_t address, const uint8_t *data, size_t byte_count)
{
    (void)info;
    (void)address;
    (void)data;
    (void)byte_count;
    return -1;
}

int nk_mcuflash_read(const void *info, uint32_t address, uint8_t *data, size_t byte_count)
{
    (void)info;
    (void)address;
    (void)data;
    (void)byte_count;
    return -1;
}

void nk_reboot(void)
{
    // To reset: enable watchdog timer, but don't answer it
    cli();
    wdt_enable(WDTO_15MS);
    for (;;) { }
};

// Disable WDT at reset
// We have to do this because rebooting the above way leaves WDT enabled after reset

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
}
