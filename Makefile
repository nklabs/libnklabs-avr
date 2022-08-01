# Example Makefile for demo project

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS= \
   -g2 \
   -mmcu=atmega32 \
   -Og \
   -funsigned-char \
   -funsigned-bitfields \
   -ffunction-sections \
   -fpack-struct \
   -fshort-enums \
   -Wall \
   -std=gnu99 \

rom.hex : demo.out
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures demo.out rom.hex

demo.out : demo.o avr.ld
	$(CC) $(CFLAGS) --verbose -o demo.out -Wl,-Map,demo.map \
	-Wl,--start-group \
	-Wl,-lm \
	-Wl,--end-group \
	-Wl,--gc-sections \
	-T avr.ld \
	-Wl,--verbose \
	demo.o

demo.o : demo.c
	$(CC) $(CFLAGS) -Os -c demo.c

clean:
	rm -f *.o *.out *.map *.hex

flash:
	avrdude -p m32 -c stk500 -P /dev/ttyUSB0 -U flash:w:rom.hex

# Change lower fuse byte from 0xE1 to 0xE0, so that external clock is used

fuse:
	avrdude -p m32 -c stk500 -P /dev/ttyUSB0 -U lfuse:w:0xe0:m

# Use avrdued -p \? to get a list of known parts
