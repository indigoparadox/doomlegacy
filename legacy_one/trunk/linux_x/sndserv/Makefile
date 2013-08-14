##########################################################
## GNU Make makefile for Doom Legacy
# Makefile for sndserver
#
# Copyright (C) 1998-2013 by Doom Legacy Team.
#
# $Id: Makefile,v 1.7 2004/04/20 00:36:48 andyp Exp $
#
# $Log: Makefile,v $
# Revision 1.7  2004/04/20 00:36:48  andyp
# Linux compilation fixes and string cleanups
#
# Revision 1.6  2002/12/17 19:56:51  bock
# Add esound support without SDL.
# Sorry about this, this code is disabled by default.
# You should build with HAVE_ESD=1 flag.
#
# Revision 1.5  2001/05/29 22:18:41  bock
# Small BSD commit: build r_opengl.so, sndserver
#
# Revision 1.4  2000/04/30 19:50:37  metzgermeister
#
# Revision 1.3  2000/03/12 22:34:07  linuxcub
# Fixed makefile (leading spaces converted to tabs)
#
# Revision 1.2  2000/02/27 00:42:12  hurdler
# Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
# Initial import into CVS (v1.29 pr3)
#
#openserver5
# MUS_OS=SCOOS5
#unixware2
# MUS_OS=SCOUW2
#unixware7
# MUS_OS=SCOUW7
#ESD demon
# HAVE_ESD=1

CC=gcc
CFLAGS=-O2 -march=pentium -Wall -DNORMALUNIX -DLINUX
LDFLAGS=
LIBS=-lm
OPTOBJ=
SNDEXE=llsndserv

# Differences from Linux

ifeq ($(SMIF), FREEBSD_X11)
 SUBDIR=freebsd
 CFLAGS:=${CFLAGS} -DFREEBSD
else
 SUBDIR=linux
endif

ifeq ($(MUS_OS), SCOOS5)
CC=gcc
# use this for an older GNU C 2.7.2
#CFLAGS=-O2 -m486 -b elf -Wall -DNORMALUNIX -DSCOOS5
# use this for EGCS 1.0.1
CFLAGS=-O2 -mpentium -Wall -DNORMALUNIX -DSCOOS5
LDFLAGS=
LIBS=-lm
OPTOBJ=wadread.o
SUBDIR=openserver5
SNDEXE=sndserver
endif

ifeq ($(MUS_OS), SCOUW2)
CC=gcc
CFLAGS=-O2 -m486 -Wall -DNORMALUNIX -DSCOUW2
LDFLAGS=
OPTOBJ=wadread.o strcmp.o
SUBDIR=unixware2
SNDEXE=sndserver
endif

ifeq ($(MUS_OS), SCOUW7)
CC=cc
CFLAGS=-O2 -K pentium -DNORMALUNIX -DSCOUW2
LDFLAGS=
OPTOBJ=wadread.o strcmp.o
SUBDIR=unixware7
SNDEXE=sndserver
endif


#############################################

O:=../../$(BASEOBJ)/$(SUBDIR)
BIN:=../../$(BASEBIN)


HWOUTOBJ	:=	linux.o
ifdef HAVE_ESD
ESDCFLAGS	:=	$(shell esd-config --cflags)
ESDLDFLAGS	:=	$(shell esd-config --libs)
LIBS		+=	$(ESDLDFLAGS)
HWOUTOBJ	:=	esd.o
CFLAGS		+=	$(ESDCFLAGS)
endif


SNDOBJLIST:=soundsrv.o sounds.o $(HWOUTOBJ) $(OPTOBJ)
SNDOBJ:=$(addprefix $(O)/, $(SNDOBJLIST))


all:     $(O)/llsndserv

clean:
	rm -f $(O)/*

dirs:
	mkdir -p $(O)

# Target
$(O)/llsndserv: $(SNDOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SNDOBJ) -o $(BIN)/$(SNDEXE) $(LIBS)
	@echo make complete.

# Rule
$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

