// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: soundsrv.c,v $
// Revision 1.8  2001/05/29 22:18:41  bock
// Small BSD commit: build r_opengl.so, sndserver
//
// Revision 1.7  2000/09/01 19:34:37  bpereira
// no message
//
// Revision 1.6  2000/04/30 19:50:37  metzgermeister
// no message
//
// Revision 1.5  2000/04/28 19:26:10  metzgermeister
// musserver fixed, sndserver amplified accordingly
//
// Revision 1.4  2000/04/22 20:30:00  metzgermeister
// fix amplification by 4
//
// Revision 1.3  2000/03/28 16:18:42  linuxcub
// Added a command to the Linux sound-server which sets a master volume.
//
// Revision 1.2  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      UNIX soundserver, run as a separate process,
//       started by DOOM program.
//      Originally conceived fopr SGI Irix,
//       mostly used with Linux voxware.
//
//-----------------------------------------------------------------------------

#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
  // malloc
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
  // memset

#include "doomdef.h"
#include "doomtype.h"
#include "soundsrv.h"

// [WDJ] Removed old duplicate sounds.c sounds.h.  Were not kept up.
#include "../../sounds.h"
  // NUMSFX
#include "../../s_sound.h"
  // SURROUND_SEP

// #define DEBUG   1

#define NUMCHAN    8

extern byte  audio_8bit_flag;


// an internal time keeper
static int      mytime = 0;


// Information for loaded sfx.
typedef struct {
    int       length;
    uint32_t  flags;
    byte *    data;
} server_sfx_t;

static server_sfx_t   sfx[NUMSFX];


typedef struct {
    // the channel data, current position
    byte *  data;  // NULL=inactive
    // the channel data end pointer
    byte *  data_end;
    // the channel step amount
    unsigned int step;
    // 0.16 bit remainder of last step
    unsigned int remainder;
    // volumes
    int  left_volume, right_volume;
    // the channel volume lookup, modifed by master volume
    int  * left_vol_tab, * right_vol_tab;
    // time that the channel started playing
    int  start_time;
    // sfx id of the playing sound effect
    uint16_t  id;                  
    // the channel handle
    uint16_t  handle;

#ifdef SURROUND_SOUND
    byte      invert_right;
#endif
} channel_info_t;

static channel_info_t   channel[NUMCHAN];


// mixing buffer
static int16_t  mixbuffer[MIXBUFFERSIZE];


int             snd_verbose=1;

int             steptable[256];

int             volume_lookup[128][256];

int             master_volume=31; /* 0..31 */


void set_channel_volume( channel_info_t * chp, int leftvol, int rightvol )
{
    if( leftvol < 0 ) leftvol = 0;
    if( leftvol > 127 ) leftvol = 127;
    chp->left_volume = leftvol;
    chp->left_vol_tab = &volume_lookup[(leftvol * master_volume)/31][0];
    if( rightvol < 0 ) rightvol = 0;
    if( rightvol > 127 ) rightvol = 127;
    chp->right_volume = rightvol;
    chp->right_vol_tab = &volume_lookup[(rightvol * master_volume)/31][0];
}

// new_volume : 0..31
void set_master_volume( int new_volume )
{
    register channel_info_t * chp = &channel[0];
#ifdef DEBUG
    fprintf(stderr, "SS: Master Volume = %i\n", new_volume );
#endif

    if( new_volume == master_volume )
       return;

    if( new_volume > 31 )  new_volume = 31;
    master_volume = new_volume;

    for ( chp = &channel[0]; chp < &channel[NUMCHAN]; chp++ )
    {
        if( chp->data )
        {
            chp->left_vol_tab = &volume_lookup[(chp->left_volume * new_volume)/31][0];
            chp->right_vol_tab = &volume_lookup[(chp->right_volume * new_volume)/31][0];
        }
    }
}



int mix(void)
{
    register unsigned int   sample;
    register int            dl, dr;
    uint16_t                sdl, sdr;
    
    int16_t               * leftout, * rightout,  * leftend;
    byte  *                 bothout;
    
    int  mix_step = 2;
    
    leftout = mixbuffer;
    rightout = mixbuffer+1;
    bothout = (byte *)mixbuffer;

    leftend = mixbuffer + SAMPLECOUNT*mix_step;

    // mix into the mixing buffer
    while (leftout != leftend)
    {

        dl = 0;
        dr = 0;

        if (channel[0].data)
        {
            sample = *channel[0].data;
            dl += channel[0].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[0].invert_right )
                dr -= channel[0].right_vol_tab[sample];
            else
                dr += channel[0].right_vol_tab[sample];
#else
            dr += channel[0].right_vol_tab[sample];
#endif
            channel[0].remainder += channel[0].step;
            channel[0].data += channel[0].remainder >> 16;
            channel[0].remainder &= 0xFFFF;

            if (channel[0].data >= channel[0].data_end)
                channel[0].data = 0;
        }

        if (channel[1].data)
        {
            sample = *channel[1].data;
            dl += channel[1].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[1].invert_right )
                dr -= channel[1].right_vol_tab[sample];
            else
                dr += channel[1].right_vol_tab[sample];
#else
            dr += channel[1].right_vol_tab[sample];
#endif
            channel[1].remainder += channel[1].step;
            channel[1].data += channel[1].remainder >> 16;
            channel[1].remainder &= 0xFFFF;

            if (channel[1].data >= channel[1].data_end)
                channel[1].data = 0;
        }

        if (channel[2].data)
        {
            sample = *channel[2].data;
            dl += channel[2].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[2].invert_right )
                dr -= channel[2].right_vol_tab[sample];
            else
                dr += channel[2].right_vol_tab[sample];
#else
            dr += channel[2].right_vol_tab[sample];
#endif
            channel[2].remainder += channel[2].step;
            channel[2].data += channel[2].remainder >> 16;
            channel[2].remainder &= 0xFFFF;

            if (channel[2].data >= channel[2].data_end)
                channel[2].data = 0;
        }
        
        if (channel[3].data)
        {
            sample = *channel[3].data;
            dl += channel[3].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[3].invert_right )
                dr -= channel[3].right_vol_tab[sample];
            else
                dr += channel[3].right_vol_tab[sample];
#else
            dr += channel[3].right_vol_tab[sample];
#endif
            channel[3].remainder += channel[3].step;
            channel[3].data += channel[3].remainder >> 16;
            channel[3].remainder &= 0xFFFF;

            if (channel[3].data >= channel[3].data_end)
                channel[3].data = 0;
        }
        
        if (channel[4].data)
        {
            sample = *channel[4].data;
            dl += channel[4].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[4].invert_right )
                dr -= channel[4].right_vol_tab[sample];
            else
                dr += channel[4].right_vol_tab[sample];
#else
            dr += channel[4].right_vol_tab[sample];
#endif
            channel[4].remainder += channel[4].step;
            channel[4].data += channel[4].remainder >> 16;
            channel[4].remainder &= 0xFFFF;

            if (channel[4].data >= channel[4].data_end)
                channel[4].data = 0;
        }
        
        if (channel[5].data)
        {
            sample = *channel[5].data;
            dl += channel[5].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[5].invert_right )
                dr -= channel[5].right_vol_tab[sample];
            else
                dr += channel[5].right_vol_tab[sample];
#else
            dr += channel[5].right_vol_tab[sample];
#endif
            channel[5].remainder += channel[5].step;
            channel[5].data += channel[5].remainder >> 16;
            channel[5].remainder &= 0xFFFF;

            if (channel[5].data >= channel[5].data_end)
                channel[5].data = 0;
        }
        
        if (channel[6].data)
        {
            sample = *channel[6].data;
            dl += channel[6].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[6].invert_right )
                dr -= channel[6].right_vol_tab[sample];
            else
                dr += channel[6].right_vol_tab[sample];
#else
            dr += channel[6].right_vol_tab[sample];
#endif
            channel[6].remainder += channel[6].step;
            channel[6].data += channel[6].remainder >> 16;
            channel[6].remainder &= 0xFFFF;

            if (channel[6].data >= channel[6].data_end)
                channel[6].data = 0;
        }

        if (channel[7].data)
        {
            sample = *channel[7].data;
            dl += channel[7].left_vol_tab[sample];
#ifdef SURROUND_SOUND
            if( channel[7].invert_right )
                dr -= channel[7].right_vol_tab[sample];
            else
                dr += channel[7].right_vol_tab[sample];
#else
            dr += channel[7].right_vol_tab[sample];
#endif
            channel[7].remainder += channel[7].step;
            channel[7].data += channel[7].remainder >> 16;
            channel[7].remainder &= 0xFFFF;

            if (channel[7].data >= channel[7].data_end)
                channel[7].data = 0;
        }

        // Has been char instead of short.
        // if (dl > 127) *leftout = 127;
        // else if (dl < -128) *leftout = -128;
        // else *leftout = dl;

        // if (dr > 127) *rightout = 127;
        // else if (dr < -128) *rightout = -128;
        // else *rightout = dr;
        
        dl <<= 3;
        dr <<= 3;

        if (!audio_8bit_flag)
        {
            if (dl > 0x7fff)
                *leftout = 0x7fff;
            else if (dl < -0x8000)
                *leftout = -0x8000;
            else
                *leftout = dl;

            if (dr > 0x7fff)
                *rightout = 0x7fff;
            else if (dr < -0x8000)
                *rightout = -0x8000;
            else
                *rightout = dr;
        }
        else
        {
            if (dl > 0x7fff)
                dl = 0x7fff;
            else if (dl < -0x8000)
                dl = -0x8000;
            sdl = dl ^ 0xfff8000;

            if (dr > 0x7fff)
                dr = 0x7fff;
            else if (dr < -0x8000)
                dr = -0x8000;
            sdr = dr ^ 0xfff8000;

            *bothout++ = (((sdr + sdl) / 2) >> 8);
        }

        leftout += mix_step;
        rightout += mix_step;

    }
    return 1;
}

static struct timeval           last={0,0};
//static struct timeval         now;

static struct timezone          m_timezone;

void updatesounds(void)
{

    mix();
    I_SubmitOutputBuffer(mixbuffer, SAMPLECOUNT);

}

//  vol : volume, 0..255
//  sep : separation, +/- 127, SURROUND_SEP special operation
void addsfx( int sfxid, int vol, int step, int sep, uint16_t handle )
{
    channel_info_t * chp, * chp2;

    int   oldest = mytime;
    int   rightvol, leftvol;

    // Play these sound effects only one at a time.
    // [WDJ] Implemented using flags from Sfx tables.
    if(sfx[sfxid].flags & SFX_single)
    {
        for ( chp2 = &channel[0]; chp2 < &channel[NUMCHAN]; chp2++ )
        {
            if (chp2->data && (chp2->id == sfxid))
            {
                if( sfx[sfxid].flags & SFX_id_fin )
//                    return chp2->handle;  // already have one
                    return;
                // Kill, Reset.
                chp2->data = NULL;  // close existing channel
                break;
            }
        }
    }

    // Find inactive channel, or oldest channel.
    chp = &channel[0];
    for ( chp2 = &channel[0]; chp2 < &channel[NUMCHAN]; chp2++ )
    {
        if( chp2->data == NULL )
        {
            chp = chp2;  // Inactive channel
            break;
        }
        if( chp2->start_time < oldest )
        {
            chp = chp2;  // older channel
            oldest = chp->start_time;
        }
    }

#ifdef DEBUG
    fprintf(stderr, "SS: channel = %i  sfx = %i\n", (chp - channel), sfxid );
#endif
    chp->id = sfxid;
    chp->data = sfx[sfxid].data;
    chp->data_end = sfx[sfxid].data + sfx[sfxid].length;

    chp->step = step;
    chp->remainder = 0;
    chp->start_time = mytime;
    chp->handle = handle;

#ifdef SURROUND_SOUND
    chp->invert_right = 0;
    if( sep == SURROUND_SEP )
    {
        // Use a normal sound data for the left channel (with pan left)
        // and an inverted sound data for the right channel (with pan right)
        leftvol = rightvol = (vol * (224 * 224)) >> 16;  // slight reduction going through panning
        chp->invert_right = 1;  // invert right channel
    }
    else
#endif
    {
        // Separation, that is, orientation/stereo.
        // sep : +/- 127, <0 is left, >0 is right
        sep += 129;  // 129 +/- 127 ; ( 1 - 256 )
        leftvol = vol - ((vol * sep * sep) >> 16);
        sep = 258 - sep;  // -129 +/- 127
        rightvol = vol - ((vol * sep * sep) >> 16);
    }
   
    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
    {
        fprintf(stderr, "rightvol out of bounds\n");
        rightvol = ( rightvol < 0 ) ? 0 : 127;
    }

    if (leftvol < 0 || leftvol > 127)
    {
        fprintf(stderr, "leftvol out of bounds\n");
        leftvol = ( leftvol < 0 ) ? 0 : 127;
    }
    
    set_channel_volume( chp, leftvol, rightvol );
}


#if 0
// [WDJ] Unused
void outputushort(int num)
{

    static unsigned char        buff[5] = { 0, 0, 0, 0, '\n' };
    static char*                badbuff = "xxxx\n";

    // outputs a 16-bit # in hex or "xxxx" if -1.
    if (num < 0)
    {
        write(1, badbuff, 5);
    }
    else
    {
        buff[0] = num>>12;
        buff[0] += buff[0] > 9 ? 'a'-10 : '0';
        buff[1] = (num>>8) & 0xf;
        buff[1] += buff[1] > 9 ? 'a'-10 : '0';
        buff[2] = (num>>4) & 0xf;
        buff[2] += buff[2] > 9 ? 'a'-10 : '0';
        buff[3] = num & 0xf;
        buff[3] += buff[3] > 9 ? 'a'-10 : '0';
        write(1, buff, 5);
    }
}
#endif

void initdata(void)
{

    int         i;
    int         j;
    
    int*        steptablemid = steptable + 128;

    memset( sfx, 0, sizeof(sfx) );
    memset( channel, 0, sizeof(channel) );
    
    gettimeofday(&last, &m_timezone);

    for (i=-128 ; i<128 ; i++)
        steptablemid[i] = pow(2.0, (i/64.0))*65536.0;

    // generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples
    // for (i=0 ; i<128 ; i++)
    // for (j=0 ; j<256 ; j++)
    // vol_lookup[i*256+j] = (i*(j-128))/127;
    
    for (i=0 ; i<128 ; i++)
    {
        for (j=0 ; j<256 ; j++)
            volume_lookup[i][j] = (i*(j-128)*256)/127;
    }
}

// Safely reads the pipe to the buffer, ensuring all bytes are read.
// This needs to be used for all pipe reads of more than 1 byte.
//  buf : to the buffer
//  req : the number of bytes to read
void  read_pipe( void * buf, int req )
{
    // hey, read on a pipe will not always
    // fill the whole buffer 19990203 by Kin
    int tlen = 0;
    while( tlen < req )
    {
        register int rc = read(0, buf+tlen, req-tlen);
        if( rc < 0 )  return;
        tlen += rc;  // bytes read
    }
}

// Load sfx from pipeline
void load_sound_data( void )
{
    server_sfx_t * sfxp;
    server_load_sound_t  sls;

    read_pipe( &sls, sizeof(sls) );  // sfx id, flags, snd_length
   
#ifdef DEBUG
    fprintf(stderr, "SS: load_sound %i, flags=%x, size=%i\n", sls.id, sls.flags, sls.snd_len );
#endif
   
    if( sls.id >= NUMSFX )  return;
    //fprintf(stderr,"%d in...\n",bln);
   
    sfxp = & sfx[sls.id];
    if( sfxp->data )
        free( sfxp->data );

    sfxp->data = malloc(sls.snd_len);
    if( sfxp->data == NULL )
    {
        fprintf(stderr, "Soundserver: sfx %i, memory req %i\n", sls.id, sls.snd_len );
        exit(-2);
    };
    sfxp->flags = sls.flags;
    sfxp->length = sls.snd_len;
    read_pipe( sfxp->data, sls.snd_len );  // the snd data
}


#if 0
// Format of play sound command.
typedef struct {
    uint16_t  sfxid;
    byte      vol;
    byte      pitch;
    int16_t   sep;
    uint16_t  handle;
} server_play_sound_t;
#endif


void play_sound( void )
{
    server_play_sound_t  sps;

    read_pipe( (byte*)&sps, sizeof(sps) );

#ifdef DEBUG
    fprintf(stderr, "SS: play_sound %i, vol=%i, pitch=%i, sep=%i\n",
            sps.sfxid, sps.vol, sps.pitch, sps.sep );
#endif
   
    if( sps.sfxid >= NUMSFX )  return;

    //if (snd_verbose)
    //{
    //  commandbuf[9]=0;
    //  fprintf(stderr, "%s\n", commandbuf);
    //}

    // The handle is determined by the main program, as it needs it
    // to stop the sound.
    addsfx( sps.sfxid, sps.vol, steptable[sps.pitch], sps.sep, sps.handle);
}

// Stop sound based on handle.
void stop_sound( void )
{
    uint16_t  handle16;
    channel_info_t * chp;

    read_pipe(&handle16, sizeof(uint16_t));    // Get stop sound handle

    for( chp = &channel[0]; chp < &channel[NUMCHAN]; chp++ )
    {
        if( chp->data && (chp->handle == handle16) )
        {
            chp->data = NULL;  // stop the channel
            break;
        }
    }
}


void quit(void)
{
    I_ShutdownMusic();
    I_ShutdownSound();
    exit(0);
}



fd_set          fdset;
fd_set          scratchset;



int main ( int c, char** v )
{

    int         done = 0;
    int         rc;
    int         nrc;

    unsigned char       commandbuf[10];
    struct timeval      zerowait = { 0, 0 };

    int         i;
    int         waiting_to_finish=0;

    // init any data
    initdata();         

    I_InitSound(11025, 16);

    I_InitMusic();

    if (snd_verbose)
        fprintf(stderr, "ready\n");
    
    // parse commands and play sounds until done
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    while (!done)
    {
        mytime++;

        if (!waiting_to_finish)
        {
            do {
                scratchset = fdset;
                rc = select(FD_SETSIZE, &scratchset, 0, 0, &zerowait);

                if (rc > 0)
                {
                    //  fprintf(stderr, "select is true\n");
                    // got a command
                    nrc = read(0, commandbuf, 1);

                    if (!nrc)
                    {
                        done = 1;
                        rc = 0;
                    }
                    else
                    {
                        //if (snd_verbose)
                        //    fprintf(stderr, "cmd: %c", commandbuf[0]);

                        switch (commandbuf[0])
                        {
                        case 'v':
                            // get master volume
                            read(0, commandbuf, 1);
                            set_master_volume( commandbuf[0] );
                            break;
                            
                        case 'p':
                            // play a new sound effect
                            play_sound();
                            break;
                        case 's':
                            // stop sound
                            stop_sound();
                            break;
                        case 'l': {
                            load_sound_data();
                            break;
                        }
                        case 'q':
                            // no '\n' 19990201 by Kin
                            //read(0, commandbuf, 1);
                            waiting_to_finish = 1;
                            rc = 0;
                            break;
                            
                            //case 's':
                            //{
                            //  int fd;
                            //  read_pipe(commandbuf, 3);
                            //  commandbuf[2] = 0;
                            //  fd = open((char*)commandbuf, O_CREAT|O_WRONLY, 0644);
                            //  commandbuf[0] -= commandbuf[0]>='a' ? 'a'-10 : '0';
                            //  commandbuf[1] -= commandbuf[1]>='a' ? 'a'-10 : '0';
                            //  sndnum = (commandbuf[0]<<4) + commandbuf[1];
                            //  write(fd, sfx[sndnum].data, sfx[sndnum].length);
                            //  close(fd);
                            //}
                            //break;
                        default:
                            fprintf(stderr, "Did not recognize command %d\n",commandbuf[0]);
                            break;
                        }
                    }
                }
                else if (rc < 0)
                {
                    quit();
                }
            } while (rc > 0);
        }

        updatesounds();

        if (waiting_to_finish)
        {
            for(i=0 ; i<NUMCHAN; i++)
               if( channel[i].data )  break;  // busy channel found
            
            if (i==NUMCHAN)  // no busy channels found
                done=1;
        }

    }

    quit();
    return 0;
}
