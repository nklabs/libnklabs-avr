# libnklabs example for 8-bit AVR microcontrollers

## ATmega32

The target is an ATmega32 located in an STK500 development board.  We have
been using the Atmel ICE instead of the STK500 to program the chip.  Atmel
ICE seems to work more reliably with "avrdude" (the open source programming
tool).

![STK500 with ATmega32](doc/stk500.png)

ATmega32 has 32K of flash and 2K of RAM.  This is enough to run the
libnklabs CLI.

The main challenge with AVR is the limited amount of RAM coupled with the
fact that string constants are by default stored in RAM.  They are stored
there by default because AVR provides a separate address space for flash
memory.  Flash memory numeric addresses overlap with RAM addresses (so the
number alone can not be used to distinguish between the spaces), and in any
case special methods must be used to access data from flash.

String constants can be forced into flash by marking them with the
__\_\_flash__ storage class in avr-gcc.  All pointers to such string
constants must also be adorned with this storage class.  A special macro,
__PSTR()__, must be used to surround string literals if they are to be
placed in flash.

libnklabs automatically puts printf format strings and help text into flash. 
This is mostly hidden to the user, since nk_printf is implemented as a macro
which invokes PSTR() on the format string.

Unfortunately, this breaks gcc's format string type checking (so directives
in format strings are not type checked).  Also, there are now two directives
for strings: use %s if the string is located in RAM, or %S if the string is
located in flash.

## AVR Oscillator

New, unprogrammed ATmega32s use a built-in 1 MHz oscillator by default.

For the serial port to work, the lower fuse byte should be changed from 0xE1
to 0xE0 so that it will instead use the STK500 supplied clock.  This clock
is 3686400 Hz ("3.68 MHz" from the documentation), which is an integer
multiple of 115200 baud.

This fuse byte can be programmed with

	make fuse

## AVR UART

PD0 and PD1 are the UART Tx and Rx lines.  These should be connected to the
RXD and TXD pins so that the "RS232 SPARE" DB9 connector of the STK500 can
be used to access the libnklabs CLI.  See the photo above.

## Build instructions

Prerequisites:

	sudo apt-get install avrdude
	sudo apt-get install avr-gcc

Build the software with:

	make

Program the chip using Atmel ICE:

	make flash
