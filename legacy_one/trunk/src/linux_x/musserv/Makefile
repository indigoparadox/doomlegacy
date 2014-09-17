# GNU Make makefile for Doom Legacy
# Makefile for musserver 1.4
#
# Copyright (C) 1998-2013 by Doom Legacy Team.
#
#openserver5
# MUS_OS=SCOOS5
#unixware2
# MUS_OS=SCOUW2
#unixware7
# MUS_OS=SCOUW7


#CC	= gcc
CFLAGS	= -I. -Wall -O2 -march=pentium
LDFLAGS	=
#LDFLAGS	= -static
OPTOBJ=

# Differences from Linux

ifeq ($(SMIF), FREEBSD_X11)
 CFLAGS	= -I. -Wall -O2 -m486 -I/usr/src/sys/i386/isa/sound -DFREEBSD
 SUBDIR=freebsd
else
 SUBDIR=linux
endif

ifeq ($(MUS_OS), SCOOS5)
# The differences from Linux
# use this for an older GNU C 2.7.2
#CFLAGS	= -I. -Wall -O2 -m486 -b elf -DSCOOS5
# use this for EGCS 1.0.1
CFLAGS	= -I. -Wall -O2 -mpentium -DSCOOS5
LDFLAGS	=
OPTOBJ=usleep.o
SUBDIR=openserver5
endif

ifeq ($(MUS_OS), SCOUW2)
CFLAGS	= -I. -Wall -O2 -m486 -DSCOUW2
LDFLAGS	=
OPTOBJ=usleep.o
SUBDIR=unixware2
endif

ifeq ($(MUS_OS), SCOUW7)
CC=cc
CFLAGS	= -I. -O2 -K pentium -DSCOUW7
LDFLAGS	=
OPTOBJ=usleep.o
SUBDIR=unixware7
endif


#############################################
# Nothing below this line should be changed #
#############################################

# Export BASEOBJ, BASEBIN from parent Make.
O:=$(BASEOBJ)/$(SUBDIR)
BIN:=$(BASEBIN)

MUSOBJLIST:=musserver.o readwad.o playmus.o sequencer.o $(OPTOBJ)
MUSOBJ:=$(addprefix $(O)/, $(MUSOBJLIST))

all: $(O)/musserver

$(O)/musserver: $(MUSOBJ)
	${CC} ${CFLAGS} ${LDFLAGS} $(MUSOBJ) -o $(BIN)/musserver

clean:
	rm -f $(O)/*

dirs:
	mkdir -p $(O)

$(O)/%.o: %.c
	@echo $(O),  $(O)
	${CC} ${CFLAGS} -c $< -o $@
