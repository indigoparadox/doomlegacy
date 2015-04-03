// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: console.c,v $
// Revision 1.23  2003/08/11 13:50:03  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.22  2003/05/04 02:27:49  sburke
// Fix for big-endian machines.
//
// Revision 1.21  2002/09/10 19:29:46  hurdler
// Add log file under Linux
//
// Revision 1.20  2002/08/25 14:59:32  hurdler
//
// Revision 1.19  2002/07/23 15:07:09  mysterial
// Messages to second player appear on his half of the screen
//
// Revision 1.18  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.17  2001/08/20 20:40:39  metzgermeister
// Revision 1.16  2001/05/16 21:21:14  bpereira
//
// Revision 1.15  2001/03/03 19:41:22  ydario
// I_OutputMsg not implemented in OS/2
//
// Revision 1.14  2001/03/03 06:17:33  bpereira
// Revision 1.13  2001/02/24 13:35:19  bpereira
//
// Revision 1.12  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.11  2000/11/12 09:48:15  bpereira
//
// Revision 1.10  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/09/28 20:57:14  bpereira
// Revision 1.8  2000/08/31 14:30:55  bpereira
//
// Revision 1.7  2000/08/10 15:01:06  ydario
// OS/2 port
//
// Revision 1.6  2000/08/03 17:57:41  bpereira
//
// Revision 1.5  2000/04/24 15:10:56  hurdler
// Support colormap for text
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
//
// Revision 1.3  2000/04/07 23:09:12  metzgermeister
// fixed array boundary error
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      console for Doom LEGACY
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "console.h"
#include "g_game.h"
#include "g_input.h"
  // keys.h, gc_console
#include "hu_stuff.h"
#include "s_sound.h"
  // sounds.h, S_StartSound
#include "v_video.h"
#include "i_video.h"
#include "i_system.h"
  // I_OutputMessage
#include "z_zone.h"
#include "d_main.h"

//#include <unistd.h>

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

boolean  con_started=false;  // console has been initialised
boolean  con_video=false;  // text mode until video started
boolean  con_self_refresh=false;  // true at game startup, screen need refreshing
boolean  con_forcepic=true;  // at startup toggle console translucency when
                             // first off
boolean  con_recalc;     // set true when screen size has changed

int      con_tick;       // console ticker for anim or blinking prompt cursor
                         // con_scrollup should use time (currenttime - lasttime)..

boolean  consoletoggle;  // true when console key pushed, ticker will handle
boolean  console_ready;  // console prompt is ready
boolean  console_open = false;  // console is open

int      con_destlines;  // vid lines used by console at final position
int      con_curlines;   // vid lines currently used by console

int      con_clipviewtop;// clip value for planes & sprites, so that the
                         // part of the view covered by the console is not
                         // drawn when not needed, this must be -1 when
                         // console is off

// TODO: choose max hud msg lines
#define  CON_MAXHUDLINES      5

static int      con_hudlines;        // number of console heads up message lines
int      con_hudtime[5];      // remaining time of display for hud msg lines

int      con_clearlines; // top screen lines to refresh when view reduced
boolean  con_hudupdate;  // when messages scroll, we need a backgrnd refresh


// console text output
char*    con_line;       // console text output current line
int      con_cx;         // cursor position in current line
int      con_cy;         // cursor line number in con_buffer, is always
                         //  increasing, and wrapped around in the text
                         //  buffer using modulo.

int      con_totallines; // lines of console text into the console buffer
int      con_width;      // columns of chars, depend on vid mode width

int      con_scrollup;   // how many rows of text to scroll up (pgup/pgdn)

int      con_lineowner[CON_MAXHUDLINES]; //In splitscreen, which player gets this line of text
                                         //0 or 1 is player 1, 2 is player 2

#define  CON_PROMPTCHAR        '>'

// Hold last CON_MAX_LINEHIST prompt lines.
// [WDJ] Power 2 only, due to INDEXMASK.
#define  CON_MAX_LINEHIST    32
#define  CON_MAX_LINEHIST_INDEXMASK  (CON_MAX_LINEHIST-1)
#define  CON_MAX_LINELEN    256

// First char is prompt.
char     inputlines[CON_MAX_LINEHIST][CON_MAX_LINELEN];

int      inputline;      // current input line number
int      inputhist;      // line number of history input line to restore
int      input_cx;       // position in current input line

pic_t*   con_backpic;    // console background picture, loaded static
pic_t*   con_bordleft;
pic_t*   con_bordright;  // console borders in translucent mode


// protos.
static void CON_InputInit (void);
static void CON_Print (char *msg);
static void CONS_Clear_f (void);
static void CON_RecalcSize ( int width );

static void CONS_speed_Change (void);
static void CON_DrawBackpic (pic_t *pic, int startx, int destwidth);


//======================================================================
//                   CONSOLE VARS AND COMMANDS
//======================================================================
#if defined( MACOS_DI ) && ! defined( __GNUC__ )
#define  CON_BUFFERSIZE   4096  //my compiler cant handle local vars >32k
#else
#define  CON_BUFFERSIZE   16384
#endif

char     con_buffer[CON_BUFFERSIZE];


// how many seconds the hud messages lasts on the screen
consvar_t   cons_msgtimeout = {"con_hudtime","5",CV_SAVE,CV_Unsigned};

// number of lines console move per frame
consvar_t   cons_speed = {"con_speed","8",CV_CALL|CV_SAVE,CV_Unsigned,&CONS_speed_Change};

// percentage of screen height to use for console
consvar_t   cons_height = {"con_height","50",CV_SAVE,CV_Unsigned};

CV_PossibleValue_t backpic_cons_t[]={{0,"translucent"},{1,"picture"},{0,NULL}};
// whether to use console background picture, or translucent mode
consvar_t   cons_backpic = {"con_backpic","0",CV_SAVE,backpic_cons_t};


//  Check CONS_speed value (must be positive and >0)
//
static void CONS_speed_Change (void)
{
    if (cons_speed.value<1)
        CV_SetValue (&cons_speed,1);
}


//  Clear console text buffer
//
static void CONS_Clear_f (void)
{
    memset(con_buffer,0,CON_BUFFERSIZE);

    con_cx = 0;
    con_cy = con_totallines-1;
    con_line = &con_buffer[con_cy*con_width];
    con_scrollup = 0;
}

// Keys defined by the BIND command.
static char *bindtable[NUMINPUTS];

void CONS_Bind_f(void)
{
    int  key;
    COM_args_t  carg;
    
    COM_Args( &carg );

    if ( carg.num!=2 && carg.num!=3 )
    {
        int nb = 0;
        CONS_Printf ("bind <keyname> [<command>]\n");
        CONS_Printf("\2bind table :\n");
        for(key=0;key<NUMINPUTS;key++)
        {
            if(bindtable[key])
            {
                CONS_Printf("%s : \"%s\"\n",G_KeynumToString (key),bindtable[key]);
                nb=1;
            }
	}
        if(!nb)
            CONS_Printf("Empty\n");
        return;
    }

    key=G_KeyStringtoNum( carg.arg[1] );
    if(!key)
    {
        CONS_Printf("Invalid key name\n");
        return;
    }

    if(bindtable[key]!=NULL)
    {
        Z_Free(bindtable[key]);
        bindtable[key]=NULL;
    }

    if( carg.num==3 )
        bindtable[key]=Z_StrDup( carg.arg[2] );
}


//======================================================================
//                          CONSOLE SETUP
//======================================================================

// Prepare a colormap for GREEN ONLY translucency over background
//
byte*   whitemap;
byte*   greenmap;
byte*   graymap;

// May be called again after command_restart
static void CON_SetupBackColormap (void)
{
    int   i,j,k;
    byte* pal;

//
//  setup the green translucent background colormap
//
    if( ! whitemap )
    {
        //  setup the green translucent background colormap
        greenmap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
        whitemap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
        graymap  = (byte *) Z_Malloc(256,PU_STATIC,NULL);
    }

    // wad containing PLAYPAL may not be found yet.
    if( W_CheckNumForName( "PLAYPAL" ) < 0 )  return;
    pal = W_CacheLumpName ("PLAYPAL",PU_CACHE); // temp, only used next loop

    for(i=0,k=0; i<768; i+=3,k++)
    {
        j = pal[i] + pal[i+1] + pal[i+2];

        if( gamemode == heretic )
        {
            greenmap[k] = 209 + (float)j*15/(3*255);   //remaps to greens(209-224)
            graymap[k]  =       (float)j*35/(3*255);   //remaps to grays(0-35)           
            whitemap[k] = 145 + (float)j*15/(3*255);   //remaps to reds(145-168)
        }
        else
            greenmap[k] = 127 - (j>>6);
    }

//
//  setup the white and gray text colormap
//
    // this one doesn't need to be aligned, unless you convert the
    // V_DrawMappedPatch() into optimised asm.

    if( gamemode != heretic )
    {
        for(i=0; i<256; i++)
        {
            whitemap[i] = i;        //remap each color to itself...
            graymap[i]  = i;
        }

        for(i=168;i<192;i++)
        {
            whitemap[i]=i-88;     //remaps reds(168-192) to whites(80-104)
            graymap[i]=i-80;      //remaps reds(168-192) to gray(88-...)
        }
        whitemap[45]=190-88; // the color[45]=color[190] !
        graymap [45]=190-80;
        whitemap[47]=191-88; // the color[47]=color[191] !
        graymap [47]=191-80;
    }
}


//  Setup the console text buffer
//
// Init messaging, before zone memory, before video
// CON buffer will save all messages for display, so must be started very early
void CON_Init(void)
{
    int i;

    // clear all lines
    con_width = 0;  // no current text
    CON_RecalcSize ( INITIAL_WINDOW_WIDTH );  // before vid is set
    CONS_Clear_f ();   // clear all lines
    con_destlines = INITIAL_WINDOW_HEIGHT;
    con_curlines = INITIAL_WINDOW_HEIGHT;

    con_hudlines = CON_MAXHUDLINES;
    CON_ClearHUD ();

    // setup console input filtering
    CON_InputInit ();

    for(i=0;i<NUMINPUTS;i++)
        bindtable[i]=NULL;

    consoletoggle = false;
    con_started = true;
}

// after zone memory init
void CON_Register(void)
{
    // register our commands
    CV_RegisterVar (&cons_msgtimeout);
    CV_RegisterVar (&cons_speed);
    CV_RegisterVar (&cons_height);
    CV_RegisterVar (&cons_backpic);
    COM_AddCommand ("cls", CONS_Clear_f);
    COM_AddCommand ("bind", CONS_Bind_f);
}

// after FullGraphics
void CON_VideoInit(void)
{
    if(dedicated)
	return;
    
    // make sure it is ready for the loading screen
    CON_RecalcSize ( vid.width );

    CON_SetupBackColormap ();

    //note: CON_Ticker should always execute at least once before D_Display()
    con_clipviewtop = -1;     // -1 does not clip

    // load console background pic
    con_backpic = (pic_t*) W_CachePicName ("CONSBACK",PU_STATIC);

    // borders MUST be there
    con_bordleft  = (pic_t*) W_CachePicName ("CBLEFT",PU_STATIC);
    con_bordright = (pic_t*) W_CachePicName ("CBRIGHT",PU_STATIC);

    // set console full screen for game startup after FullGraphics
    con_destlines = vid.height;
    con_curlines = vid.height;

    con_self_refresh = true; // need explicit screen refresh
                        // until we are in Doomloop
    con_video = true;   // if move CON init to before video startup
}


//  Console input initialization
//
static void CON_InputInit (void)
{
    int    i;

    // prepare the first prompt line
    memset (inputlines,0,sizeof(inputlines));
    for (i=0; i<CON_MAX_LINEHIST; i++)
        inputlines[i][0] = CON_PROMPTCHAR;
    inputline = 0;
    input_cx = 1;
}



//======================================================================
//                        CONSOLE EXECUTION
//======================================================================


//  Called at screen size change to set the rows and line size of the
//  console text buffer.
//
static void CON_RecalcSize ( int width )
{
    int   min_conwidth = (BASEVIDWIDTH>>3)-2;  // minimum console width
    int   new_conwidth, oldcon_width, oldnumlines, oldcon_cy;
    int   i, conw;
    char  tmp_buffer[CON_BUFFERSIZE];
    char  string[CON_BUFFERSIZE]; // BP: it is a line but who know

    con_recalc = false;

    new_conwidth = (width>>3)-2;  // going to be the new con_width
    if ( new_conwidth < min_conwidth )
        new_conwidth = min_conwidth;

    // check for change of video width
    if (new_conwidth == con_width)
        return;                 // didnt change

    // save current
    oldcon_width = con_width;
    oldnumlines = con_totallines;
    oldcon_cy = con_cy;
    memcpy(tmp_buffer, con_buffer, CON_BUFFERSIZE);

    // setup to new width
    con_width = new_conwidth;
    con_totallines = CON_BUFFERSIZE / con_width;
    CONS_Clear_f ();

    // re-arrange console text buffer to keep text
    if(oldcon_width) // not the first time
    {
        for(i=oldcon_cy+1;i<oldcon_cy+oldnumlines;i++)
        {
            if( tmp_buffer[(i% oldnumlines)*oldcon_width])
            {
                memcpy(string, &tmp_buffer[(i% oldnumlines)*oldcon_width], oldcon_width);
                conw=oldcon_width-1;
                while(string[conw]==' ' && conw) conw--;
                string[conw+1]='\n';
                string[conw+2]='\0';
                CON_Print(string);
            }
        }
    }
}


// Handles Console moves in/out of screen (per frame)
//
static void CON_MoveConsole (void)
{
    // up/down move to dest
    if (con_curlines < con_destlines)
    {
        con_curlines+=cons_speed.value;
        if (con_curlines > con_destlines)
           con_curlines = con_destlines;
    }
    else if (con_curlines > con_destlines)
    {
        con_curlines-=cons_speed.value;
        if (con_curlines < con_destlines)
            con_curlines = con_destlines;
    }

}


//  Clear time of console heads up messages
//
void CON_ClearHUD (void)
{
    int    i;

    for(i=0; i<con_hudlines; i++)
        con_hudtime[i]=0;
}


// Force console to move out immediately
// note: con_ticker will set console_ready false
void CON_ToggleOff (void)
{
    if (!con_destlines)
        return;

    con_destlines = 0;
    con_curlines = 0;
    CON_ClearHUD ();
    con_forcepic = 0;
    con_clipviewtop = -1;       //remove console clipping of view
    console_open = false;  // instant off
}


//  Console ticker : handles console move in/out, cursor blinking
//
// Call once per tic.
void CON_Ticker (void)
{
    int    i;

    // cursor blinking
    con_tick++;
    con_tick &= 7;

    // console key was pushed
    if (consoletoggle)
    {
        consoletoggle = false;

        if (con_destlines > 0)
        {
	    // toggle off console
            con_destlines = 0;
            CON_ClearHUD ();
        }
        else
        {
            // toggle console in
            con_destlines = (cons_height.value*vid.height)/100;
            if (con_destlines < 20)
                con_destlines = 20;
            else
            if (con_destlines > vid.height-stbarheight)
                con_destlines = vid.height-stbarheight;

            con_destlines &= ~0x3;      // multiple of text row height
        }
    }

    // console movement
    if (con_destlines!=con_curlines)
        CON_MoveConsole ();  // update con_curlines


    // clip the view, so that the part under the console is not drawn
    con_clipviewtop = -1;
    if (cons_backpic.value)   // clip only when using an opaque background
    {
        if (con_curlines > 0)
            con_clipviewtop = con_curlines - viewwindowy - 1 - 10;
//NOTE: BIG HACK::SUBTRACT 10, SO THAT WATER DON'T COPY LINES OF THE CONSOLE
//      WINDOW!!! (draw some more lines behind the bottom of the console)
        if (con_clipviewtop<0)
            con_clipviewtop = -1;   //maybe not necessary, provided it's <0
    }

    // check if console ready for prompt
//    if ((con_curlines==con_destlines) && (con_destlines>=20))
    console_ready = (con_destlines >= 20);

    // To detect console.
    console_open = ((con_destlines + con_curlines) != 0);

    // make overlay messages disappear after a while
    for (i=0 ; i<con_hudlines; i++)
    {
        con_hudtime[i]--;
        if (con_hudtime[i]<0)
            con_hudtime[i]=0;
    }
}


//  Handles console key input
//
boolean CON_Responder(event_t *ev)
{
// sequential completions a la 4dos
static char    completion[80];
static int     comskips,varskips;

    char *cmd = NULL;

    // [WDJ]  The compiler re-optimizes the returns.  Collecting them
    // into common return true, and return false, has no net effect.
    //  
    // Return true: eat the key
    //        false: reject the key
    
    if(chat_on)
        return false; 

    // let go keyup events, don't eat them
    if (ev->type != ev_keydown)
        return false;

    int key = ev->data1;

    // Detect console activate key (user definable).
    if (key == gamecontrol[gc_console][0] ||
        key == gamecontrol[gc_console][1] )   goto toggle_console;

    if (! console_ready)
    {
        // Console prompt not active.  This is the path during game play.
        // Check game playing keys defined by BIND command.
	// metzgermeister: boundary check !!
        if((key < NUMINPUTS) && bindtable[key])
        {
            COM_BufAddText (bindtable[key]);
            COM_BufAddText ("\n");
            return true;
        }
        return false;
    }

    // Console prompt active
    // [WDJ] Trying to use a switch stmt, increases the size for unknown
    // reasons related to optimization.  It uses extra tests to gain speed.
    // It optimizes the repeated tests of the key better than the switch.

    // eat shift only if console active
    if (key == KEY_RSHIFT || key == KEY_LSHIFT)
      return true;

    // escape key toggle off console
    if (key == KEY_ESCAPE)   goto toggle_console;

    // command completion forward (tab) and backward (shift-tab)
    if (key == KEY_TAB)
    {
        // TOTALLY UTTERLY UGLY NIGHT CODING BY FAB!!! :-)
        //
        // sequential command completion forward and backward

        // remember typing for several completions (�-la-4dos)
        if (inputlines[inputline][input_cx-1] != ' ')
        {
            if (strlen (inputlines[inputline]+1)<80)
                strcpy (completion, inputlines[inputline]+1);
            else
                completion[0] = 0;

            comskips = varskips = 0;
        }
        else
        {
	    // comskips < 0  indicates var name completion
            if (shiftdown)
            {
                if (comskips<0)
                {
                    if (--varskips<0)
                        comskips = -(comskips+2);
                }
                else
                if (comskips>0)
                    comskips--;
            }
            else
            {
                if (comskips<0)
                    varskips++;
                else
                    comskips++;
            }
        }

        if (comskips>=0)
        {
            cmd = COM_CompleteCommand (completion, comskips);
            if (!cmd)
	    {
	        // No command, try var completion.
                // dirty:make sure if comskips is zero, to have a neg value
                comskips = -(comskips+1);
	    }
        }
        if (comskips<0)
            cmd = CV_CompleteVar (completion, varskips);

        if (cmd)
        {
            memset(inputlines[inputline]+1,0,CON_MAX_LINELEN-1);
            strcpy (inputlines[inputline]+1, cmd);
            input_cx = strlen(cmd)+1;
            inputlines[inputline][input_cx] = ' ';
            input_cx++;
            inputlines[inputline][input_cx] = 0;
        }
        else
        {
	    // No command, no var completion.  Backup off this candidate.
            if (comskips>0)
                comskips--;
            else
            if (varskips>0)
                varskips--;
        }

        return true;
    }

    // move up (backward) in console textbuffer
    if (key == KEY_PGUP)
    {
        if (con_scrollup < (con_totallines-((con_curlines-16)>>3)) )
            con_scrollup++;
        return true;
    }
    if (key == KEY_PGDN)
    {
        if (con_scrollup>0)
            con_scrollup--;
        return true;
    }

    // oldset text in buffer
    if (key == KEY_HOME)
    {
        con_scrollup = (con_totallines-((con_curlines-16)>>3));
        return true;
    }
    // most recent text in buffer
    if (key == KEY_END)
    {
        con_scrollup = 0;
        return true;
    }

    // command enter
    if (key == KEY_ENTER)
    {
        if (input_cx<2)
            return true;  // nothing significant

        // push the command
        COM_BufAddText (inputlines[inputline]+1);
        COM_BufAddText ("\n");

        CONS_Printf("%s\n",inputlines[inputline]);

        // Advance to next inputline.
        inputline = (inputline+1) & CON_MAX_LINEHIST_INDEXMASK;
        inputhist = inputline;

        memset(inputlines[inputline],0,CON_MAX_LINELEN);
        inputlines[inputline][0] = CON_PROMPTCHAR;
        input_cx = 1;

        return true;
    }

    // backspace command prompt
    if (key == KEY_BACKSPACE)
    {
        if (input_cx>1)  // back to prompt
        {
            input_cx--;
            inputlines[inputline][input_cx] = 0;
        }
        return true;
    }

    // move back in input history
    if (key == KEY_UPARROW)
    {
        // copy one of the previous inputlines to the current
        do{
            inputhist = (inputhist - 1) & CON_MAX_LINEHIST_INDEXMASK; // cycle back
        }while (inputhist!=inputline && !inputlines[inputhist][1]);

        // stop at the last history input line, which is the
        // current line + 1 because we cycle through the 32 input lines
        if (inputhist==inputline)
            inputhist = (inputline + 1) & CON_MAX_LINEHIST_INDEXMASK;

        memcpy (inputlines[inputline],inputlines[inputhist],CON_MAX_LINELEN);
        input_cx = strlen(inputlines[inputline]);

        return true;
    }

    // move forward in input history
    if (key == KEY_DOWNARROW)
    {
        if (inputhist==inputline) return true;

        do{
            inputhist = (inputhist + 1) & CON_MAX_LINEHIST_INDEXMASK;
        } while (inputhist!=inputline && !inputlines[inputhist][1]);

        memset (inputlines[inputline],0,CON_MAX_LINELEN);

        // back to currentline
        if (inputhist==inputline)
        {
            inputlines[inputline][0] = CON_PROMPTCHAR;
            input_cx = 1;
        }
        else
        {
            strcpy (inputlines[inputline],inputlines[inputhist]);
            input_cx = strlen(inputlines[inputline]);
        }
        return true;
    }

    // interpret it as input char
    char c = ev->data2;

    // allow people to use keypad in console (good for typing IP addresses) - Calum
    if (key >= KEY_KEYPAD0 && key <= KEY_PLUSPAD)
    {
      const char keypad_translation[] = {'0','1','2','3','4','5','6','7','8','9','.','/','*','-','+'};
      c = keypad_translation[key - KEY_KEYPAD0];
    }

    // enter a printable char into the command prompt
    if (c < ' ' || c > '~')
      return false;

    // add key to cmd line here
    if (input_cx<CON_MAX_LINELEN)
    {
        // make sure letters are lowercase for commands & cvars
        if (c >= 'A' && c <= 'Z')
            c = c + 'a' - 'A';

        inputlines[inputline][input_cx] = c;
        input_cx++;
        inputlines[inputline][input_cx] = 0;
    }

    return true;
 
toggle_console:
    consoletoggle = true;  // signal to CON_Ticker
    return true;
}


//  Insert a new line in the console text buffer
//
static void CON_Linefeed (int second_player_message)
{
    // set time for heads up messages
    con_hudtime[con_cy%con_hudlines] = cons_msgtimeout.value*TICRATE;

    if (second_player_message == 1)
        con_lineowner[con_cy%con_hudlines] = 2; //Msg for second player
    else
        con_lineowner[con_cy%con_hudlines] = 1;
        
    con_cy++;
    con_cx = 0;

    con_line = &con_buffer[(con_cy%con_totallines)*con_width];
    memset(con_line,' ',con_width);

    // make sure the view borders are refreshed if hud messages scroll
    con_hudupdate = true;         // see HU_Erase()
}


//  Outputs text into the console text buffer
//
//TODO: fix this mess!!
static void CON_Print (char *msg)
{
    int      l;
    int      mask=0;
    int      second_player_message=0;

    //TODO: finish text colors
    if (*msg<5)
    {
      if (*msg=='\2')  // set white color
          mask = 128;
      else if (*msg=='\3')
      {
          mask = 128;                         // white text + sound
          if ( gamemode == doom2_commercial )
              S_StartSound(0, sfx_radio);
          else
              S_StartSound(0, sfx_tink);
      }
      else if (*msg=='\4') //Splitscreen: This message is for the second player
          second_player_message = 1;
      
    }

    while (*msg)
    {
        // skip non-printable characters and white spaces
        while (*msg && *msg<=' ')
        {
            // carriage return
            if (*msg=='\r')
            {
                con_cy--;
                CON_Linefeed (second_player_message);
            }
            else
            // linefeed
            if (*msg=='\n')
                CON_Linefeed (second_player_message);
            else
            if (*msg==' ')
            {
                con_line[con_cx++] = ' ';
                if (con_cx>=con_width)
                    CON_Linefeed(second_player_message);
            }
            else if (*msg=='\t')
            {
                //adds tab spaces for nice layout in console
                do
                {
                    con_line[con_cx++] = ' ';
                } while (con_cx%4 != 0);
                
                if (con_cx>=con_width)
                    CON_Linefeed(second_player_message);
            }
            msg++;
        }

        if (*msg==0)
            return;

        // printable character
        for (l=0; l<con_width && msg[l]>' '; l++)
            ;

        // word wrap
        if (con_cx+l>con_width)
            CON_Linefeed (second_player_message);

        // a word at a time
        for ( ; l>0; l--)
            con_line[con_cx++] = *(msg++) | mask;

    }
}


//  Console print! Wahooo! Lots o fun!
//
#define CONS_BUF_SIZE 1024

// [WDJ] print from va_list
// Caller must have va_start, va_end
void CONS_Printf_va (const char *fmt, va_list ap)
{
    char  txt[CONS_BUF_SIZE];

    // print the error
    vsnprintf(txt, CONS_BUF_SIZE, fmt, ap);
    txt[CONS_BUF_SIZE-1] = '\0'; // term, when length limited

#ifdef LOGMESSAGES
    // echo console prints to log file
    if (logstream)
      fputs(txt, logstream);
#endif
    DEBFILE(txt);

    // Disable debug messages for release version
#ifndef  SHOW_DEBUG_MESSAGES
    if( EMSG_flags & EMSG_debtst )  goto done;  // disable debug messages
#endif

    if( EMSG_flags & (EMSG_text | EMSG_error) )
    {
        // Errors to terminal, and before graphics
        I_OutputMsg ("%s",txt);
        fflush(NULL);
    }

#if 0
#ifdef LINUX
    // Keep debug messages off console, for some versions
    if( EMSG_flags & EMSG_debtst )  goto done;
#endif
#endif

    if( ! con_started )  goto done;
    if( EMSG_flags & EMSG_error )
    {
#ifndef LAUNCHER
        // Blocks error to Launcher fatal error display
        // errors to CON unless no con_video yet
        if( ! con_video )  goto done;
#endif
    }
    else if( ! (EMSG_flags & EMSG_CONS) )  goto done;  // no CONS flag
   
    // print to EMSG_CONS
    // write message in con text buffer
    CON_Print (txt);

    // make sure new text is visible
    con_scrollup = 0;

    // if not in display loop, force screen update
    if ( con_self_refresh || (EMSG_flags & EMSG_now) )
    {
        if( ! graphics_started )   goto done;
        // have graphics, but do not have refresh loop running
#if defined(WIN_NATIVE) || defined(OS2_NATIVE) 
        // show startup screen and message using only 'software' graphics
        // (rendermode may be hardware accelerated, but the video mode is not set yet)
        CON_DrawBackpic (con_backpic, 0, vid.width);    // put console background
        I_LoadingScreen ( txt );
#else
        V_ClearDisplay();
        // here we display the console background and console text
        // (no hardware accelerated support for these versions)
        CON_Drawer ();
        I_FinishUpdate ();              // page flip or blit buffer
#endif
    }
    else if ( ! con_video )
    {
        if( ! graphics_started || ! vid.display )   goto done;
        // messages before graphics
        CON_DrawConsole ();
        I_FinishUpdate ();
    }
 done:
    return;
}

// General printf interface for CONS_Printf
void CONS_Printf (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    CONS_Printf_va( fmt, ap );
    va_end(ap);
}

//  Print an error message, and wait for ENTER key to continue.
//  To make sure the user has seen the message
//
void CONS_Error (char *msg)
{
    byte save_emsg_flags = EMSG_flags;
    EMSG_flags |= EMSG_error;

#ifdef WIN_NATIVE
    if(!graphics_started)
    {
        I_MsgBox (msg);
        return;
    }
#endif
    CONS_Printf ("\2%s",msg);   // write error msg in different colour

    // CONS_Printf ("Press ENTER to continue\n");
    // dirty quick hack, but for the good cause
    // while (I_GetKey() != KEY_ENTER)
    //   ;
    EMSG_flags = save_emsg_flags;
}


// For info, debug, dev, verbose messages
// print to text, console, and logs
void GenPrintf (byte emsgflags, const char *fmt, ...)
{
    va_list ap;
    byte save_emsg_flags = EMSG_flags;  // emsgflags are temporary
    EMSG_flags = (EMSG_flags & EMSG_text) | emsgflags;
    va_start(ap, fmt);
    CONS_Printf_va( fmt, ap );  // print to text, console, and logs
    va_end(ap);
    EMSG_flags = save_emsg_flags;
}


//======================================================================
//                          CONSOLE DRAW
//======================================================================


// draw console prompt line
//
static void CON_DrawInput (void)
{
    char    *p;
    int     x,y;

    // Draw console text, screen0
    // V_SetupDraw( 0 | V_NOSCALEPATCH | V_NOSCALESTART );

    // input line scrolls left if it gets too long
    //
    p = inputlines[inputline];
    if (input_cx>=con_width)
        p += input_cx - con_width + 1;

    y = con_curlines - 12;

    for(x=0; x<con_width; x++)
        V_DrawCharacter( (x+1)<<3, y, p[x] );  // red

    // draw the blinking cursor
    //
    x = (input_cx>=con_width) ? con_width - 1 : input_cx;
    if (con_tick<4)
        V_DrawCharacter( (x+1)<<3, y, 0x80 | '_' );  // white
}


// draw the last lines of console text to the top of the screen
//
#ifdef HWRENDER //Added by Mysterial
    extern float gr_viewheight; //needed for drawing second player's messages
                                //halfway down
#endif

static void CON_DrawHudlines (void)
{
    char       *p;
    int        i,x,y,y2;

    if (con_hudlines<=0)
        return;

    V_SetupDraw( 0 | V_NOSCALEPATCH | V_NOSCALESTART );

    if (chat_on)
        y = 8;   // leave place for chat input in the first row of text
    else
        y = 0;
    y2 = 0; //player 2's message y in splitscreen

    for (i= con_cy-con_hudlines+1; i<=con_cy; i++)
    {
        if (i < 0)
            continue;
        if (con_hudtime[i%con_hudlines] == 0)
            continue;

        p = &con_buffer[(i%con_totallines)*con_width];

        for (x=0; x<con_width; x++)
        {
#ifdef HWRENDER //Added by Mysterial
            if (con_lineowner[i%con_hudlines] == 2)
                V_DrawCharacter ( x<<3, y2+gr_viewheight, (p[x]&0xff) );  // red
            else
#endif
                V_DrawCharacter ( x<<3, y, (p[x]&0xff) );  // red
        }

        if (con_lineowner[i%con_hudlines] == 2)
           y2 += 8;
        else
           y += 8;
    }

    // top screen lines that might need clearing when view is reduced
    con_clearlines = y;      // this is handled by HU_Erase ();
}


//  Scale a pic_t at 'startx' pos, to 'destwidth' columns.
//                startx,destwidth is resolution dependent
//  Used to draw console borders, console background.
//  The pic must be sized BASEVIDHEIGHT height.
//
//  TODO: ASM routine!!! lazy Fab!!
//
static void CON_DrawBackpic (pic_t *pic, int startx, int destwidth)
{
    int   pic_h = pic->height;
    int   pic_w = pic->width;
    int   x, y;
    int   v;
    fixed_t  frac, fracstep;
    byte  *src;
    byte  *dest;  // within screen buffer
   
    // [WDJ] Draw picture for all bpp, bytepp, and padded lines.
    dest = V_GetDrawAddr( startx, 0 );  // screen0 buffer

    for (y=0 ; y<con_curlines ; y++, dest += vid.ybytes)
    {
        // scale the picture to the resolution
        v = pic_h - ((con_curlines - y)*(BASEVIDHEIGHT-1)/vid.height) - 1;

        src = pic->data + v*pic_w;

        // in case of the console backpic, simplify
        if (pic_w == destwidth && vid.bytepp == 1)
            memcpy (dest, src, destwidth);
        else
        {
            // scale pic to screen width
            frac = 0;
            fracstep = (pic_w<<16)/destwidth;
            for (x=0 ; x<destwidth ; x+=4)
            {
                V_DrawPixel( dest, x, src[frac>>16] );
                frac += fracstep;
                V_DrawPixel( dest, x+1, src[frac>>16] );
                frac += fracstep;
                V_DrawPixel( dest, x+2, src[frac>>16] );
                frac += fracstep;
                V_DrawPixel( dest, x+3, src[frac>>16] );
                frac += fracstep;
            }
        }
    }

}


// Draw the console background, text, and prompt if enough places.
// May use font1 or wad fonts.
//
void CON_DrawConsole (void)
{
    char  *p;
    int   i,x,y;
    int   w = 0, x2 = 0;
    fontinfo_t * fip = V_FontInfo();  // draw font1 and wad font strings

    if (con_curlines <= 0)
        return;

    if ( rendermode != render_soft && use_font1 )
        return;  // opengl graphics without hu_font loaded yet

    V_SetupDraw( 0 | V_NOSCALEPATCH | V_NOSCALESTART );

    //FIXME: refresh borders only when console bg is translucent
    con_clearlines = con_curlines;    // clear console draw from view borders
    con_hudupdate = true;             // always refresh while console is on

    // draw console background
    if (!con_video)
    {
        V_ClearDisplay();
    }
    else
    if (cons_backpic.value || con_forcepic)
    {
#ifdef HWRENDER // not win32 only 19990829 by Kin
        if (rendermode!=render_soft)
            V_DrawScalePic_Num (0, con_curlines-200*vid.fdupy,
				W_GetNumForName ("CONSBACK") );
        else
#endif
            CON_DrawBackpic (con_backpic,0,vid.width);   // picture as background
    }
    else
    {
#ifdef HWRENDER // not win32 only 19990829 by Kin
        if (rendermode==render_soft)
#endif
        {
            w = fip->xinc * vid.dupx;  // font1 or wad font
            x2 = vid.width - w;
            CON_DrawBackpic (con_bordleft,0,w);
            CON_DrawBackpic (con_bordright,x2,w);
        }
        //Hurdler: what's the correct value of w and x2 in hardware mode ???
        V_DrawFadeConsBack (w,0,x2,con_curlines);     // translucent background
    }

    // draw console text lines from bottom to top
    // (going backward in console buffer text)
    //
    if (con_curlines <20)       //8+8+4
        return;

    i = con_cy - con_scrollup;

    // skip the last empty line due to the cursor being at the start
    // of a new line
    if (!con_scrollup && !con_cx)
        i--;

    // draw lines with font1 or wad font
    for (y=con_curlines-20; y>=0; y-=fip->yinc,i--)
    {
        if (i<0)
            i=0;

        p = &con_buffer[(i%con_totallines)*con_width];

        for (x=0;x<con_width;x++)
            V_DrawCharacter( (x+1)*fip->xinc, y, p[x] );  // red
    }


    // draw prompt if enough place (not while game startup)
    //
    if ((con_curlines==con_destlines) && (con_curlines>=20) && !con_self_refresh)
        CON_DrawInput ();
}


//  Console refresh drawer, call each frame
//
void CON_Drawer (void)
{
    if (!con_started)
        return;

    if (con_recalc)
        CON_RecalcSize ( vid.width );
   
    if ( use_font1 )
        return;  // hu_font not loaded yet

    //Fab: bighack: patch 'I' letter leftoffset so it centers
    hu_font['I'-HU_FONTSTART]->leftoffset = -2;

    if (con_curlines>0)
        CON_DrawConsole ();
    else
    if (gamestate==GS_LEVEL)
        CON_DrawHudlines ();

    hu_font['I'-HU_FONTSTART]->leftoffset = 0;
}
