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
TARGET := target/$(PROGNAME)

# AVR Compiler and Programmer setup
CC := gcc
AS := gcc
CFLAGS := -Wall --std=gnu99 -O2 -funsigned-char -funsigned-bitfields\
 -fshort-enums -Wno-nonnull -I$(INCDIR)


$(OBJDIR)/%.o:	%.c 
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	%.s 
	$(AS) $(ASFLAGS) -c -o $@ $<

$(TARGET):	$(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target: $(TARGET) ;

install:
	install -m 0755 target/$(PROGNAME) /usr/bin/$(PROGNAME)
	strip /usr/bin/$(PROGNAME)

debug: CFLAGS += -DDEBUG -ggdb
debug: target ;

clean:
	rm -f $(OBJDIR)/*.o $(BINS) target/*


.PHONY:	clean all target install
