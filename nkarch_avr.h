// MCU abstraction layer

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

#ifndef _Inkarch_avr
#define _Inkarch_avr

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

// AVR: define to put help text and format strings in flash memory
#define NK_PSTR

#ifdef NK_PSTR
#define NK_FLASH __flash
#else
#define NK_FLASH
#endif

typedef uint8_t nk_irq_flag_t;
typedef uint8_t nk_spinlock_t;
#define SPIN_LOCK_UNLOCKED 0

// Restore interrupt enable flag
// Release spinlock on multi-core systems

inline __attribute__((always_inline)) void nk_irq_unlock(nk_spinlock_t *lock, nk_irq_flag_t flags)
{
    (void)lock;
    SREG = flags;
}

// Restore interrupt enable flag
// Release spinlock on multi-core systems
// Sleep until an interrupt occurs

inline __attribute__((always_inline)) void nk_irq_unlock_and_wait(nk_spinlock_t *lock, nk_irq_flag_t flags, int deepness)
{
    (void)lock;
    (void)deepness;
    SREG = flags;
}

// Save interrupt enable flag and disable all interrupts
// Acquire spinlock on multi-core systems

inline __attribute__((always_inline)) nk_irq_flag_t nk_irq_lock(nk_spinlock_t *lock)
{
    (void)lock;
    nk_irq_flag_t flags = SREG;
    cli();
    return flags;
}

// Scheduler timer

typedef uint32_t nk_time_t;

// Units for scheduler time
#define NK_TIME_COUNTS_PER_SECOND 1000
#define NK_TIME_COUNTS_PER_USECOND 0

// Get current time
nk_time_t nk_get_time();

// Convert milliseconds into scheduler time
nk_time_t nk_convert_delay(uint32_t delay);

// Initialize scheduler timer
void nk_init_sched_timer();

// Set an alarm for 'when'.  An interrupt should fire at this time.
// Interrupts will be masked when this is called.

// If when is now or in the past, the interrupt should also fire.

// The system must wake up immediately on the next call to nk_irq_unlock_and_wait.

void nk_sched_wakeup(nk_time_t when);

// Microsecond delay

void nk_udelay(unsigned long usec);

// Erase size for MCU flash- in this case it's an external SPI flash

#define NK_MCUFLASH_ERASE_SIZE 4096

// Reboot

void nk_reboot(void);

#endif
