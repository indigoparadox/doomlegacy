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
// $Log: f_wipe.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Mission begin melt/wipe screen special effect.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"

#include "m_random.h"
#include "f_wipe.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "r_draw.h" // transtable
#include "p_pspr.h" // tr_transxxx
#include "z_zone.h"

//--------------------------------------------------------------------------
//                        SCREEN WIPE PACKAGE
//--------------------------------------------------------------------------

// when zero, stop the wipe
static boolean  go = 0;

static byte*    wipe_scr_start;
static byte*    wipe_scr_end;
static byte*    wipe_scr;


void wipe_ColMajorXform ( short*        array,
                                int           width,
                                int           height )
{
    int         x;
    int         y;
    short*      dest;

    dest = (short*) Z_Malloc(width*height*2, PU_STATIC, 0);

    for(y=0;y<height;y++)
        for(x=0;x<width;x++)
            dest[x*height+y] = array[y*width+x];

    memcpy(array, dest, width*height*2);

    Z_Free(dest);

}


int wipe_initColorXForm ( int   width,
                          int   height,
                          int   ticks )
{
    memcpy(wipe_scr, wipe_scr_start, width*height*vid.bytepp);
    return 0;
}

/* BP:the original one, work only in hicolor
int wipe_doColorXForm ( int   width,
                        int   height,
                        int   ticks )

{
    boolean     changed;
    byte*       w;
    byte*       e;
    int         newval;

    changed = false;
    w = wipe_scr;
    e = wipe_scr_end;

    while (w!=wipe_scr+width*height)
    {
        if (*w != *e)
        {
            if (*w > *e)
            {
                newval = *w - ticks;
                if (newval < *e)
                    *w = *e;
                else
                    *w = newval;
                changed = true;
            }
            else if (*w < *e)
            {
                newval = *w + ticks;
                if (newval > *e)
                    *w = *e;
                else
                    *w = newval;
                changed = true;
            }
        }
        w++;
        e++;
    }

    return !changed;

}
*/


int wipe_doColorXForm ( int   width,
                        int   height,
                        int   ticks )

{
    boolean     changed;
    byte*       w;
    byte*       e;
    byte        newval;
    static int  slowdown=0;
    changed = false;

    while(ticks--)
    {
        // slowdown
        if(slowdown++) { slowdown=0;return false; }
        
        w = wipe_scr;
        e = wipe_scr_end;
        
        
        while (w!=wipe_scr+width*height)
        {
            if (*w != *e)
            {
                if((newval=translucenttables[TRANSLU_TABLE_more+(*e<<8)+*w])==*w)
                    if((newval=translucenttables[TRANSLU_TABLE_med+(*e<<8)+*w])==*w)
                        if((newval=translucenttables[TRANSLU_TABLE_more+(*w<<8)+*e])==*w)
                            newval=*e;
                *w=newval;
                changed = true;
            }
            w++;
            e++;
        }
    }
    return !changed;
}

int wipe_exitColorXForm ( int   width,
                          int   height,
                          int   ticks )
{
    return 0;
}


static int*     y;


int wipe_initMelt ( int   width,
                    int   height,
                    int   ticks )
{
    int i, r;

    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width*height*vid.bytepp);

    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_ColMajorXform((short*)wipe_scr_start, width*vid.bytepp/2, height);
    wipe_ColMajorXform((short*)wipe_scr_end, width*vid.bytepp/2, height);

    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(width*sizeof(int), PU_STATIC, 0);
    y[0] = -(M_Random()%16);
    for (i=1;i<width;i++)
    {
        r = (M_Random()%3) - 1; 
        y[i] = y[i-1] + r;
        if (y[i] > 0) y[i] = 0;
        else if (y[i] == -16) y[i] = -15;
    }
    // dup for normal speed in high res
    for (i=0;i<width;i++)
        y[i]*=vid.dupy;

    return 0;
}


int wipe_doMelt ( int   width,
                  int   height,
                  int   ticks )
{
    int         i;
    int         j;
    int         dy;
    int         idx;

    short*      s;
    short*      d;
    boolean     done = true;

    width = (width * vid.bytepp) / 2;

    while (ticks--)
    {
        for (i=0;i<width;i++)
        {
            if (y[i]<0)
            {
                y[i]++; done = false;
            }
            else if (y[i] < height)
            {
                dy = (y[i] < 16) ? y[i]+1 : 8;
                dy *= vid.dupy;
                if (y[i]+dy >= height) dy = height - y[i];
                s = &((short *)wipe_scr_end)[i*height+y[i]];
                d = &((short *)wipe_scr)[y[i]*width+i];
                idx = 0;
                for (j=dy;j;j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                y[i] += dy;
                s = &((short *)wipe_scr_start)[i*height];
                d = &((short *)wipe_scr)[y[i]*width+i];
                idx = 0;
                for (j=height-y[i];j;j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                done = false;
            }
        }
    }

    return done;

}


int wipe_exitMelt ( int   width,
                    int   height,
                    int   ticks )
{
    Z_Free(y);
    return 0;
}


//  save the 'before' screen of the wipe (the one that melts down)
//
int wipe_StartScreen ( int   x,
                       int   y,
                       int   width,
                       int   height )
{
    wipe_scr_start = screens[2];
    I_ReadScreen(wipe_scr_start);  // copy vid.display in screen format
    return 0;
}


//  save the 'after' screen of the wipe (the one that show behind the melt)
//
int wipe_EndScreen ( int   x,
                     int   y,
                     int   width,
                     int   height )
{
    wipe_scr_end = screens[3];
    I_ReadScreen(wipe_scr_end);  // copy vid.display in screen format
    // restore start scr.
// old: V_DrawBlock(x, y, 0, width, height, wipe_scr_start); // restore start scr.
//    V_CopyRect(x, y, 2, width, height, x, y, 0);  // screen[2] -> screen[0]
//  full copy, ignore parameters
    VID_BlitLinearScreen(wipe_scr_start, screens[0], vid.width, vid.height, vid.ybytes, vid.ybytes);
    return 0;
}


int wipe_ScreenWipe ( int   wipeno,
                      int   x,
                      int   y,
                      int   width,
                      int   height,
                      int   ticks )
{
    int rc;
    static int (*wipes[])(int, int, int) =
    {
        wipe_initColorXForm, 
        wipe_doColorXForm, 
        wipe_exitColorXForm,
        wipe_initMelt, 
        wipe_doMelt, 
        wipe_exitMelt
    };

#ifdef DIRTY_RECT
    //Fab: obsolete (we don't use dirty-rectangles type of refresh)
    //void V_MarkRect(int, int, int, int);
#endif

    // initial stuff
    if (!go)
    {
        go = 1;
        // wipe_scr = (byte *) Z_Malloc(width*height*vid.bytepp, PU_STATIC, 0); // DEBUG
        wipe_scr = screens[0];
        (*wipes[wipeno*3])(width, height, ticks);
    }

    // do a piece of wipe-in
#ifdef DIRTY_RECT
    //V_MarkRect(0, 0, width, height);
#endif
    rc = (*wipes[wipeno*3+1])(width, height, ticks);
    //  V_DrawBlock(x, y, 0, width, height, wipe_scr); // DEBUG

    // final stuff
    if (rc)
    {
        go = 0;
        (*wipes[wipeno*3+2])(width, height, ticks);
    }

    return !go;

}
