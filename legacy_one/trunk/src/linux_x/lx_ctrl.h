// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: lx_ctrl.h 1403 2018-07-06 09:49:21Z wesleyjohnson $
//
// Copyright (C) 2021 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
//
// DESCRIPTION:
//   Sound device defines, handling.
//   Sound Menu Controls.
//   Used by Internal-Sound Interface, and by SoundServer.
//
//-----------------------------------------------------------------------------

#ifndef LX_CTRL_DEV_LOGIC
#define LX_CTRL_DEV_LOGIC

#include "command.h"

#ifdef DEV_OPT_OSS
#ifndef DEV_OSS
# define DEV_OSS
#endif
# define NEED_DLOPEN
#endif

#ifdef DEV_OPT_ESD
#ifndef DEV_ESD
# define DEV_ESD
#endif
# define NEED_DLOPEN
#endif

#ifdef DEV_OPT_ALSA
#ifndef DEV_ALSA
# define DEV_ALSA
#endif
# define NEED_DLOPEN
#endif

#ifdef DEV_OPT_JACK
#ifndef DEV_JACK
# define DEV_JACK
#endif
# define NEED_DLOPEN
#endif

#ifdef DEV_OPT_PULSE
#ifndef DEV_PULSE
# define DEV_PULSE
#endif
# define NEED_DLOPEN
#endif

// Sound Device selections for X11
typedef enum {
   SD_NULL = 0,
   SD_S1 = 1,
   SD_S2 = 2,
   SD_S3 = 3,
#if 1   
   SD_OSS,
   SD_ESD,
   SD_ALSA,
   SD_PULSE,
   SD_JACK,
#else   
   SD_OSS = 4,
   SD_ESD = 5,
   SD_ALSA = 6,
   SD_PULSE = 7,
   SD_JACK = 8,
#endif
   SD_DEV6,
   SD_DEV7,
   SD_DEV8,
   SD_DEV9,
} sound_dev_e;


// Detect multiple sound devices.
//  SOUND_DEV1  when 1 DEV
//  SOUND_DEVICE_OPTION  when 2 or more DEV

#ifdef DEV_OSS
#    define SOUND_DEV1  SD_OSS
#endif

#ifdef DEV_ALSA
#  ifdef SOUND_DEV1
#    ifndef SOUND_DEVICE_OPTION
#      define SOUND_DEVICE_OPTION
#    endif
#  else
#    define SOUND_DEV1  SD_ALSA
#  endif
#endif

#ifdef DEV_ESD
#  ifdef SOUND_DEV1
#    ifndef SOUND_DEVICE_OPTION
#      define SOUND_DEVICE_OPTION
#    endif
#  else
#    define SOUND_DEV1  SD_ESD
#  endif
#endif

#ifdef DEV_JACK
#  ifdef SOUND_DEV1
#    ifndef SOUND_DEVICE_OPTION
#      define SOUND_DEVICE_OPTION
#    endif
#  else
#    define SOUND_DEV1  SD_JACK
#  endif
#endif

#ifdef DEV_PULSE
#  ifdef SOUND_DEV1
#    ifndef SOUND_DEVICE_OPTION
#      define SOUND_DEVICE_OPTION
#    endif
#  else
#    define SOUND_DEV1  SD_PULSE
#  endif
#endif


// Sound device selection menu and control.

#ifdef SOUND_DEVICE_OPTION

// The values of snd_opt are defined in linux_x/i_sound.c
// to ensure that menu values and implementation always match.
extern CV_PossibleValue_t snd_opt_cons_t[];
// The actual consvar are declared in s_sound.c
extern consvar_t cv_snd_opt;

#else
// Static sound device selection

// Without SOUND_DEVICE_OPTION, there can only be one sound device.

// End Static sound device selection
#endif

#ifdef SNDSERV
extern consvar_t cv_sndserver_cmd;
extern consvar_t cv_sndserver_arg;
#endif

#ifdef MUSSERV
// The values of musserv_opt are defined in linux_x/i_sound.c
// to ensure that menu values and implementation always match.
extern CV_PossibleValue_t musserv_opt_cons_t[];
// The actual consvar are declared in s_sound.c
extern consvar_t cv_musserver_opt;

extern consvar_t cv_musserver_cmd;
extern consvar_t cv_musserver_arg;
#endif

#endif