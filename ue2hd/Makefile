
PROJ	= ue2hd

CC		= msp430-gcc
OBJCOPY	= msp430-objcopy

PWD		= $(shell pwd)
BUILDDIR = $(PWD)/build
OBJDIR	= $(BUILDDIR)/obj

MCU		= msp430g2131
C_SRCS	= ue2hd.c
INCLUDES = -IInclude
CFLAGS	= -mmcu=$(MCU) -g -Os -Wall -Wunused $(INCLUDES)
LDFLAGS	= -mmcu=$(MCU) -Wl,-Map=$(BUILDDIR)/$(PROJ).map

OUTFILES = $(BUILDDIR)/$(PROJ).elf $(BUILDDIR)/$(PROJ).hex

C_OBJS  = $(addprefix $(OBJDIR)/, $(notdir $(C_SRCS:.c=.o)))
OBJS	= $(C_OBJS)

.PHONY: dflt

dflt: $(OUTFILES)

$(OBJS): | $(OBJDIR)

$(BUILDDIR) $(OBJDIR):
	@mkdir -p $(OBJDIR)

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

$(C_OBJS) : $(OBJDIR)/%.o : %.c Makefile *.h
	@echo $<
	@$(CC) $< ${CFLAGS} -c -o $@

clean:
	@rm -rf $(BUILDDIR)
