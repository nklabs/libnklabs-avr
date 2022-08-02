all: obj rom.hex

# Get application version number

NK_VERSION_MAJOR := $(shell cat VERSION_MAJOR)
NK_VERSION_MINOR := $(shell cat VERSION_MINOR)

# Get date/time

NK_DATE := $(shell date -u -Iminute)
NK_YEAR := $(shell expr $(shell echo $(NK_DATE) | cut -b 1-4) + 0)
NK_MONTH := $(shell expr $(shell echo $(NK_DATE) | cut -b 6-7) + 0)
NK_DAY := $(shell expr $(shell echo $(NK_DATE) | cut -b 9-10) + 0)
NK_HOUR := $(shell expr $(shell echo $(NK_DATE) | cut -b 12-13) + 0)
NK_MINUTE := $(shell expr $(shell echo $(NK_DATE) | cut -b 15-16) + 0)

# A define for the platform
NK_PLATFORM := NK_PLATFORM_AVR

# Get git hash as a string
# It is postfixed with -dirty if there are uncommitted changed; otherwise, it is postfixed with -clean.
NK_GIT_REV := \"$(shell git rev-parse HEAD)-$(shell if git diff-index --quiet HEAD --; then echo clean; else echo dirty; fi)\"


# Example Makefile for demo project

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS= \
   -I libnklabs/inc \
   -I config \
   -I . \
   -mmcu=atmega32 \
   -Os \
   -funsigned-char \
   -funsigned-bitfields \
   -ffunction-sections \
   -fpack-struct \
   -fshort-enums \
   -Wall \
   -Wconversion -Wunused -Wwrite-strings \
   -Wstrict-overflow=4 -Wmissing-include-dirs -Winit-self -Wcast-qual \
   -Wundef -Wlogical-op -Wmissing-declarations -Wformat \
   -Wmissing-format-attribute -Wformat-nonliteral \
   -Wswitch-enum -Wshadow -Wextra -Wformat-security \
   -std=gnu99 \
   -D$(NK_PLATFORM) -DNK_PLATFORM=\"$(NK_PLATFORM)\" -DNK_VERSION_MAJOR=$(NK_VERSION_MAJOR)  -DNK_VERSION_MINOR=$(NK_VERSION_MINOR) -DNK_YEAR=$(NK_YEAR) -DNK_MONTH=$(NK_MONTH) -DNK_DAY=$(NK_DAY) -DNK_HOUR=$(NK_HOUR) -DNK_MINUTE=$(NK_MINUTE) -DNK_GIT_REV=$(NK_GIT_REV)

OBJS = demo.o

OBJS = \
  main.o \
  nkuart_avr.o \
  nkarch_avr.o \
  libnklabs/src/nksched.o \
  libnklabs/src/nkprintf.o \
  libnklabs/src/nkoutfile.o \
  libnklabs/src/nkinfile.o \
  libnklabs/src/nkscan.o \
  libnklabs/src/nkcli.o \
  libnklabs/src/nkreadline.o \
  libnklabs/src/nkstring.o \
  info_cmd.o \
  basic_cmds.o \
  libnklabs/src/nkcrclib.o \
  libnklabs/src/nkmcuflash.o \
  libnklabs/src/nkdbase.o \
  libnklabs/src/nkchecked.o \
  libnklabs/src/nkdirect.o \
  libnklabs/src/nkserialize.o \
  libnklabs/src/nkymodem.o

# Keep them in a subdirectory
MOST_OBJS = $(addprefix obj/, $(OBJS))

SUBDIR_OBJS = $(MOST_OBJS) obj/version.o

# Rebuild version.o if any other file changed
obj/version.o: $(MOST_OBJS) VERSION_MAJOR VERSION_MINOR


rom.hex : demo.out
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures demo.out rom.hex

demo.out : $(SUBDIR_OBJS) avr.ld
	$(CC) $(CFLAGS) --verbose -o demo.out -Wl,-Map,demo.map \
	-Wl,--start-group \
	-Wl,-lm \
	-Wl,--end-group \
	-Wl,--gc-sections \
	-T avr.ld \
	$(SUBDIR_OBJS)



# Add to print verbose linker information
#	-Wl,--verbose \

#demo.o : demo.c
#	$(CC) $(CFLAGS) -Os -c demo.c

clean:
	rm -f *.o *.out *.map *.hex
	rm -rf obj

flash:
	avrdude -p m32 -c atmelice_isp -U flash:w:rom.hex
#	avrdude -p m32 -c stk500 -P /dev/ttyUSB0 -U flash:w:rom.hex

verify:
	avrdude -p m32 -c stk500 -P /dev/ttyUSB0 -U flash:v:rom.hex

# Change lower fuse byte from 0xE1 to 0xE0, so that external clock is used

fuse:
	avrdude -p m32 -c stk500 -P /dev/ttyUSB0 -U lfuse:w:0xe0:m

obj:
	mkdir obj
	mkdir obj/libnklabs
	mkdir obj/libnklabs/src


# Use avrdued -p \? to get a list of known parts

# include dependancy files if they exist
-include $(SUBDIR_OBJS:.o=.d)

# compile and generate dependency info
obj/%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o obj/$*.o
	@$(CC) -MM $(CFLAGS) $*.c > obj/$*.d
# Improve dependency file produced by gcc... allows files to be renamed and source files to exist
# in subdirectories.
	@mv -f obj/$*.d obj/$*.d.tmp
	@sed -e 's|.*:|obj/$*.o:|' < obj/$*.d.tmp > obj/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < obj/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> obj/$*.d
	@rm -f obj/$*.d.tmp

obj/%.o: %.S
	$(CC) -c $(CFLAGS) $*.S -o obj/$*.o
