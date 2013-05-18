#############################################################################
#
# $Id$
# GNU Make makefile for Doom Legacy (legacy@newdoom.com)
#
# Copyright (C) 1998-2011 by Doom Legacy Team.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
#
#  The preferred system media interface is SDL.
#     Compile the generic SDL version with 'make' (default)
#     Add 'DEBUG=1' for debugging information.
#
#  Other options (possibly outdated):
#     Compile the Linux X11 version with 'make SMIF=LINUX_X11'
#     Compile the FreeBSD X11 version with 'make SMIF=FREEBSD_X11'
#     Compile the OS/2 version with  'make SMIF=OS2_NATIVE'
#
#############################################################################

# User tunable settings.  Edit here or set by command line SMIF=LINUX_X11
# Most can just uncomment an appropriate selection for your system.
# WIN32=1
# CC_MINGW=1
# CC_WATCOM=1

# SMIF - System Media Interface (default=SDL)
#SMIF=SDL
#SMIF=LINUX_X11
#SMIF=FREEBSD_X11
#SMIF=OS2_NATIVE
#SMIF=DJGPPDOS_NATIVE

# SDL Mixer, to get music
HAVE_MIXER=1

# CD-Music enable/disable, if you don't have the special libraries, etc.
#ifndef NOCDMUS
CDMUS=1
#endif

# Developers with svn can enable this to have svn revision number in executable.
# Causes compile error message otherwise.
# Until can find test for presence of svn, this is best that can be done.
#SVN_ENABLE=1

# Subdirectories for objects and binaries
O=../objs
BIN=../bin

# Warning flags
WFLAGS = -Wall # -Wwrite-strings

# Optimization level, -O0 to -O5
OPTLEV = -O3
#OPTLEV = -O5

# Debug enables, save on reinventing this everytime.
#DEBUG=1
#PROFILEMODE=1
# Customize to your debugging environment.
ifdef DEBUG
# Normal debug (-g), MACRO info (-g3), stabs or gdb debug info
#  DEBUGFLAGS:= -g3 -DDEBUG_WINDOWED
  DEBUGFLAGS:= -ggdb -DDEBUG_WINDOWED
  # Debuggers do not cope with optimization well, so DEBUG ignores OPTLEV.
  # To debug with optimization, include it here, (-O0, -O1, or $(OPTLEV)).
#  DEBUGFLAGS+= $(OPTLEV)
endif

# standard
#STD= -std=c99
  # not ready yet
  # fails because of alloca, it is not std under c99
  # std=c89, does not support // commments, no inline, no asm

# gcc or g++
CC=gcc
ifdef CC_WATCOM
  CC=WATCOMC
endif

# End of User tunable settings

# Values changed by system detection
# file delete command, init value
RM=rm
# random stuff
NASMFORMAT=elf -DLINUX

ifdef WIN32
  RM=del
endif

# default system media interface
ifndef SMIF
  SMIF = SDL
endif

#=================================
ifeq ($(SMIF), SDL)
# SDL is the default system media interface. This is what all modern systems should use.
  # Requires SDL libs, and optionally SDL_mixer libs

# name of the executable
  EXENAME := doomlegacy

# interface directory (where you put all i_*.c)
  INTERFACE=sdl

  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS := $(O)/i_system.o $(O)/i_video.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o \
    $(O)/r_opengl.o $(O)/ogl_sdl.o $(O)/hwsym_sdl.o

# platform
  ifdef WIN32
    # Windows (MinGW)
    EXENAME := $(EXENAME).exe
    OPTS := -DWIN32
    # if have windows >= WinNT, and want large memory reports
#    OPTS += -DWIN_LARGE_MEM
    LIBS := -lopengl32 -lglu32 -lwsock32 -lm
  else
    # default is Linux, for all unix SDL
    OPTS := -DLINUX
    LDFLAGS=-L/usr/X11R6/lib
    LIBS := -lGL -lGLU -lm
    # -L/usr/X11R6/lib is needed by Linux 2.4 and others that still have
    # the GLU libraries in an X11 directory.
    # -lm is needed for pow, powf, and other MATH1 functions.
    SDL_CONFIG=1
  endif

  ifdef SDL_CONFIG
    OPTS += $(shell sdl-config --cflags)
    LIBS += $(shell sdl-config --libs)
  else
    # Extracted from sdl-config script
    ifdef CC_MINGW
      OPTS += -I\SDL\include\SDL
#      OPTS += -I/mingw/include/SDL
      LIBS += -L/SDL/lib
#      LIBS += -L/mingw/lib
#      LIBS += -L/windows/system32
      LIBS += -lsdl -lmingw32 -mwindows
    endif
    ifdef CC_WATCOM
      OPTS += -I/SDL/include/SDL
#     OPTS += -I/watcom/h/SDL
      LIBS += -L/SDL/lib
#     LIBS += -L/windows/system32
      LIBS += -lsdl -lwatcom -mwindows
    endif
    OPTS += -D_GNU_SOURCE=1 -Dmain=SDL_main
    LIBS += -lSDLmain -lSDL
  endif

  OPTS += -DHWRENDER -DSDL -I.

  ifdef HAVE_MIXER
    OPTS += -DHAVE_MIXER
    LIBS += -lSDL_mixer 
  endif



#=================================
else
ifeq ($(SMIF), LINUX_X11)
  # Requires X, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV=linux_x/musserv
  SNDSERV=linux_x/sndserv

  #use the x86 asm code
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER=-DPOLL_POINTER

  OPTS=-DLINUX -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  ifdef CDMUS
    OPTS += -DMUSSERV
  endif
  # objs common to xshm and ggi, video is added later
  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/searchp.o $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o \
    $(O)/r_opengl.o $(O)/ogl_x11.o

  X=1

  ifdef X
    OPTS := $(OPTS) $(POLL_POINTER) -I. -I/usr/X11R6/include
    LDFLAGS=-L/usr/X11R6/lib
    LIBS=-lGL -lGLU -lXext -lX11 -lm -ldl -lXxf86vm

    #WITH_DGA=1
    ifdef WITH_DGA
      OPTS += -DWITH_DGA
      LIBS += -lXxf86dga -lXext
    endif #WITH_DGA

# name of the exefile
    EXENAME=llxdoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_xshm.o
  else
    OPTS := $(OPTS) -DVID_GGI -I.
    LDFLAGS=
    LIBS=-lggi -lm
# name of the exefile
    EXENAME=llggidoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_ggi.o
  endif


#=================================
else
ifeq ($(SMIF), FREEBSD_X11)
  # Requires X11/Xlib, and xshm or ggi libs
  INTERFACE=linux_x

  MUSSERV=linux_x/musserv
  SNDSERV=linux_x/sndserv
  OSVERSION:=$(shell /sbin/sysctl -n kern.osreldate)
  OSMAJOR:=$(shell echo $(OSVERSION) | sed 's/.....$$//')

  #use the x86 asm code
  #USEASM=1

  # Uncomment if you want to use the POLL_POINTER hack
  #POLL_POINTER=-DPOLL_POINTER

  OPTS=-DLINUX -DFREEBSD -DOLD_SOUND_DRIVER -DSNDSERV -DHWRENDER
  # objs common to xshm and ggi, video is added later
  # Note: free-bsd version of cdmus
  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus_fbsd.o
  OBJS=$(O)/i_system.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/searchp.o $(O)/filesrch.o $(O)/dosstr.o $(O)/endtxt.o \
    $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
    $(O)/hw_md2.o $(O)/hw_cache.o $(O)/hw_trick.o

  ifndef FBSD_GGI
    X=1
  endif

  ifdef X
    OPTS := $(OPTS) $(POLL_POINTER) -I. -I/usr/X11R6/include
    LDFLAGS=-L/usr/X11R6/lib
    LIBS=-lXext -lX11 -lm -lXxf86vm -lipx -lkvm -pthread
    # name of the exefile
    EXENAME=llxdoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_xshm.o
  else #ifdef X
    OPTS := $(OPTS) -DVID_GGI -I. -I/usr/local/include
    LDFLAGS=
    LIBS=-lggi -lm -lipx
    # name of the exefile
    EXENAME=llggidoom
    SFLAGS=-g $(OPTS)
    OBJS += $(O)/i_video_ggi.o
  endif #ifdef X


#=================================
else
ifeq ($(SMIF), OS2_NATIVE)
  #Requires OS2 libs, os2.h, os2me.h, fourcc.h, DIVE libs, and mmpm2 libs
  #Does not have cdmusic support
  INTERFACE=os2
  NASMFORMAT=coff

#use the x86 asm code
  NASM=nasm
#USEASM=1
  PROFILEMODE=1
# options
  OPTS=-Zmt -D__OS2__ -I.. -I. -Ios2

  DEBUGLIBS=-lmmpm2 -lsocket -g
  LIBS= $(INTERFACE)/I_pm.def -lmmpm2 -lsocket -g

  SFLAGS=-g $(OPTS)

  MAINOBJ := $(O)/main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_video.o $(O)/i_sound.o $(O)/i_net.o \
    $(O)/i_dart.o $(O)/i_dive.o $(O)/I_pm.o \
    $(O)/printf.o $(O)/I_pm.res $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe


#=================================
else
ifeq ($(SMIF), WIN32_NATIVE)
  # Requires windows.h, windowsx.h, ddraw.h and ddraw libs, fmod libs
  INTERFACE=win32
  NASMFORMAT=coff

#use the x86 asm code
  #USEASM=1

# options
  OPTS= -DWIN32 -g

  DEBUGLIBS=-lfmod -lwinmm -lsocket
  LIBS=-lfmod -lwinmm -lsocket

  SFLAGS=

  MAINOBJ := $(O)/win_main.o
  OBJS=$(O)/win_sys.o $(O)/win_vid.o $(O)/win_snd.o \
    $(O)/win_net.o $(O)/win_cd.o $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe

#=================================
else
ifeq ($(SMIF), DJGPPDOS_NATIVE)
  # Requires allegro libs, bcd libs (cd-music)
  RM=del
  INTERFACE=djgppdos
  NASMFORMAT=coff

#use the x86 asm code
  #USEASM=1

# options
  OPTS=-DPC_DOS -g

  DEBUGLIBS=-lalleg -lsocket
  LIBS=-lalleg -lsocket
  ifdef CDMUS
     LIBS+=-lbcd
     DEBUGLIBS+=-lbcd
  endif

  SFLAGS=

  MAINOBJ := $(O)/i_main.o
  CDMUSOBJ := $(O)/i_cdmus.o
  OBJS=$(O)/i_system.o $(O)/i_video.o $(O)/vid_vesa.o \
    $(O)/i_sound.o $(O)/i_net.o \
    $(O)/filesrch.o

# name of the exefile
  EXENAME=legacy.exe
endif
endif
endif
endif
endif
endif
#=================================




# use assembly routines?
ifdef USEASM
   OBJS:= $(OBJS) $(O)/tmap.o $(O)/vid_copy.o
   OPTS:= $(OPTS) -DUSEASM
endif

# CD-Music
ifdef CDMUS
   OBJS += $(CDMUSOBJ)
   OPTS += -DCDMUS
endif

# compiler and linker flags
CFLAGS = $(WFLAGS)

ifdef PROFILEMODE
# build with gprof profiling information
  CFLAGS += -g -pg -fomit-frame-pointer
else
ifdef DEBUG
# No -fomit-frame-pointer because debugging on some machines depends upon it.
  #CFLAGS += -g3 -O0
  CFLAGS += $(DEBUGFLAGS) -fno-guess-branch-probability
else
# build a normal optimized version
  #CFLAGS += -O3
  CFLAGS += $(OPTLEV) -fomit-frame-pointer
endif
endif

CFLAGS += -ffast-math -fno-strict-aliasing $(OPTS)

ifdef MAC
else
  LDFLAGS += -Xlinker --warn-common
endif

SNDSERV_EXTRAFLAGS:=
ifdef HAVE_ESD
  SNDSERV_EXTRAFLAGS:= HAVE_ESD=1
endif



# not too sophisticated dependency
# objs that are in main directories, INTERFACE specific obj files are in
# listed in the interface #if above
OBJS :=  $(OBJS) \
	$(O)/i_tcp.o	\
	$(O)/sounds.o  $(O)/qmus2mid.o  $(O)/s_sound.o  $(O)/s_amb.o	 \
	$(O)/v_video.o  $(O)/screen.o	\
	$(O)/z_zone.o	\
	$(O)/dstrings.o  $(O)/tables.o  $(O)/info.o	\
		$(O)/f_finale.o  \
		$(O)/f_wipe.o	 \
		$(O)/d_main.o	 \
		$(O)/d_net.o	 \
		$(O)/d_items.o	 \
		$(O)/g_game.o	 \
		$(O)/m_menu.o	 \
		$(O)/m_misc.o	 \
		$(O)/m_argv.o	 \
		$(O)/m_bbox.o	 \
		$(O)/m_fixed.o	 \
		$(O)/m_swap.o	 \
		$(O)/m_cheat.o	 \
		$(O)/m_random.o  \
		$(O)/am_map.o	 \
		$(O)/p_ceilng.o  \
		$(O)/p_doors.o	 \
		$(O)/p_enemy.o	 \
		$(O)/p_floor.o	 \
		$(O)/p_genlin.o  \
		$(O)/p_info.o	 \
		$(O)/p_inter.o	 \
		$(O)/p_lights.o  \
		$(O)/p_map.o	 \
		$(O)/p_maputl.o  \
		$(O)/p_plats.o	 \
		$(O)/p_pspr.o	 \
		$(O)/p_setup.o	 \
		$(O)/p_sight.o	 \
		$(O)/p_spec.o	 \
		$(O)/p_switch.o  \
		$(O)/p_mobj.o	 \
		$(O)/p_telept.o  \
		$(O)/p_tick.o	 \
		$(O)/p_saveg.o	 \
		$(O)/p_user.o	 \
		$(O)/r_bsp.o	 \
		$(O)/r_data.o	 \
		$(O)/r_draw.o	 \
		$(O)/r_main.o	 \
		$(O)/r_plane.o	 \
		$(O)/r_segs.o	 \
		$(O)/r_sky.o	 \
		$(O)/r_things.o  \
		$(O)/w_wad.o	 \
		$(O)/wi_stuff.o  \
		$(O)/st_lib.o	 \
		$(O)/st_stuff.o  \
		$(O)/hu_stuff.o  \
		$(O)/p_fab.o	 \
		$(O)/dehacked.o  \
		$(O)/g_input.o	 \
		$(O)/console.o	 \
		$(O)/command.o	 \
		$(O)/d_netcmd.o  \
		$(O)/d_clisrv.o  \
		$(O)/r_splats.o  \
		$(O)/g_state.o	 \
		$(O)/t_array.o   \
		$(O)/t_func.o	 \
		$(O)/t_oper.o	 \
		$(O)/t_parse.o	 \
		$(O)/t_prepro.o  \
		$(O)/t_script.o  \
		$(O)/t_spec.o	 \
		$(O)/t_vari.o	 \
		$(O)/d_netfil.o  \
		$(O)/mserv.o	 \
		$(O)/p_hsight.o  \
		$(O)/sb_bar.o	 \
		$(O)/p_heretic.o \
		$(O)/md5.o	 \
		$(O)/p_chex.o	 \
		$(O)/b_game.o	\
		$(O)/b_look.o	\
		$(O)/b_node.o	\
		$(O)/b_search.o



#=======================================================
# rules
#=======================================================
.PHONY : all clean asm dll wad


ifeq ($(SMIF), LINUX_X11)
all:	 $(BIN)/$(EXENAME) dll musserv sndserv

else
ifeq ($(SMIF), FREEBSD_X11)
all:	 $(BIN)/$(EXENAME) dll sndserv

else
all:	 $(BIN)/$(EXENAME)

endif
endif


clean:
	$(RM) $(O)/*
ifeq ($(SMIF), LINUX_X11)
	$(RM) $(SNDSERV)/linux/*
	$(RM) $(MUSSERV)/linux/*
else
ifeq ($(SMIF), FREEBSD_X11)
	$(RM) $(SNDSERV)/linux/*
	$(RM) $(MUSSERV)/linux/*
endif
endif



$(O):
	@mkdir $(O)

.PHONY : versionstring
# This may fail because, (a) svn not installed, (b) not a svn directory.
# This compiles d_main a second time, with SVN_REV set.
versionstring:
ifdef SVN_ENABLE
	$(CC) -c $(CFLAGS) -DSVN_REV=\"`svn info | grep Revision | sed -e 's/Revision: //'`\" d_main.c -o $(O)/d_main.o
endif


wad	:
ifdef WAD
	@echo "Building a new legacy.wad using an old version $(WAD)..."
	if test ! -d ../wad; then mkdir ../wad; fi
	cd ../wad ; \
	../../common/bin/wadtool -x $(WAD) ; \
	cp ../trunk/resources/* . ; \
	../../common/bin/wadtool -c legacy.wad legacy.wad.inventory ; \
	mv legacy.wad $(BIN) ; \
	rm *.txt *.h *.lmp
	@echo "Finished building legacy.wad."
else
	@echo "Usage: make wad WAD=/path/to/old/legacy.wad"
endif




#make a disasm of the exe
asm: $(BIN)/$(EXENAME)
	objdump -d $(BIN)/$(EXENAME) --no-show-raw-insn > $(BIN)/doomlegacy.s


# executable
$(BIN)/$(EXENAME): $(O) $(OBJS) $(MAINOBJ) versionstring
#	 @mkdir $(BIN)
	@echo Linking...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(MAINOBJ) \
	-o $(BIN)/$(EXENAME) $(LIBS)



#dll
dll : $(O)/r_opengl.o $(O)/ogl_x11.o
	$(CC) -O3 -o $(BIN)/r_opengl.so -shared -nostartfiles $(O)/r_opengl.o $(O)/ogl_x11.o -L/usr/X11R6/lib -L/usr/local/lib -lX11 -lXext -lGL -lGLU -lm

musserv:
	@mkdir -p $(MUSSERV)/linux
	cd $(MUSSERV) \
	&& make -f Makefile.linux

sndserv:
	@mkdir -p $(SNDSERV)/linux
	cd $(SNDSERV) \
	&& $(MAKE) $(SNDSERV_EXTRAFLAGS)



ifeq ($(SMIF), SDL)
$(O)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -c $< -o $@

else 
# LINUX_X11, FREEBSD_X11 at least
$(O)/r_opengl.o: hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -c $< -o $@
endif


$(O)/ogl_x11.o:  hardware/r_opengl/ogl_x11.c hardware/r_opengl/r_opengl.h
	$(CC) $(CFLAGS) -shared -nostartfiles -I/usr/X11R6/include -c $< -o $@ 


#=======================================================

$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: $(INTERFACE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o: hardware/%.c
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(SMIF), OS2_NATIVE)
$(O)/%.res: os2/%.rc
	rc.exe -i .\os2 -r $< $@
endif

#=======================================================
#[WDJ] Dependencies, replaces makedep.mak, which was unmaintained.

# This PHONY disables automatic dep file generation when a file changes.
# Comment it out for automatic dep file generation.
.PHONY : ../dep/*.dep

.PHONY : depend
depend: ../dep/main1.dep ../dep/$(INTERFACE).dep

../dep/main1.dep : a*.c b*.c d*.c f*.c g*.c h*.c
../dep/main2.dep : i*.c m*.c p*.c
../dep/main3.dep : q*.c r*.c s*.c t*.c v*.c z*.c
# none: e*.c j*.c k*.c l*.c n*.c o*.c
../dep/hardware.dep : hardware/*.c hardware/*/*.c
# $(INTERFACE).dep : $(INTERFACE)/*.c
../dep/djgppdos.dep : djgppdos/*.c
../dep/linux_x.dep : linux_x/*.c linux_x/musserv/*.c linux_x/sndserv/*.c
../dep/os_2.dep : os2/*.c
../dep/macos.dep : macos/*.c
../dep/sdl.dep : sdl/*.c
../dep/win32.dep : win32/*.c

vpath %.o $(O)

ifdef WIN32
../dep :
	@mkdir ..\dep;

%.dep :
	@echo "Making dependencies $(@F)"
	$(CC) $(CFLAGS) -MM $^ > ..\dep\$(@F)
else
../dep :
	@mkdir ../dep;

%.dep :
	@echo "Making dependencies $(@F)"
	if test ! -d ../dep; then mkdir ../dep; fi
	$(CC) $(CFLAGS) -MM $^ > ../dep/$(@F)
	sed --separate -e "s@^[a-zA-Z0-9_]*.o:@\$$(O)/&@" ../dep/$(@F) > sed.dep
	mv sed.dep ../dep/$(@F)
endif

$(O)/*.o : doomdef.h

$(O)/r_draw.o : r_draw.c r_draw8.c r_draw16.c r_draw24.c r_draw32.c

include ../dep/main1.dep
include ../dep/main2.dep
include ../dep/main3.dep
include ../dep/hardware.dep
include ../dep/$(INTERFACE).dep

#############################################################
#
#############################################################
