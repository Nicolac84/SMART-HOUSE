# SmartHouse Project Makefile - AVR
.POSIX:
.SUFFIXES: # Reset all implicit rules
.PRECIOUS: $(OBJDIR)/%.o # Keep objects when generated as intermediate targets

# Project parameters
PROGNAME := smarthouse

# Repository structure
SRCDIR := sources
INCDIR := include
OBJDIR := objects
RESDIR := resources

# Search paths
vpath %.c $(SRCDIR)
vpath %.s $(SRCDIR)
vpath %.o $(OBJDIR)

OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
TARGET := target/$(PROGNAME).elf

# AVR Compiler and Programmer setup
CC := avr-gcc
AS := avr-gcc
CFLAGS := -Wall --std=gnu99 -DF_CPU=16000000UL -O3 -funsigned-char\
 -funsigned-bitfields  -fshort-enums -Wstrict-prototypes\
 -mmcu=atmega2560 -I$(INCDIR) -DAVR -D__AVR_3_BYTE_PC__
ASFLAGS := -x assembler-with-cpp $(CFLAGS)

AVRDUDE := avrdude
AVRDUDE_PROGRAMMER := wiring
AVRDUDE_PORT := /dev/ttyACM0
AVRDUDE_CONFIG != find /usr/share/arduino/hardware -name avrdude.conf
AVRDUDE_NO_VERIFY := -V

AVRDUDE_FLAGS := -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b\
 115200 $(AVRDUDE_NO_VERIFY) $(AVRDUDE_VERBOSE) $(AVRDUDE_ERASE_COUNTER)\
 -D -q -C $(AVRDUDE_CONFIG)

# Use these as functions
avrdude_write_flash = -U flash:w:$(strip $(1)):i
avrdude_write_eeprom = -U eeprom:w:$(strip $(1)):i


$(OBJDIR)/%.o:	%.c 
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	%.s 
	$(AS) $(ASFLAGS) -c -o $@ $<

%.eep:	%.elf
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load"\
	  --change-section-lma .eeprom=0 --no-change-warnings\
	  -O ihex $< $@

%.hex:	%.eep %.elf
	avr-objcopy -O ihex -R .eeprom $(patsubst %.hex, %.elf, $@) $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(call avrdude_write_flash, $@)\
	  $(call avrdude_write_eeprom, $(patsubst %.hex, %.eep, $@))


$(TARGET):	$(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target: $(TARGET) ;

flash: target/$(PROGNAME).hex ;

client:
	@cd client/ && make

client-debug:
	@cd client/ && make clean debug

install:
	@cd client/ && make install

clean:	
	rm -f $(OBJDIR)/*.o $(BINS) target/* client/objects/* client/target/*


.PHONY:	clean all target flash client
