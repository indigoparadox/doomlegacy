// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: m_menu.c,v $
// Revision 1.55  2005/12/20 14:58:25  darkwolf95
// Monster behavior CVAR - Affects how monsters react when they shoot each other
//
// Revision 1.54  2004/09/12 19:40:06  darkwolf95
// additional chex quest 1 support
//
// Revision 1.53  2004/07/27 08:19:36  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.52  2003/08/11 13:50:01  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.51  2003/03/22 22:35:59  hurdler
//
// Revision 1.50  2002/09/27 16:40:08  tonyd
// First commit of acbot
//
// Revision 1.49  2002/09/12 20:10:50  hurdler
// Added some cvars
//
// Revision 1.48  2002/08/24 22:42:03  hurdler
// Apply Robert Hogberg patches
//
// Revision 1.47  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.46  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.45  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.44  2001/11/02 23:29:13  judgecutor
// Fixed "secondary player controls" bug
//
// Revision 1.43  2001/11/02 21:44:05  judgecutor
// Added Frag's weapon falling
//
// Revision 1.42  2001/08/20 21:37:34  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.40  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.39  2001/06/10 21:16:01  bpereira
// Revision 1.38  2001/05/27 13:42:47  bpereira
// Revision 1.37  2001/05/16 22:00:10  hurdler
// Revision 1.36  2001/05/16 21:21:14  bpereira
//
// Revision 1.35  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.34  2001/04/29 14:25:26  hurdler
// Revision 1.33  2001/04/01 17:35:06  bpereira
// Revision 1.32  2001/03/03 06:17:33  bpereira
// Revision 1.31  2001/02/24 13:35:20  bpereira
// Revision 1.30  2001/02/10 12:27:14  bpereira
//
// Revision 1.29  2001/01/25 22:15:42  bpereira
// added heretic support
//
// Revision 1.28  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.27  2000/10/21 08:43:29  bpereira
//
// Revision 1.26  2000/10/17 10:09:27  hurdler
// Update master server code for easy connect from menu
//
// Revision 1.25  2000/10/16 20:02:29  bpereira
// Revision 1.24  2000/10/08 13:30:01  bpereira
// Revision 1.23  2000/10/02 18:25:45  bpereira
//
// Revision 1.22  2000/10/01 15:20:23  hurdler
// Add private server
//
// Revision 1.21  2000/10/01 10:18:17  bpereira
//
// Revision 1.20  2000/10/01 09:09:36  hurdler
// Put the md2 code in #ifdef TANDL
//
// Revision 1.19  2000/09/15 19:49:22  bpereira
//
// Revision 1.18  2000/09/08 22:28:30  hurdler
// merge masterserver_ip/port in one cvar, add -private
//
// Revision 1.17  2000/09/02 15:38:24  hurdler
// Add master server to menus (temporaray)
//
// Revision 1.16  2000/08/31 14:30:55  bpereira
//
// Revision 1.15  2000/04/24 15:10:56  hurdler
// Support colormap for text
//
// Revision 1.14  2000/04/23 00:29:28  hurdler
// fix a small bug in skin color
//
// Revision 1.13  2000/04/23 00:25:20  hurdler
// fix a small bug in skin color
//
// Revision 1.12  2000/04/22 21:12:15  hurdler
//
// Revision 1.11  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.10  2000/04/16 18:38:07  bpereira
// Revision 1.9  2000/04/13 16:26:41  hurdler
//
// Revision 1.8  2000/04/12 19:31:37  metzgermeister
// added use_mouse to menu
//
// Revision 1.7  2000/04/08 17:29:24  stroggonmeth
//
// Revision 1.6  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.5  2000/04/04 10:44:00  hurdler
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
// NOTE:
//      Drawing is through V_SetupDraw() and V_DrawScaledPatch()
//      so that the menu is scaled to the screen size. The scaling is always
//      an integer multiple of the original size, so that the graphics look
//      good.
//
//-----------------------------------------------------------------------------

#include <unistd.h>
#include <fcntl.h>

#include "doomincl.h"
#include "am_map.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"
#include "p_chex.h"
  // Chex_safe_pictures

#include "p_saveg.h"
  // savegame header read

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "d_net.h"
#include "d_clisrv.h"
  // server1, server2, server3
#include "mserv.h"
#include "p_inter.h"


boolean                 menuactive;

// [WDJ] mouse sensitivity, 30 for old feel, 40..50 to reduce
#define MENU_MOUSE_TRIG   40

#define SKULLXOFF       -32
#define LINEHEIGHT       16
#define STRINGHEIGHT     10
#define FONTBHEIGHT      20
#define SMALLLINEHEIGHT   8
#define SLIDER_RANGE     10
#define SLIDER_WIDTH    (8*SLIDER_RANGE+6)
#define MAXSTRINGLENGTH  32

// [WDJ] Definition of slot.
// SLOT is the number attached to the savegame name.
// MSLOT is the number of MENU_SLOT.
// The savegamedisp table is indexed by slotindex [0..5],
// which for 99 savegames and with directories (slotindex=[1..6]),
// is different than the slot number [0..99].
// Entry savegamedisp[6] is reserved for quickSave.

// Save and Load menus must match the SAVEGAME_NUM_SLOT
#define SAVEGAME_NUM_MSLOT  6

// Map and Time on left side, otherwise on the right side
#define SAVEGAME_MTLEFT

// [WDJ] Fonts are variable width, so it does not actually overlap, most times.
#define SAVEGAME_MTLEN  12

#ifdef SAVEGAME_MTLEFT
// position in the line of map name and time
#define SAVE_DESC_POS   12
#define SAVE_DESC_XPOS  (SAVE_DESC_POS*8)
#define SAVELINELEN (SAVE_DESC_POS+SAVESTRINGSIZE-2)
#else
// position in the line of map name and time
#define SAVE_MT_POS     22
#define SAVELINELEN (SAVE_MT_POS+SAVEGAME_MTLEN)
#endif

#ifdef SAVEGAMEDIR
char  savegamedir[SAVESTRINGSIZE] = "";  // default is main legacy dir
#endif

typedef struct
{
#ifdef SAVEGAME99
    byte  savegameid;	// 0..99, else invalid
#endif
    char  desc[SAVESTRINGSIZE];
    char  levtime[SAVEGAME_MTLEN];
} savegame_disp_t;

// disp slots 0..5, and an extra 6=quicksave
static savegame_disp_t    savegamedisp[SAVEGAME_NUM_MSLOT+1];

static int   slotindex;  // MSLOT, used for reading and editing slot desc, 0..5
// quicksave_slotid: -1 = no slot picked!, -2 = select slot now, else is slot
static int   quicksave_slotid; // -1,-2, 0..5 or 0..99 (savegameid)

#if defined SAVEGAMEDIR || defined SAVEGAME99
#define     QUICKSAVE_INDEX   SAVEGAME_NUM_MSLOT
static int     scroll_index = 0;  // scroll position
static void    (*scroll_callback)(int amount) = NULL; // call to scroll
static void    (*delete_callback)(int ch) = NULL; // call to delete
#else
// otherwise the slot and index are the same
#define     QUICKSAVE_INDEX   quicksave_slotid
#endif


// menu string edit, used for entering a savegame string
static boolean edit_enable;
static int     edit_index;  // which char we're editing
// menu edit
static char    edit_buffer[SAVESTRINGSIZE];
static void    (*edit_done_callback)(void) = NULL;  // call upon edit done

// clear the savegamedisp from mslot to SAVEGAME_NUM_MSLOT
void clear_remaining_savegamedisp( int sgslot )
{
    // fill out as empty any remaining menu positions
    while( sgslot < SAVEGAME_NUM_MSLOT )  // do not overrun quicksave
    {
        savegamedisp[sgslot].desc[0] = '\0';
        savegamedisp[sgslot].levtime[0] = '\0';
#ifdef SAVEGAME99
        savegamedisp[sgslot].savegameid = 255;   // invalid
#endif
        sgslot ++;
    }
}

void setup_net_savegame( void )
{
    strcpy( savegamedir, "net" );	// default for network play
}

// flags for items in the menu
typedef enum {
// menu handle (what we do when key is pressed)
  IT_TYPE =  0x000E,   // field
// TYPE field values
  IT_SPACE =      0,   // no handling
  IT_CALL =       2,   // call the function
  IT_ARROWS =     4,   // call function with 0 for left arrow and 1 for right arrow in param
  IT_KEYHANDLER = 6,   // call with the key in param
  IT_SUBMENU =    8,   // go to sub menu
  IT_CVAR =      10,   // handle as a cvar
  IT_MSGHANDLER =12,   // same as key but with event and sometime can handle y/n key (special for message

  IT_DISPLAY =     0x00F0,  // field
// DISPLAY field values
  IT_NOTHING =          0,  // space
  IT_PATCH =       0x0010,  // a patch or a string with big font
  IT_STRING =      0x0020,  // little string (spaced with 10)
  IT_WHITESTRING = 0x0030,  // little string in white
  IT_DYBIGSPACE =  0x0040,  // same as nothing
  IT_DYLITLSPACE = 0x0050,  // little space
  IT_STRING2 =     0x0060,  // a simple string
  IT_GRAYPATCH =   0x0070,  // grayed patch or big font string
  IT_BIGSLIDER =   0x0080,  // volume sound use this

//consvar specific
  IT_CVARTYPE =	  0x0700,   // field
// CVARTYPE values
  IT_CV_NORMAL =       0,
  IT_CV_SLIDER =  0x0100,
  IT_CV_STRING =  0x0200,
  IT_CV_NOPRINT = 0x0300,
  IT_CV_NOMOD =   0x0400,

  IT_OPTION  = 0x3000,    // field
  IT_YOFFSET = 0x1000,    // alphaKey is offset
  IT_KEYID   = 0x2000,    // alphaKey is id

// in short for some common use
  IT_BIGSPACE =   (IT_SPACE  | IT_DYBIGSPACE),
  IT_LITLSPACE =  (IT_SPACE  | IT_DYLITLSPACE),
  IT_CONTROL =    (IT_STRING2| IT_CALL | IT_KEYID),
  IT_CVARMAX =    (IT_CVAR   | IT_CV_NOMOD),
  IT_DISABLED =   (IT_SPACE  | IT_GRAYPATCH),
} menu_control_e;



typedef void (*menufunc_t)(int choice);

static char input_char; // [smite] dirty hack, contains a second parameter to IT_KEYHANDLER functions (int choice is the key)
static inline boolean is_printable(char c) { return c >= ' ' && c <= '~'; }

typedef union
{
    struct menu_s     *submenu;               // IT_SUBMENU
    consvar_t         *cvar;                  // IT_CVAR
    menufunc_t         routine;  // IT_CALL, IT_KEYHANDLER, IT_ARROWS
} itemaction_t;

//
// MENU TYPEDEFS
//
typedef struct menuitem_s
{
    // show IT_xxx
    uint16_t  status;

    char      *patch;
    char      *text;  // used when FONTBxx lump is found

    // FIXME: should be itemaction_t !!!
    void      *itemaction;

    // hotkey in menu
    // or y of the item when IT_YOFFSET (uses M_DrawGenericMenu)
    // or in control menus, the control to change (uses M_DrawControl)
    byte      alphaKey;
} menuitem_t;

typedef struct menu_s
{
    char            *menutitlepic;
    char            *menutitle;             // title as string for display with fontb if present
    short           numitems;               // # of menu items
    struct menu_s*  prevMenu;               // previous menu
    menuitem_t*     menuitems;              // menu items
    void            (*drawroutine)(void);   // draw routine
    short           x;
    short           y;                      // x,y of menu
    short           lastOn;                 // last item user was on in menu
    boolean         (*quitroutine)(void);   // called before quit a menu return true if we can
} menu_t;

// current menudef
menu_t*   currentMenu = NULL;
short     itemOn;                       // menu item skull is on
short     skullAnimCounter;             // skull animation counter
short     whichSkull;                   // which skull to draw
int       SkullBaseLump;

// graphic name of skulls
char      skullName[2][9] = {"M_SKULL1","M_SKULL2"};

// [WDJ] menu sounds
static short menu_sfx_updown = sfx_menuud;
static short menu_sfx_val = sfx_menuva;
static short menu_sfx_enter = sfx_menuen;
static short menu_sfx_open = sfx_menuop;
static short menu_sfx_esc = sfx_menuop;
static short menu_sfx_action = sfx_menuac;

CV_PossibleValue_t menusound_cons_t[] =
  {{0,"Auto"}, {1,"Legacy"}, {2,"Doom"}, {3,"Heretic"}, {0,NULL}};
void CV_menusound_OnChange(void);
consvar_t cv_menusound = {"menusound", "1", CV_SAVE | CV_CALL, menusound_cons_t, CV_menusound_OnChange };

void CV_menusound_OnChange(void)
{
    int menusound = cv_menusound.value;

    switch ( gamemode )
    {
      case doom2_commercial:
      case doom_shareware:
      case doom_registered:
      case ultdoom_retail:
        if ( menusound == 0 || menusound == 3 )
           menusound = 2;
        break;
      case heretic:
        if ( menusound == 0 || menusound == 2 )
           menusound = 3;
        break;
      case chexquest1:
        if ( menusound == 0 || menusound == 3 )
           menusound = 1;
        break;
      default:
        break;
    }
    switch ( menusound )
    {
      default:
      case 0: // auto
      case 1: // Legacy
        menu_sfx_updown = sfx_menuud;
        menu_sfx_val = sfx_menuva;
        menu_sfx_enter = sfx_menuen;
        menu_sfx_open = sfx_menuop;
        menu_sfx_esc = sfx_menuop;
        menu_sfx_action = sfx_menuac;
        break;
      case 2: // Doom
        //Boom
        // help, save, load, volume menu open = sfx_swtchn
        // backspace = sfx_swtchn
        // menu action = sfx_itemup
        // next menu = sfx_swtchx
        menu_sfx_updown = sfx_pstop;
        menu_sfx_val = sfx_stnmov;
        menu_sfx_enter = sfx_pistol;
        menu_sfx_open = sfx_swtchn;
        menu_sfx_esc = sfx_swtchx;
        menu_sfx_action = sfx_swtchx;
        break;
      case 3: // Heretic
        //heretic
        // quit, chat val = sfx_chat;
        // chat keys = sfx_keyup;
        // info, save, load, volume menu open  = sfx_dorcls;
        // enter, activate menu, deactivate menu = sfx_dorcls;
        // escape = none;
        // backspace = sfx_switch;
        menu_sfx_updown = sfx_swtchx;  // sfx_switch
        menu_sfx_val = sfx_keyup;
        menu_sfx_enter = sfx_dorcls;
        menu_sfx_open = sfx_dorcls;
        menu_sfx_esc = sfx_menuva;  // none
//        menu_sfx_action = sfx_chat;  // don't have sfx_chat
        menu_sfx_action = sfx_menuac;
        break;
    } 
}


//
// PROTOTYPES
//
void M_DrawSaveLoadBorder(int x, int y, boolean longer);
void M_SetupNextMenu(menu_t *menudef);
void M_Setup_prevMenu( void );

void M_DrawTextBox (int x, int y, int width, int lines);     //added:06-02-98:
void M_DrawThermo(int x,int y,consvar_t *cv);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_DrawSlider (int x, int y, int range);
void M_CentreText(int y, char* string);        //added:30-01-98:writetext centered

void M_StartControlPanel(void);
void M_StopMessage(int choice);
void M_ClearMenus (boolean callexitmenufunc);
int  M_StringHeight(char* string);
void M_GameOption(int choice);
void M_AdvOption(int choice);
void M_NetOption(int choice);
//28/08/99: added by Hurdler
void M_OpenGLOption(int choice);

menu_t MainDef,SingleMultiDef,TwoPlayerDef,MultiPlayerDef,SetupMultiPlayerDef,
       EpiDef,NewDef,OptionsDef,VidModeDef,ControlDef,SoundDef,
       ReadDef2,ReadDef1,SaveDef,LoadDef,ControlDef2,GameOptionDef,
       AdvOptionsDef,EffectsOptionsDef,VideoOptionsDef,MouseOptionsDef,
       NetOptionDef,ConnectOptionDef,ServerOptionsDef,MPOptionDef;


//===========================================================================
//Generic Stuffs (more easy to create menus :))
//===========================================================================

void M_DrawMenuTitle(void)
{
    if( FontBBaseLump && currentMenu->menutitle )
    {
        int xtitle = (BASEVIDWIDTH-V_TextBWidth(currentMenu->menutitle))/2;
        int ytitle = (currentMenu->y-V_TextBHeight(currentMenu->menutitle))/2;
        if(xtitle<0) xtitle=0;
        if(ytitle<0) ytitle=0;

        V_DrawTextB(currentMenu->menutitle, xtitle, ytitle);
    }
    else
    if( currentMenu->menutitlepic )
    {
        patch_t* tp = W_CachePatchName(currentMenu->menutitlepic,PU_CACHE);  // endian fix
#if 1
        //SoM: 4/7/2000: Old code was causing problems with large graphics.
//        int xtitle = (vid.width / 2) - (p->width / 2);
//        int ytitle = (y-p->height)/2;
        int xtitle = 94;
        int ytitle = 2;
#else
        int xtitle = (BASEVIDWIDTH - tp->width)/2;
        int ytitle = (currentMenu->y - tp->height)/2;
#endif

        if(xtitle<0) xtitle=0;
        if(ytitle<0) ytitle=0;
        V_DrawScaledPatch (xtitle, ytitle, tp);
    }
}



void M_DrawGenericMenu(void)
{
    int x, y, i, w;
    int cursory=0;
    fontinfo_t * fip = V_FontInfo();
    menuitem_t * mip;

    // DRAW MENU
    // Draw to screen0, scaled
    x = currentMenu->x;
    y = currentMenu->y;

    // draw title (or big pic)
    M_DrawMenuTitle();

    for (i=0; i<currentMenu->numitems; i++)
    {
        mip = & currentMenu->menuitems[i];
        // handle Y offsets independent of IT_STRING and IT_WHITESTRING
        if( ((mip->status & IT_OPTION) == IT_YOFFSET) && mip->alphaKey )
        {
            y = currentMenu->y + mip->alphaKey;
        }
        if (i==itemOn)
            cursory=y;

        switch (mip->status & IT_DISPLAY) {
           case IT_PATCH  :
               if( FontBBaseLump && mip->text )
               {
                   V_DrawTextB(mip->text, x, y);
                   y += FONTBHEIGHT-LINEHEIGHT;
               }
               else 
               if( mip->patch && mip->patch[0] )
               {
                   V_DrawScaledPatch_Name (x,y, mip->patch );
               }
           case IT_NOTHING:
           case IT_DYBIGSPACE:
               y += LINEHEIGHT;
               break;
           case IT_BIGSLIDER :
               M_DrawThermo( x, y, (consvar_t *)mip->itemaction);
               y += LINEHEIGHT;
               break;
           case IT_STRING :
           case IT_WHITESTRING :
               if( (mip->status & IT_DISPLAY)==IT_STRING ) 
                   V_DrawString(x,y,0,mip->text);
               else
                   V_DrawString(x,y,V_WHITEMAP,mip->text);

               // Cvar specific handling
               switch (mip->status & IT_TYPE)
                   case IT_CVAR:
                   {
                    consvar_t *cv=(consvar_t *)mip->itemaction;
                    switch (mip->status & IT_CVARTYPE)
                    {
                       case IT_CV_SLIDER :
                           M_DrawSlider (BASEVIDWIDTH-x-SLIDER_WIDTH,
                                         y,
                                         ( (cv->value - cv->PossibleValue[0].value) * 100 /
                                         (cv->PossibleValue[1].value - cv->PossibleValue[0].value)));
                       case IT_CV_NOPRINT: // color use this 
                           break;
                       case IT_CV_STRING:
                           w = V_StringWidth(cv->string);
                           if( use_font1 )
                           {
                               char * sp = cv->string;
                               // Setup is centered, but this needs left justify.
                               M_DrawTextBox(-BASEVIDWIDTH/2,y+12,BASEVIDWIDTH/7,1);
                               while( *sp && w > BASEVIDWIDTH - 8 )
                               {
                                   w -= fip->xinc;
                                   sp++;
                               }
                               V_DrawString (x+8,y+12,0, sp);
//                             if( skullAnimCounter<4 && i==itemOn )
                               if( i==itemOn )
                                  V_DrawCharacter( x+8+w, y+12,  '_' | 0x80);  // white
                           }
                           else
                           {
                               M_DrawTextBox(x,y+4,MAXSTRINGLENGTH,1);
                               V_DrawString (x+8,y+12,0,cv->string);
                               if( skullAnimCounter<4 && i==itemOn )
                                  V_DrawCharacter( x+8+w, y+12,  '_' | 0x80);  // white
                           }
                           y+=16;
                           break;
                       default:
                           if( ! cv->string )
                           {
                               I_SoftError("GenMenu: cv_var NULL string %s\n", cv->name );
                               break;
                           }
                           V_DrawString(BASEVIDWIDTH-x-V_StringWidth (cv->string),
                                        y, V_WHITEMAP, 
                                        cv->string);
                           break;
                   }
                   break;
               }
               y+=STRINGHEIGHT;
               break;
           case IT_STRING2:
               V_DrawString (x,y,0,mip->text);
           case IT_DYLITLSPACE:
               y+=SMALLLINEHEIGHT;
               break;
           case IT_GRAYPATCH:
               if( FontBBaseLump && mip->text )
               {
                   V_DrawTextBGray(mip->text, x, y);
                   y += FONTBHEIGHT-LINEHEIGHT;
               }
               else 
               if( mip->patch &&
                   mip->patch[0] )
               {
                   V_DrawMappedPatch_Name (x,y, mip->patch, graymap );
               }
               y += LINEHEIGHT;
               break;

        }
    }

    // DRAW THE SKULL CURSOR
    if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_PATCH)
      ||((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_NOTHING) )
    {
        V_DrawScaledPatch_Name(currentMenu->x + SKULLXOFF, cursory-5,
                          skullName[whichSkull] );
    }
    else
    if (skullAnimCounter<4 * NEWTICRATERATIO)  //blink cursor
    {
        V_DrawCharacter(currentMenu->x - 10, cursory,
                        '*' | 0x80);  // white
    }

}

//===========================================================================
// All ready playing, quit current game
//===========================================================================

// [WDJ] message temp buffer (replacing 3 shorter ones)
// StartMessage copies this, and only one possible message at a time
#define      MSGTMP_LEN  255
static char  msgtmp[MSGTMP_LEN+1];

//const char *ALLREADYPLAYING="You are already playing\n\nLeave this game first\n";
const char * ALLREADYPLAYING="You are already playing.\n\nAbort this game ? Y/N\n";
const char * ABORTGAME="\nAbort this game ? Y/N\n";

const event_t  reenter_event = { ev_keydown, KEY_ENTER, 0, 1 }; // see M_Responder

void M_Choose_to_quit_Response(int ch)
{
    if (ch == 'y')
    {
#if 1
        // [WDJ] The quick way to exitgame, including netgame.
        Command_ExitGame_f();
#else
        // [WDJ] This is why it is not being done this way.
        // Why is this system being used for other menus ??
        // The hard way to exitgame
        COM_BufAddText("exitgame\n");
        // unfortunately this takes time, and a couple tics
        int i;
        for( i=100; i>0; i-- )
        {
            COM_BufExecute(); // subject to com_wait and other delays
            if( ! Game_Playing()  ) break;
            // It must be not-playing before re-invoking the menu.
        }
#endif       
        // YES, re-invoke the caller routine at itemOn press
        D_PostEvent(&reenter_event);  // delayed reinvoke
    }
}

// [WDJ] Ask user to quit, if they already have a game in-progess.
// Return 1 if already playing, and rejects quitting.
boolean  M_already_playing( boolean check_netgame )
{
    if( Game_Playing() )
    {
        M_StartMessage(ALLREADYPLAYING, M_Choose_to_quit_Response, MM_YESNO);
        return 1;
    }
    if (check_netgame && netgame)
    {
        // cannot start a new game while in a network game
        M_SimpleMessage(NEWGAME);
        snprintf(msgtmp, MSGTMP_LEN, "%s\n%s", NEWGAME, ABORTGAME );
        msgtmp[MSGTMP_LEN-1] = '\0';
        M_StartMessage(msgtmp, M_Choose_to_quit_Response, MM_YESNO);
        return 1;
    }
    return 0;
}

//===========================================================================
//MAIN MENU
//===========================================================================

void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_QuitDOOM(int choice);

enum
{
    newgame = 0,
    loadgame,
    savegame,
    options,
    readthis,	// referenced
    quitdoom,	// referenced
    main_end,
} main_e;

// Compatible with modifications to original graphics
menuitem_t MainMenu[]=
{
    {IT_SUBMENU | IT_PATCH,"M_NGAME" ,"NEW GAME" ,&SingleMultiDef,'n'},
    {IT_CALL    | IT_PATCH,"M_LOADG" ,"LOAD GAME",M_LoadGame,'l'},
    {IT_CALL    | IT_PATCH,"M_SAVEG" ,"SAVE GAME",M_SaveGame,'s'},
    {IT_SUBMENU | IT_PATCH,"M_OPTION","OPTIONS"  ,&OptionsDef,'o'},
    {IT_SUBMENU | IT_PATCH,"M_RDTHIS","INFO"     ,&ReadDef1  ,'r'},  // Another hickup with Special edition.
    {IT_CALL    | IT_PATCH,"M_QUITG" ,"QUIT GAME",M_QuitDOOM,'q'}
};

void HereticMainMenuDrawer(void)
{
    int frame = (I_GetTime()/3)%18;

    V_DrawScaledPatch_Num(40, 10, SkullBaseLump+(17-frame) );
    V_DrawScaledPatch_Num(232, 10, SkullBaseLump+frame );
    M_DrawGenericMenu();
}

menu_t  MainDef =
{
    "M_DOOM",
    NULL,
    main_end,
    NULL,
    MainMenu,
    M_DrawGenericMenu,
    97,64,
    0
};

//===========================================================================
//SINGLE/MULTI PLAYER GAME MENU
//===========================================================================

void M_SingleNewGame(int choice);
void M_TwoPlayerMenu(int choice);
void M_EndGame(int choice);

#if 0
enum
{
    sm_single = 0,
    sm_2player,
    sm_multi,
    sm_end
} singlemulti_e;
#endif

// DoomLegacy graphics from legacy.wad: M_SINGLE, M_2PLAYR, M_MULTI
menuitem_t SingleMulti_Menu[] =
{
    {IT_CALL | IT_PATCH,"M_SINGLE","SINGLE PLAYER",M_SingleNewGame ,'s'},
    {IT_CALL | IT_PATCH,"M_2PLAYR","TWO PLAYER GAME",M_TwoPlayerMenu ,'n'},
    {IT_SUBMENU | IT_PATCH,"M_MULTI" ,"MULTIPLAYER",&MultiPlayerDef  ,'m'},
    {IT_CALL | IT_PATCH,"M_ENDGAM","END GAME",M_EndGame ,'e'}
};

menu_t  SingleMultiDef =
{
    "M_NGAME",
    "Single Multi New Game",
    sizeof(SingleMulti_Menu)/sizeof(menuitem_t),
    &MainDef,
    SingleMulti_Menu,
    M_DrawGenericMenu,
    97,64,
    0
};


//===========================================================================
// Connect Menu
//===========================================================================

CV_PossibleValue_t serversearch_cons_t[] = {{0,"Local Lan"}
                                           ,{1,"Internet"}
                                           ,{0,NULL}};


consvar_t cv_serversearch = {"serversearch"    ,"0",CV_HIDEN,serversearch_cons_t};

#define FIRSTSERVERLINE 3

void M_Connect( int choice )
{
    // do not call menuexitfunc 
    M_ClearMenus(false);

    COM_BufAddText(va("connect node %d\n",
                      serverlist[choice-FIRSTSERVERLINE].server_node));
    setup_net_savegame();
}

static int localservercount;

void M_Refresh( int choice )
{
    CL_Update_ServerList( cv_serversearch.value );
}

menuitem_t  ConnectMenu[] =
{
    {IT_STRING | IT_CVAR ,0,"Search On"       ,&cv_serversearch       ,0},
    {IT_STRING | IT_CALL ,0,"Refresh"         ,M_Refresh              ,0},
    {IT_WHITESTRING | IT_SPACE,0,
                "Server Name                           ping players dm" ,0 ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
};

void M_DrawConnectMenu( void )
{
    int i, sly;
    char *p;

    for( i=FIRSTSERVERLINE; i<localservercount+FIRSTSERVERLINE; i++ )
        ConnectMenu[i].status = IT_STRING | IT_SPACE;

    sly = currentMenu->y + (FIRSTSERVERLINE * STRINGHEIGHT);
    if( serverlistcount <= 0 )
        V_DrawString (currentMenu->x, sly, 0, "No server found");
    else
    for( i=0; i<serverlistcount; i++ )
    {
        if( i >= ((sizeof(ConnectMenu)/sizeof(menuitem_t)) - FIRSTSERVERLINE) )
            break; // too many to draw all
        V_DrawString (currentMenu->x, sly, 0, serverlist[i].info.servername);
        p = va("%d", serverlist[i].info.trip_time);  // ping time
        V_DrawString (currentMenu->x + 200 - V_StringWidth(p), sly, 0, p);
        p = va("%d/%d  %d", serverlist[i].info.numberofplayer,
                            serverlist[i].info.maxplayer,
                            serverlist[i].info.deathmatch);
        V_DrawString (currentMenu->x + 250 - V_StringWidth(p), sly, 0, p);
        sly += STRINGHEIGHT;

        ConnectMenu[i+FIRSTSERVERLINE].status = IT_STRING | IT_CALL;
    }
    localservercount = serverlistcount;

    M_DrawGenericMenu();
}

boolean M_CancelConnect(void)
{
    D_CloseConnection();
    return true;
}

menu_t  Connectdef =
{
    "M_CONNEC", // in legacy.wad
    "Connect Server",
    sizeof(ConnectMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    ConnectMenu,
    M_DrawConnectMenu,
    27,40,
    0,
    M_CancelConnect
};

// Select Connect Menu
void M_ConnectMenu(int choice)
{
    if( M_already_playing(0) )  return;

    M_SetupNextMenu(&Connectdef);
    M_Refresh(0);
}

//===========================================================================
// Start Server Menu
//===========================================================================

CV_PossibleValue_t skill_cons_t[] = {{1,"I'm too young to die"}
                                    ,{2,"Hey, not too rough"}
                                    ,{3,"Hurt me plenty"}
                                    ,{4,"Ultra violence"}
                                    ,{5,"Nightmare!" }
                                    ,{0,NULL}};

CV_PossibleValue_t map_cons_t[] = {{ 1,"map01"} ,{ 2,"map02"} ,{ 3,"map03"}
                                  ,{ 4,"map04"} ,{ 5,"map05"} ,{ 6,"map06"}
                                  ,{ 7,"map07"} ,{ 8,"map08"} ,{ 9,"map09"}
                                  ,{10,"map10"} ,{11,"map11"} ,{12,"map12"}
                                  ,{13,"map13"} ,{14,"map14"} ,{15,"map15"}
                                  ,{16,"map16"} ,{17,"map17"} ,{18,"map18"}
                                  ,{19,"map19"} ,{20,"map20"} ,{21,"map21"}
                                  ,{22,"map22"} ,{23,"map23"} ,{24,"map24"}
                                  ,{25,"map25"} ,{26,"map26"} ,{27,"map27"}
                                  ,{28,"map28"} ,{29,"map29"} ,{30,"map30"}
                                  ,{31,"map31"} ,{32,"map32"} ,{0,NULL}};

CV_PossibleValue_t exmy_cons_t[] ={{11,"e1m1"} ,{12,"e1m2"} ,{13,"e1m3"}
                                  ,{14,"e1m4"} ,{15,"e1m5"} ,{16,"e1m6"}
                                  ,{17,"e1m7"} ,{18,"e1m8"} ,{19,"e1m9"}
                                  ,{21,"e2m1"} ,{22,"e2m2"} ,{23,"e2m3"}
                                  ,{24,"e2m4"} ,{25,"e2m5"} ,{26,"e2m6"}
                                  ,{27,"e2m7"} ,{28,"e2m8"} ,{29,"e2m9"}
                                  ,{31,"e3m1"} ,{32,"e3m2"} ,{33,"e3m3"}
                                  ,{34,"e3m4"} ,{35,"e3m5"} ,{36,"e3m6"}
                                  ,{37,"e3m7"} ,{38,"e3m8"} ,{39,"e3m9"}
                                  ,{41,"e4m1"} ,{42,"e4m2"} ,{43,"e4m3"}
                                  ,{44,"e4m4"} ,{45,"e4m5"} ,{46,"e4m6"}
                                  ,{47,"e4m7"} ,{48,"e4m8"} ,{49,"e4m9"}
                                  ,{41,"e5m1"} ,{42,"e5m2"} ,{43,"e5m3"}
                                  ,{44,"e5m4"} ,{45,"e5m5"} ,{46,"e5m6"}
                                  ,{47,"e5m7"} ,{48,"e5m8"} ,{49,"e5m9"}
                                  ,{0,NULL}};

consvar_t cv_skill    = {"skill"    ,"4",CV_HIDEN,skill_cons_t};
consvar_t cv_monsters = {"monsters" ,"0",CV_HIDEN,CV_YesNo};
consvar_t cv_nextmap  = {"nextmap"  ,"1",CV_HIDEN,map_cons_t};
consvar_t cv_nextepmap  = {"nextepmap"  ,"11",CV_HIDEN,exmy_cons_t};
extern CV_PossibleValue_t deathmatch_cons_t[];
consvar_t cv_newdeathmatch  = {"newdeathmatch"  ,"3",CV_HIDEN,deathmatch_cons_t};
CV_PossibleValue_t wait_players_cons_t[]=   {{0,"MIN"}, {32,"MAX"}, {0,NULL}};
consvar_t cv_wait_players = {"wait_players" ,"2",CV_HIDEN,wait_players_cons_t};
CV_PossibleValue_t wait_timeout_cons_t[]=   {{0,"MIN"}, {5,"INC"}, {244,"MAX"}, {0,NULL}};
consvar_t cv_wait_timeout = {"wait_timeout" ,"0",CV_HIDEN,wait_timeout_cons_t};

static boolean StartSplitScreenGame = false;

void M_StartServer( int choice )
{
    M_ClearMenus(true);

    netgame = true;
    multiplayer = true;
    if( choice == 9 )
    {
        dedicated = true;
        nodrawers = true;
        I_ShutdownGraphics();
    }
    D_WaitPlayer_Setup();

    COM_BufAddText(va("stopdemo;splitscreen %d;deathmatch %d;map \"%s\" -monsters %d skill %d\n", 
                      StartSplitScreenGame, cv_newdeathmatch.value, 
                      (gamemode==doom2_commercial)? cv_nextmap.string : cv_nextepmap.string,
                      cv_monsters.value, cv_skill.value));
    // skin change
    if (StartSplitScreenGame
        && ( ! displayplayer2_ptr
             || (cv_skin2.value != displayplayer2_ptr->skin) ) )
    {
        COM_BufAddText ( va("%s \"%s\"", cv_skin2.name, skins[cv_skin2.value].name));
    }
}

menuitem_t  ServerMenu[] =
{
    {IT_STRING | IT_CVAR,0,"Map"             ,&cv_nextmap          ,0},
    {IT_STRING | IT_CVAR,0,"Skill"           ,&cv_skill            ,0},
    {IT_STRING | IT_CVAR,0,"Monsters"        ,&cv_monsters         ,0},
    {IT_STRING | IT_CVAR,0,"Deathmatch Type" ,&cv_newdeathmatch    ,0},
    {IT_STRING | IT_CVAR,0,"Wait Players"    ,&cv_wait_players     ,0},
    {IT_STRING | IT_CVAR,0,"Wait Timeout"    ,&cv_wait_timeout     ,0},
    {IT_STRING | IT_CVAR,0,"Internet Server" ,&cv_internetserver   ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_STRING     ,0,"Server Name"     ,&cv_servername       ,0},
    {IT_WHITESTRING | IT_CALL | IT_YOFFSET,
                         0,"Start"           ,M_StartServer        ,110}, // 8
    {IT_WHITESTRING | IT_CALL | IT_YOFFSET,
                         0,"Dedicated"       ,M_StartServer        ,120}  // 9
};

menuitem_t  ServerMenu_Map =
    {IT_STRING | IT_CVAR,0,"Map"             ,&cv_nextmap          ,0};
menuitem_t  ServerMenu_EpisodeMap =
    {IT_STRING | IT_CVAR,0,"Episode Map"     ,&cv_nextepmap        ,0};

menu_t  ServerDef =
{
    "M_STSERV", // in legacy.wad
    "Start Server",
    sizeof(ServerMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    ServerMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

void M_StartServerMenu(int choice)
{
    if( M_already_playing(0) )  return;

    ServerMenu[0] = (gamemode==doom2_commercial)?
         ServerMenu_Map  // Doom2
       : ServerMenu_EpisodeMap;  // Ult doom, Heretic

    // StartSplitScreenGame already set by TwoPlayer menu
    M_SetupNextMenu(&ServerDef);
    setup_net_savegame();
}

//===========================================================================
//                            MULTI PLAYER MENU
//===========================================================================
void M_SetupMultiPlayer (int choice);
void M_SetupMultiPlayer2 (int choice);
void M_TwoPlayerMenu(int choice);

enum {
    startsplitscreengame = 0,
    setupplayer1,
    setupplayer2,	// referenced in M_Player2_MenuEnable
    multiplayeroptions,
    connectmultiplayermenu,
    startserver,
    end_game,
    multiplayer_end
} multiplayer_e;

// DoomLegacy graphics from legacy.wad: M_STSERV, M_CONNEC, M_2PLAYR, M_SETUPA, M_SETUPB
menuitem_t MultiPlayerMenu[] =
{
    // BIG font menu. BIG font does not work, lump is missing.
    // Cannot put all three options here.
    {IT_CALL | IT_PATCH,"M_2PLAYR","TWO PLAYER GAME",M_TwoPlayerMenu ,'n'},
    {IT_CALL | IT_PATCH,"M_SETUPA","SETUP PLAYER 1" ,M_SetupMultiPlayer ,'s'},
    {IT_CALL | IT_PATCH,"M_SETUPB","SETUP PLAYER 2" ,M_SetupMultiPlayer2 ,'t'},
    {IT_SUBMENU | IT_PATCH,"M_OPTION","OPTIONS"     ,&MPOptionDef ,'o'},
    {IT_CALL | IT_PATCH,"M_CONNEC","CONNECT SERVER" ,M_ConnectMenu ,'c'},
    {IT_CALL | IT_PATCH,"M_STSERV","CREATE SERVER"  ,M_StartServerMenu ,'a'},
    {IT_CALL | IT_PATCH,"M_ENDGAM","END GAME"       ,M_EndGame ,'e'}
};

menu_t  MultiPlayerDef =
{
    "M_MULTI", // in legacy.wad
    "Multiplayer",
    multiplayer_end,
    &SingleMultiDef,
    MultiPlayerMenu,
    M_DrawGenericMenu,
    85,40,
    0
};


//===========================================================================
// Two Player menu
//===========================================================================

#if 0
enum {
    setupplayer1,
    setupplayer2,
    multiplayeroptions,
    start,
    multiplayer,
    twoplayer_end
} twoplayer_e;
#endif

// DoomLegacy graphics from legacy.wad: M_SETUPA, M_SETUPB, M_STSERV, M_MULTI
menuitem_t TwoPlayerMenu[] =
{
    {IT_CALL | IT_PATCH,"M_SETUPA","SETUP PLAYER 1",M_SetupMultiPlayer ,'s'},
    {IT_CALL | IT_PATCH,"M_SETUPB","SETUP PLAYER 2",M_SetupMultiPlayer2 ,'t'},
    {IT_CALL | IT_PATCH,"M_OPTION","OPTIONS"       ,M_NetOption ,'o'},
    {IT_CALL | IT_PATCH,"M_STSERV","START GAME"    ,M_StartServerMenu , 0},
    {IT_SUBMENU | IT_PATCH,"M_MULTI" ,"MULTIPLAYER",&MultiPlayerDef  ,'m'},
};

menu_t  TwoPlayerDef =
{
    "M_2PLAYR",  // from legacy.wad
    "Two Player",
    sizeof(TwoPlayerMenu)/sizeof(menuitem_t),
    &SingleMultiDef,
    TwoPlayerMenu,
    M_DrawGenericMenu,
    85,40,
    0
};


void M_TwoPlayerMenu(int choice)
{
    TwoPlayerDef.prevMenu = currentMenu;
    StartSplitScreenGame = true;
    M_Player2_MenuEnable( 1 );
    M_SetupNextMenu(&TwoPlayerDef);
}


//===========================================================================
// Second mouse config for the splitscreen player
//===========================================================================

menuitem_t  SecondMouseCfgMenu[] =
{
    {IT_STRING | IT_CVAR,0,"Second Mouse Serial Port", &cv_mouse2port,0},
    {IT_STRING | IT_CVAR,0,"Use Mouse2", &cv_usemouse2        ,0},
    {IT_STRING | IT_CVAR,0,"Always Mouse2Look", &cv_alwaysfreelook2  ,0},
    {IT_STRING | IT_CVAR,0,"Mouse2 Move",       &cv_mouse2_move       ,0},
    {IT_STRING | IT_CVAR,0,"Invert Mouse2",     &cv_mouse2_invert     ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse2 x Speed",    &cv_mouse2_sens_x    ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse2 y Speed",    &cv_mouse2_sens_y    ,0},
};

menu_t  SecondMouseCfgdef =
{
    "M_OPTTTL",
    "Options",
    sizeof(SecondMouseCfgMenu)/sizeof(menuitem_t),
    &SetupMultiPlayerDef,
    SecondMouseCfgMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

//===========================================================================
// Second options for the splitscreen player
//===========================================================================

menuitem_t  SecondOptionsMenu[] =
{
    //Hurdler: for now, only autorun is implemented 
    //         others should be implemented as well if we want to be complete
//    {IT_STRING | IT_CVAR,"Messages:"       ,&cv_showmessages2    ,0},
    {IT_STRING | IT_CVAR,0,"Always Run"      ,&cv_autorun2         ,0},
//    {IT_STRING | IT_CVAR,"Crosshair"       ,&cv_crosshair2       ,0},
//    {IT_STRING | IT_CVAR,"Autoaim"         ,&cv_autoaim2         ,0},
//    {IT_STRING | IT_CVAR,"Control per key" ,&cv_controlperkey2   ,0},
};

menu_t  SecondOptionsdef =
{
    "M_OPTTTL",
    "Options",
    sizeof(SecondOptionsMenu)/sizeof(menuitem_t),
    &SetupMultiPlayerDef,
    SecondOptionsMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

//===========================================================================
//MULTI PLAYER SETUP MENU
//===========================================================================
void M_DrawSetupMultiPlayerMenu(void);
void M_HandleSetupMultiPlayer(int choice);
void M_SetupControlsMenu(int choice);
boolean M_QuitMultiPlayerMenu(void);

menuitem_t SetupMultiPlayerMenu[] =
{
    {IT_KEYHANDLER | IT_STRING          ,0,"Your name" ,M_HandleSetupMultiPlayer,0},
    {IT_CVAR | IT_STRING | IT_CV_NOPRINT | IT_YOFFSET, 0,"Your color",&cv_playercolor         ,16},
    {IT_KEYHANDLER | IT_STRING | IT_YOFFSET, 0,"Your skin" ,M_HandleSetupMultiPlayer,96},
#if 0
    /* this line calls the setup controls for secondary player, only if numitems is > 3 */
    //Hurdler: uncomment this line when other options are available
    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0,"Second Player config...", &SecondOptionsdef, 110},
#else
    //... and remove this one
    {IT_STRING | IT_CVAR | IT_YOFFSET,   0,"Always Run"      ,&cv_autorun2         ,110},
#endif
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Setup Controls...", M_SetupControlsMenu, 120},
    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0,"Second Mouse config...", &SecondMouseCfgdef, 130}
};

enum {
    setupmultiplayer_name = 0,
    setupmultiplayer_color,
    setupmultiplayer_skin,
    setupmultiplayer_options2,
    setupmultiplayer_controls,
    setupmultiplayer_mouse2,
    setupmulti_end
};

menu_t  SetupMultiPlayerDef =
{
    "M_MULTI", // in legacy.wad
    "Multiplayer",
    sizeof(SetupMultiPlayerMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    SetupMultiPlayerMenu,
    M_DrawSetupMultiPlayerMenu,
    27,40,
    0,
    M_QuitMultiPlayerMenu
};


#define PLBOXW    8
#define PLBOXH    9

static  int       multi_tics;
static  state_t*  multi_state;

// this is set before entering the MultiPlayer setup menu,
// for either player 1 or 2
static  char       setupm_name[MAXPLAYERNAME+1];
static  player_t*  setupm_player;
static  consvar_t* setupm_cvskin;
static  consvar_t* setupm_cvcolor;
static  consvar_t* setupm_cvname;

void M_SetupMultiPlayer (int choice)
{
    multi_state = &states[mobjinfo[MT_PLAYER].seestate];
    multi_tics = multi_state->tics;
    strcpy(setupm_name, cv_playername.string);
    SetupMultiPlayerDef.numitems = setupmultiplayer_skin +1;      //remove player2 setup controls and mouse2 

    // set for player 1
    SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor;
    setupm_player = consoleplayer_ptr;
    setupm_cvskin = &cv_skin;
    setupm_cvcolor = &cv_playercolor;
    setupm_cvname = &cv_playername;
    SetupMultiPlayerDef.prevMenu = currentMenu;
    M_SetupNextMenu (&SetupMultiPlayerDef);
}

// start the multiplayer setup menu, for secondary player (splitscreen mode)
void M_SetupMultiPlayer2 (int choice)
{
    multi_state = &states[mobjinfo[MT_PLAYER].seestate];
    multi_tics = multi_state->tics;
    strcpy (setupm_name, cv_playername2.string);
    SetupMultiPlayerDef.numitems = setupmulti_end;          //activate the setup controls for player 2

    // set for splitscreen secondary player
    SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor2;
    setupm_player = displayplayer2_ptr;  // player 2
    setupm_cvskin = &cv_skin2;
    setupm_cvcolor = &cv_playercolor2;
    setupm_cvname = &cv_playername2;
    SetupMultiPlayerDef.prevMenu = currentMenu;
    M_SetupNextMenu (&SetupMultiPlayerDef);
}


// called at cv_splitscreen changes
void M_Player2_MenuEnable( boolean player2_enable )
{
// activate setup for player 2
    if ( player2_enable )
        MultiPlayerMenu[setupplayer2].status = IT_CALL | IT_PATCH;
    else
    {
        MultiPlayerMenu[setupplayer2].status = IT_DISABLED;
        if( MultiPlayerDef.lastOn==setupplayer2)
            MultiPlayerDef.lastOn=setupplayer1;
    }
}


//
//  Draw the multi player setup menu, had some fun with player anim
//
void M_DrawSetupMultiPlayerMenu(void)
{
    int             mx,my;
    spritedef_t*    sprdef;
    spriteframe_t*  sprframe;
    int             lump;
    patch_t*        patch;
    int             st;
    byte*           colormap;

    // Draw to screen0, scaled
    mx = SetupMultiPlayerDef.x;
    my = SetupMultiPlayerDef.y;

    // use generic drawer for cursor, items and title
    M_DrawGenericMenu();

    // draw name string
    M_DrawTextBox(mx+90,my-8,MAXPLAYERNAME,1);
    V_DrawString (mx+98,my,0,setupm_name);

    // draw skin string
    V_DrawString (mx+90, my+96,0, setupm_cvskin->string);

    // draw text cursor for name
    if (itemOn==0 &&
        skullAnimCounter<4)   //blink cursor
        V_DrawCharacter(mx+98+V_StringWidth(setupm_name), my, '_' | 0x80);  // white

    // draw box around guy
    M_DrawTextBox(mx+90,my+8, PLBOXW, PLBOXH);

    // anim the player in the box
    if (--multi_tics<=0)
    {
        st = multi_state->nextstate;
        if (st!=S_NULL)
            multi_state = &states[st];
        multi_tics = multi_state->tics;
        if (multi_tics==-1)
            multi_tics=15;
    }

    // skin 0 is default player sprite
    sprdef    = &skins[R_SkinAvailable(setupm_cvskin->string)].spritedef;
    sprframe  = &sprdef->spriteframes[ multi_state->frame & FF_FRAMEMASK];
    lump  = sprframe->lumppat[0];

    colormap = (setupm_cvcolor->value) ?
         SKIN_TO_SKINMAP( setupm_cvcolor->value )
       : reg_colormaps;  // default green skin

    // draw player sprite
    // temp usage of sprite lump, until end of function
    patch = W_CachePatchNum (lump, PU_CACHE_DEFAULT);  // endian fix
    V_DrawMappedPatch (mx+98+(PLBOXW*8/2),my+16+(PLBOXH*8)-8, patch, colormap);
}


//
// Handle Setup MultiPlayer Menu
//
void M_HandleSetupMultiPlayer (int key)
{
    int      l;
    boolean  exitmenu = false;  // exit to previous menu and send name change
    int      myskin;

    myskin  = setupm_cvskin->value;

    switch (key)
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL, menu_sfx_updown);
        if (itemOn+1 >= SetupMultiPlayerDef.numitems)
            itemOn = 0;
        else itemOn++;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL, menu_sfx_updown);
        if (!itemOn)
            itemOn = SetupMultiPlayerDef.numitems-1;
        else itemOn--;
        break;

      case KEY_LEFTARROW:
        if (itemOn==2)       //player skin
        {
            S_StartSound(NULL, menu_sfx_val);
            myskin--;
        }
        break;

      case KEY_RIGHTARROW:
        if (itemOn==2)       //player skin
        {
            S_StartSound(NULL, menu_sfx_val);
            myskin++;
        }
        break;

      case KEY_ENTER:
        S_StartSound(NULL, menu_sfx_enter);
        exitmenu = true;
        break;

      case KEY_ESCAPE:
        S_StartSound(NULL, menu_sfx_esc);
        exitmenu = true;
        break;

      case KEY_BACKSPACE:
        if ( (l=strlen(setupm_name))!=0 && itemOn==0)
        {
            S_StartSound(NULL, menu_sfx_val);
            setupm_name[l-1]=0;
        }
        break;

      default:
        if (!is_printable(input_char) || itemOn != 0)
          break;
        l = strlen(setupm_name);
        if (l<MAXPLAYERNAME-1)
        {
            S_StartSound(NULL, menu_sfx_val);
            setupm_name[l] = input_char;
            setupm_name[l+1] = 0;
        }
        break;
    }

    // check skin
    if (myskin <0)
        myskin = numskins-1;
    if (myskin >numskins-1)
        myskin = 0;

    // check skin change
    // If not updated here then another chance after server start
    if (setupm_player && myskin != setupm_player->skin)
        COM_BufAddText ( va("%s \"%s\"",setupm_cvskin->name ,skins[myskin].name));
    setupm_cvskin->value = myskin;

    if (exitmenu)
    {
        M_Setup_prevMenu();
    }
}

boolean M_QuitMultiPlayerMenu(void)
{
    int      l;
    // send name if changed
    if (strcmp(setupm_name, setupm_cvname->string))
    {
        // remove trailing whitespaces
        for (l= strlen(setupm_name)-1;
             l>=0 && setupm_name[l]==' '; l--)
            setupm_name[l]=0;
        COM_BufAddText ( va("%s \"%s\"",setupm_cvname->name ,setupm_name));
        
    }
    return true;
}


//===========================================================================
//                              EPISODE SELECT
//===========================================================================

void M_Episode(int choice);

enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep5,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {IT_CALL | IT_PATCH,"M_EPI1","Knee-Deep in the Dead", M_Episode,'k'},
    {IT_CALL | IT_PATCH,"M_EPI2","The Shores of Hell"   , M_Episode,'t'},
    {IT_CALL | IT_PATCH,"M_EPI3","Inferno"              , M_Episode,'i'},
    {IT_CALL | IT_PATCH,"M_EPI4","Thy Flesh consumed"   , M_Episode,'t'},
    {IT_CALL | IT_PATCH,"M_EPI5","Episode 5"            , M_Episode,'t'},
};

menu_t  EpiDef =
{
    "M_EPISOD",
    "Which Episode?",
    ep_end,             // # of menu items
    &MainDef,           // previous menu
    EpisodeMenu,        // menuitem_t ->
    M_DrawGenericMenu,  // drawing routine ->
    48,63,              // x,y
    ep1                 // lastOn, flags
};

//
//      M_Episode
//
int     epi;

void M_Episode(int choice)
{
    if ( (gamemode == doom_shareware)
         && choice)
    {
        M_SetupNextMenu(&ReadDef1);
        M_SimpleMessage(SWSTRING);
        return;
    }

    // Yet another hack...
    if ( (gamemode == doom_registered)
         && (choice > 2))
    {
        I_Error("M_Episode: 4th episode requires UltimateDOOM\n");
        choice = 0;
    }

    epi = choice;
    M_SetupNextMenu(&NewDef);
}


//===========================================================================
//                           NEW GAME FOR SINGLE PLAYER
//===========================================================================
void M_DrawNewGame(void);

void M_ChooseSkill(int choice);

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {IT_CALL | IT_PATCH,"M_JKILL","I'm too young to die.",M_ChooseSkill, 'i'},
    {IT_CALL | IT_PATCH,"M_ROUGH","Hey, not too rough."  ,M_ChooseSkill, 'h'},
    {IT_CALL | IT_PATCH,"M_HURT" ,"Hurt me plenty."      ,M_ChooseSkill, 'h'},
    {IT_CALL | IT_PATCH,"M_ULTRA","Ultra-Violence"       ,M_ChooseSkill, 'u'},
    {IT_CALL | IT_PATCH,"M_NMARE","Nightmare!"           ,M_ChooseSkill, 'n'}
};

menu_t  NewDef =
{
    "M_NEWG",
    "New Game",
    newg_end,           // # of menu items
    &EpiDef,            // previous menu
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    violence            // lastOn
};

void M_DrawNewGame(void)
{
    // Draw to screen0, scaled
    //faB: testing with glide
    patch_t* p = W_CachePatchName("M_SKILL",PU_CACHE);  // endian fix
    V_DrawScaledPatch ((BASEVIDWIDTH-p->width)/2,38, p);

    //    V_DrawScaledPatch_Name (54,38, "M_SKILL" );
    M_DrawGenericMenu();
}

void M_SingleNewGame(int choice)
{
    // to get out of two player game, and can then backout to multiplayer
    StartSplitScreenGame = false;
    M_Player2_MenuEnable( 0 );

    if( M_already_playing(1) )  return;

    if ( gamemode == doom2_commercial
         || (gamemode == chexquest1 && !modifiedgame) //DarkWolf95: Support for Chex Quest
         )
        M_SetupNextMenu(&NewDef);
    else
        M_SetupNextMenu(&EpiDef);
}

void M_VerifyNightmare(int ch);

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(NIGHTMARE, M_VerifyNightmare, MM_YESNO);
        return;
    }

    G_DeferedInitNew(choice, G_BuildMapName(epi+1,1), StartSplitScreenGame);
    M_ClearMenus (true);
}

void M_VerifyNightmare(int ch)
{
    if (ch != 'y')
        return;

    G_DeferedInitNew (nightmare, G_BuildMapName(epi+1,1), StartSplitScreenGame);
    M_ClearMenus (true);
}

//===========================================================================
//                             OPTIONS MENU
//===========================================================================
//
// M_Options
//

menuitem_t OptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Messages:"       ,&cv_showmessages    ,0},
    {IT_STRING | IT_CVAR,0,"Always Run"      ,&cv_autorun         ,0},
    {IT_STRING | IT_CVAR,0,"Crosshair"       ,&cv_crosshair       ,0},
//    {IT_STRING | IT_CVAR,0,"Crosshair scale" ,&cv_crosshairscale  ,0},
    {IT_STRING | IT_CVAR,0,"Autoaim"         ,&cv_autoaim         ,0},
    {IT_STRING | IT_CVAR,0,"Control per key" ,&cv_controlperkey   ,0},

    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0,"Effects Options...",&EffectsOptionsDef ,60},
    {IT_CALL    | IT_WHITESTRING,0,"Game Options..."  ,M_GameOption       ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Connect Options...",&ConnectOptionDef ,0},
    {IT_CALL    | IT_WHITESTRING,0,"Network Options...",M_NetOption     ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Server Options...",&ServerOptionsDef  ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Sound Volume..."  ,&SoundDef          ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Video Options..." ,&VideoOptionsDef   ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Mouse Options..." ,&MouseOptionsDef   ,0},
    {IT_CALL    | IT_WHITESTRING,0,"Setup Controls...",M_SetupControlsMenu,0}
};

menu_t  OptionsDef =
{
    "M_OPTTTL",
    "Options",
    sizeof(OptionsMenu)/sizeof(menuitem_t),
    &MainDef,
    OptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//
//  A smaller 'Thermo', with range given as percents (0-100)
//
void M_DrawSlider (int x, int y, int range)
{
    int i;

    // Draw to screen0, scaled
    if (range < 0)
        range = 0;
    if (range > 100)
        range = 100;

    V_DrawScaledPatch_Name(x-8, y, "M_SLIDEL"); // in legacy.wad

    for (i=0 ; i<SLIDER_RANGE ; i++)
      V_DrawScaledPatch_Name(x+i*8, y, "M_SLIDEM"); // in legacy.wad

    V_DrawScaledPatch_Name(x+SLIDER_RANGE*8, y, "M_SLIDER"); // in legacy.wad

    // draw the slider cursor
    V_DrawMappedPatch_Name(x + ((SLIDER_RANGE-1)*8*range)/100, y,
                           "M_SLIDEC", whitemap); // in legacy.wad
}

//===========================================================================
//                        Effects OPTIONS MENU
//===========================================================================

menuitem_t EffectsOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,    "Translucency"     , &cv_translucency  , 0},
    {IT_STRING | IT_CVAR,0,    "Splats"           , &cv_splats        , 0},
    {IT_STRING | IT_CVAR,0,    "Max splats"       , &cv_maxsplats     , 0},
    {IT_STRING | IT_CVAR,0,    "Sprites limit"    , &cv_spritelim     , 0},
    {IT_STRING | IT_CVAR,0,    "Screens Link"     , &cv_screenslink   , 0},
    {IT_STRING | IT_CVAR,0,    "Pickup Flash"     , &cv_pickupflash   , 0},
    {IT_STRING | IT_CVAR,0,    "Random sound pitch",&cv_rndsoundpitch , 0},
    {IT_STRING | IT_CVAR,0,    "Menu Sounds",       &cv_menusound     , 0},
    {IT_STRING | IT_CVAR,0,    "Boom Colormap"    , &cv_boom_colormap , 0},
    {IT_STRING | IT_CVAR,0,    "Water Effect"    , &cv_water_effect   , 0},
    {IT_STRING | IT_CVAR,0,    "Fog Effect"      , &cv_fog_effect     , 0},
};

menu_t  EffectsOptionsDef =
{
    "M_OPTTTL",
    "Effects",
    sizeof(EffectsOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    EffectsOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Video OPTIONS MENU
//===========================================================================

menuitem_t VideoOptionsMenu[]=
{
    {IT_STRING | IT_SUBMENU,0, "Video Modes..."   , &VidModeDef       , 0},
// if these are moved then fix MenuGammaFunc_dependencies
    {IT_STRING | IT_CVAR,0,    "Gamma Function"   , &cv_gammafunc     , 0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,    "Gamma"            , &cv_usegamma      , 0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,    "Black level"      , &cv_black         , 0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,    "Brightness"       , &cv_bright        , 0},
    {IT_STRING | IT_CVAR,0,    "Wait Retrace"     , &cv_vidwait       , 0},
#ifndef __DJGPP__
    {IT_STRING | IT_CVAR,0,    "Fullscreen"       , &cv_fullscreen    , 0},
#endif
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,    "Screen Size"      , &cv_viewsize      , 0},
    {IT_STRING | IT_CVAR,0,    "Scale Status Bar" , &cv_scalestatusbar, 0},
    {IT_STRING | IT_CVAR,0,    "Dark Back"        , &cv_darkback      , 0},
    {IT_STRING | IT_CVAR,0,    "Console font"     , &cv_con_fontsize  , 0},
    {IT_STRING | IT_CVAR,0,    "Message font"     , &cv_msg_fontsize  , 0},
    {IT_STRING | IT_CVAR,0,    "Show Ticrate"     , &cv_ticrate       , 0},
#ifdef HWRENDER
    //17/10/99: added by Hurdler
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0, "3D Card Options...", M_OpenGLOption    ,130},
#endif
};

menu_t  VideoOptionsDef =
{
    "M_OPTTTL",
    "Video Options",
    sizeof(VideoOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    VideoOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void MenuGammaFunc_dependencies( byte gamma_en,
                                 byte black_en, byte bright_en )
{
   VideoOptionsMenu[2].status = 
     ( gamma_en ) ? (IT_STRING | IT_CVAR | IT_CV_SLIDER )
       : (IT_WHITESTRING | IT_SPACE);
   VideoOptionsMenu[3].status = 
     ( black_en ) ? (IT_STRING | IT_CVAR | IT_CV_SLIDER )
       : (IT_WHITESTRING | IT_SPACE);
   VideoOptionsMenu[4].status = 
     ( bright_en ) ? (IT_STRING | IT_CVAR | IT_CV_SLIDER )
       : (IT_WHITESTRING | IT_SPACE);
}

//===========================================================================
//                        Mouse OPTIONS MENU
//===========================================================================

menuitem_t MouseOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Use Mouse",        &cv_usemouse        ,0},
    {IT_STRING | IT_CVAR,0,"Always MouseLook", &cv_alwaysfreelook  ,0},
    {IT_STRING | IT_CVAR,0,"Mouse Move",    &cv_mouse_move      ,0},
    {IT_STRING | IT_CVAR,0,"Invert Mouse",  &cv_mouse_invert    ,0},
#ifdef SMIF_SDL
    {IT_STRING | IT_CVAR,0,"Mouse motion",  &cv_mouse_motion    ,0},
#endif
    {IT_STRING | IT_CVAR,0,"Grab input", &cv_grabinput ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse x Speed", &cv_mouse_sens_x    ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse y Speed", &cv_mouse_sens_y    ,0}
#if 0
//[WDJ] disabled in 143beta_macosx
//[segabor]
# ifdef MACOS_DI
// specific to macos directory
    ,{IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Configure Input Sprocket..."  ,macConfigureInput     ,60}
# endif
#endif
};

menu_t  MouseOptionsDef =
{
    "M_OPTTTL",
    "Mouse Options",
    sizeof(MouseOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    MouseOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Game OPTIONS MENU
//===========================================================================

menuitem_t GameOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Item Respawn"        ,&cv_itemrespawn        ,0},
    {IT_STRING | IT_CVAR,0,"Item Respawn time"   ,&cv_itemrespawntime    ,0},
    {IT_STRING | IT_CVAR,0,"Monster Respawn"     ,&cv_respawnmonsters    ,0},
    {IT_STRING | IT_CVAR,0,"Monster Respawn time",&cv_respawnmonsterstime,0},
    {IT_STRING | IT_CVAR,0,"Monster Behavior"	 ,&cv_monbehavior        ,0},
    {IT_STRING | IT_CVAR,0,"Fast Monsters"       ,&cv_fastmonsters       ,0},
    {IT_STRING | IT_CVAR,0,"Predicting Monsters" ,&cv_predictingmonsters ,0},	//added by AC for predmonsters
    {IT_STRING | IT_CVAR,0,"Solid corpse"        ,&cv_solidcorpse        ,0},
    {IT_STRING | IT_CVAR,0,"BloodTime"           ,&cv_bloodtime          ,0},
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Adv Options..."      ,M_AdvOption     ,120},
//    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Network Options..."  ,M_NetOption     ,130}
    {IT_CALL   | IT_WHITESTRING,0,"Network Options..."  ,M_NetOption     ,0}
};

menu_t  GameOptionDef =
{
    "M_OPTTTL",
    "Game Options",
    sizeof(GameOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    GameOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_GameOption(int choice)
{
    if(!server)
    {
        M_SimpleMessage("You are not the server\nYou cannot change game options\n");
        return;
    }
    M_SetupNextMenu(&GameOptionDef);
}

//===========================================================================
//                        Adv OPTIONS MENU
//===========================================================================

menuitem_t AdvOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Gravity"             ,&cv_gravity            ,0},
    {IT_STRING | IT_CVAR,0,"Monster friction"    ,&cv_monsterfriction    ,0},  // [WDJ]
    {IT_STRING | IT_CVAR,0,"Monster door stuck"  ,&cv_doorstuck          ,0},  // [WDJ]
    {IT_STRING | IT_CVAR,0,"Voodoo mode"         ,&cv_voodoo_mode        ,0},  // [WDJ]
    {IT_STRING | IT_CVAR,0,"Insta-death"         ,&cv_instadeath         ,0},  // [WDJ]
#ifdef DOORDELAY_CONTROL
    {IT_STRING | IT_CVAR,0,"Door Delay"          ,&cv_doordelay          ,0},  // [WDJ]
#endif
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Games Options..."    ,M_GameOption    ,120},
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Network Options..."  ,M_NetOption     ,130}
};

menu_t  AdvOptionDef =
{
    "M_OPTTTL",
    "Adv Options",
    sizeof(AdvOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    AdvOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_AdvOption(int choice)
{
    if(!server)
    {
        M_SimpleMessage("You are not the server\nYou cannot change adv options\n");
        return;
    }
    M_SetupNextMenu(&AdvOptionDef);
}

//===========================================================================
//                        Network OPTIONS MENU
//===========================================================================

menuitem_t NetOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Allow Jump"      ,&cv_allowjump       ,0},
    //SoM: 3/28/2000
    {IT_STRING | IT_CVAR,0,"Allow Rocket Jump",&cv_allowrocketjump,0},
    {IT_STRING | IT_CVAR,0,"Allow autoaim"   ,&cv_allowautoaim    ,0},
    {IT_STRING | IT_CVAR,0,"Allow turbo"     ,&cv_allowturbo      ,0},
    {IT_STRING | IT_CVAR,0,"Allow exitlevel" ,&cv_allowexitlevel  ,0},
    {IT_STRING | IT_CVAR,0,"Allow join player",&cv_allownewplayer ,0},
    {IT_STRING | IT_CVAR,0,"Teamplay"        ,&cv_teamplay        ,0},
    {IT_STRING | IT_CVAR,0,"TeamDamage"      ,&cv_teamdamage      ,0},
    {IT_STRING | IT_CVAR,0,"Fraglimit"       ,&cv_fraglimit       ,0},
    {IT_STRING | IT_CVAR,0,"Timelimit"       ,&cv_timelimit       ,0},
    {IT_STRING | IT_CVAR,0,"Deathmatch Type" ,&cv_deathmatch      ,0},
    {IT_STRING | IT_CVAR,0,"Frag's Weapon Falling", &cv_fragsweaponfalling, 0},
    {IT_STRING | IT_CVAR,0,"Maxplayers"      ,&cv_maxplayers      ,0},
    {IT_CALL | IT_WHITESTRING | IT_YOFFSET, 0,"Games Options..." ,M_GameOption ,132},
};

menu_t  NetOptionDef =
{
    "M_OPTTTL",
    "Net Options",
    sizeof(NetOptionsMenu)/sizeof(menuitem_t),
    &MPOptionDef,
    NetOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_NetOption(int choice)
{
    if(!server)
    {
        M_SimpleMessage("You are not the server\nYou cannot change network options\n");
        return;
    }
    NetOptionDef.prevMenu = currentMenu;
    M_SetupNextMenu(&NetOptionDef);
}

//===========================================================================
//                    Connect OPTIONS MENU
//===========================================================================


menuitem_t ConnectOptionMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Download files", &cv_downloadfiles , 0},
    {IT_STRING | IT_CVAR | IT_CV_STRING,0,"Server 1", &cv_server1 , 0},
    {IT_STRING | IT_CVAR | IT_CV_STRING,0,"Server 2", &cv_server2 , 0},
    {IT_STRING | IT_CVAR | IT_CV_STRING,0,"Server 3", &cv_server3 , 0},
};

menu_t  ConnectOptionDef =
{
    "M_OPTTTL",
    "Connect Options",
    sizeof(ConnectOptionMenu)/sizeof(menuitem_t),
    &MPOptionDef,
    ConnectOptionMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_ConnectOption(int choice)
{
    ConnectOptionDef.prevMenu = currentMenu;
    M_SetupNextMenu(&ConnectOptionDef);
}


//===========================================================================
//                        Server OPTIONS MENU
//===========================================================================
menuitem_t ServerOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Internet server",     &cv_internetserver   ,  0},
    {IT_STRING | IT_CVAR
        | IT_CV_STRING  ,0, "Master server",       &cv_masterserver     ,  0},
    {IT_STRING | IT_CVAR
        | IT_CV_STRING  ,0, "Server name",         &cv_servername       ,  0},
};

menu_t  ServerOptionsDef =
{
    "M_OPTTTL",
    "Server Options",
    sizeof(ServerOptionsMenu)/sizeof(menuitem_t),
    &MPOptionDef,
    ServerOptionsMenu,
    M_DrawGenericMenu,
    28,40,
    0
};

//===========================================================================
//                    MultiPlayer OPTIONS MENU
//===========================================================================


menuitem_t MPOptionMenu[]=
{
    {IT_SUBMENU | IT_WHITESTRING,0,"Connect Options...",&ConnectOptionDef ,0},
    {IT_CALL    | IT_WHITESTRING,0,"Network Options...",M_NetOption       ,0},
    {IT_SUBMENU | IT_WHITESTRING,0,"Server Options...",&ServerOptionsDef  ,0},
    {IT_CALL    | IT_WHITESTRING,0,"Game Options..."  ,M_GameOption       ,0},
};

menu_t  MPOptionDef =
{
    "M_OPTTTL",
    "MultiPlayer Options",
    sizeof(MPOptionMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    MPOptionMenu,
    M_DrawGenericMenu,
    60,40,
    0
};


//===========================================================================
//                          Read This! MENU 1
//===========================================================================

void M_DrawReadThis1(void);
void M_DrawReadThis2(void);

enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {IT_SUBMENU | IT_NOTHING,0,"",&ReadDef2,0}
};

menu_t  ReadDef1 =
{
    NULL,
    NULL,
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    // Draw to screen0, scaled
    switch ( gamemode )
    {
      case doom2_commercial:
        V_DrawScaledPatch_Name (0,0, "HELP");
        break;
      case doom_shareware:
      case doom_registered:
      case ultdoom_retail:
        V_DrawScaledPatch_Name (0,0, "HELP1");
        break;
      case heretic:
        V_DrawRawScreen_Num(0,0,W_GetNumForName("HELP1"), 320, 200);
        break;
      default:
        break;
    }
    return;
}

//===========================================================================
//                          Read This! MENU 2
//===========================================================================

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {IT_SUBMENU | IT_NOTHING,0,"",&MainDef,0}
};

menu_t  ReadDef2 =
{
    NULL,
    NULL,
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    // Draw to screen0, scaled
    switch ( gamemode )
    {
      case ultdoom_retail:
      case doom2_commercial:
        // This hack keeps us from having to change menus.
        V_DrawScaledPatch_Name (0,0, "CREDIT");
        break;
      case doom_shareware:
      case doom_registered:
        V_DrawScaledPatch_Name (0,0, "HELP2");
        break;
      case heretic :
        V_DrawRawScreen_Num(0,0,W_GetNumForName("HELP2"), 320, 200);
      default:
        break;
    }
    return;
}

//===========================================================================
//                        SOUND VOLUME MENU
//===========================================================================

void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_CDAudioVol (int choice);

// [WDJ] unique names, mostly unused
enum
{
    SVM_sfx_vol,
    SVM_sfx_empty1,
    SVM_music_vol,
    SVM_sfx_empty2,
    SVM_cdaudio_vol,
    SVM_sfx_empty3,
    SVM_sfx_mus_pref,
    SVM_sound_end
} SVM_sound_e;

// DoomLegacy graphics from legacy.wad: M_CDVOL
menuitem_t SoundMenu[]=
{
    {IT_CVARMAX   | IT_PATCH ,"M_SFXVOL","Sound Volume",&cv_soundvolume  ,'s'},
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cv_soundvolume      },
    {IT_CVARMAX   | IT_PATCH ,"M_MUSVOL","Music Volume",&cv_musicvolume  ,'m'},
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cv_musicvolume      },
#ifdef CDMUS
    {IT_CVARMAX   | IT_PATCH ,"M_CDVOL" ,"CD Volume"   ,&cd_volume       ,'c'}, // in legacy.wad
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cd_volume           },
#endif
#ifdef MUSSERV
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Music Pref",  &cv_musserver_opt , 110},
#endif
};

menu_t  SoundDef =
{
    "M_SVOL",
    "Sound Volume",
    SVM_sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawGenericMenu,
    80,50,
    0
};


//===========================================================================
//                          CONTROLS MENU
//===========================================================================
void M_DrawControl(void);               // added 3-1-98
void M_ChangeControl(int choice);

//
// this is the same for all control pages
//
// IT_CONTROL: alphaKey is the control to be changed
menuitem_t ControlMenu[]=
{
    {IT_CONTROL, 0,"Fire"        ,M_ChangeControl,gc_fire       },
    {IT_CONTROL, 0,"Use/Open"    ,M_ChangeControl,gc_use        },
    {IT_CONTROL, 0,"Jump"        ,M_ChangeControl,gc_jump       },
    {IT_CONTROL, 0,"Forward"     ,M_ChangeControl,gc_forward    },
    {IT_CONTROL, 0,"Backpedal"   ,M_ChangeControl,gc_backward   },
    {IT_CONTROL, 0,"Turn Left"   ,M_ChangeControl,gc_turnleft   },
    {IT_CONTROL, 0,"Turn Right"  ,M_ChangeControl,gc_turnright  },
    {IT_CONTROL, 0,"Run"         ,M_ChangeControl,gc_speed      },
    {IT_CONTROL, 0,"Strafe On"   ,M_ChangeControl,gc_strafe     },
    {IT_CONTROL, 0,"Strafe Left" ,M_ChangeControl,gc_strafeleft },
    {IT_CONTROL, 0,"Strafe Right",M_ChangeControl,gc_straferight},
    {IT_CONTROL, 0,"Look Up"     ,M_ChangeControl,gc_lookup     },
    {IT_CONTROL, 0,"Look Down"   ,M_ChangeControl,gc_lookdown   },
    {IT_CONTROL, 0,"Center View" ,M_ChangeControl,gc_centerview },
    {IT_CONTROL, 0,"Mouselook"   ,M_ChangeControl,gc_mouseaiming},

    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0,"next" ,&ControlDef2,128}
};

menu_t  ControlDef =
{
    "M_CONTRO", // in legacy.wad
    "Setup Controls",
    sizeof(ControlMenu)/sizeof(menuitem_t),
    &OptionsDef,
    ControlMenu,
    M_DrawControl,
    50,40,
    0
};

//
//  Controls page 1
//
// IT_CONTROL: alphaKey is the control to be changed
menuitem_t ControlMenu2[]=
{
  {IT_CONTROL, 0,"Fist/Chainsaw"  ,M_ChangeControl,gc_weapon1},
  {IT_CONTROL, 0,"Pistol"         ,M_ChangeControl,gc_weapon2},
  {IT_CONTROL, 0,"Shotgun/Double" ,M_ChangeControl,gc_weapon3},
  {IT_CONTROL, 0,"Chaingun"       ,M_ChangeControl,gc_weapon4},
  {IT_CONTROL, 0,"Rocket Launcher",M_ChangeControl,gc_weapon5},
  {IT_CONTROL, 0,"Plasma rifle"   ,M_ChangeControl,gc_weapon6},
  {IT_CONTROL, 0,"BFG"            ,M_ChangeControl,gc_weapon7},
  {IT_CONTROL, 0,"Chainsaw"       ,M_ChangeControl,gc_weapon8},
  {IT_CONTROL, 0,"Previous Weapon",M_ChangeControl,gc_prevweapon},
  {IT_CONTROL, 0,"Next Weapon"    ,M_ChangeControl,gc_nextweapon},
  {IT_CONTROL, 0,"Best Weapon"    ,M_ChangeControl,gc_bestweapon},
  {IT_CONTROL, 0,"Talk key"       ,M_ChangeControl,gc_talkkey},
  {IT_CONTROL, 0,"Rankings/Scores",M_ChangeControl,gc_scores },
  {IT_CONTROL, 0,"Console"        ,M_ChangeControl,gc_console},
  {IT_CONTROL, 0,"Inventory Left" ,M_ChangeControl,gc_invprev},  
  {IT_CONTROL, 0,"Inventory Right",M_ChangeControl,gc_invnext},
  {IT_CONTROL, 0,"Inventory Use"  ,M_ChangeControl,gc_invuse },
  {IT_CONTROL, 0,"Fly down"       ,M_ChangeControl,gc_flydown},
                       
  {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0,"next"    ,&ControlDef,148}
};

menu_t  ControlDef2 =
{
    "M_CONTRO", // in legacy.wad
    "Setup Controls",
    sizeof(ControlMenu2)/sizeof(menuitem_t),
    &OptionsDef,
    ControlMenu2,
    M_DrawControl,
    50,40,
    0
};


//
// Start the controls menu, setting it up for either the console player,
// or the secondary splitscreen player
//
static  boolean setupcontrols_secondaryplayer;
static  int   (*setupcontrols)[2];  // pointer to the gamecontrols of the player being edited
void M_SetupControlsMenu (int choice)
{
    // set the gamecontrols to be edited
    if (choice == setupmultiplayer_controls) {
        setupcontrols_secondaryplayer = true;
        setupcontrols = gamecontrol2;     // was called from secondary player's multiplayer setup menu
    }
    else {
        setupcontrols_secondaryplayer = false;
        setupcontrols = gamecontrol;        // was called from main Options (for console player, then)
    }
    currentMenu->lastOn = itemOn;
    ControlDef.prevMenu = currentMenu;
    M_SetupNextMenu(&ControlDef);
}


//
//  Draws the Customized Controls menu
//
void M_DrawControl(void)
{
    char     tmp[50];
    int      i;
    int      keys[2];
    menuitem_t * mip;

    // draw title, strings and submenu
    M_DrawGenericMenu();

    M_CentreText (ControlDef.y-12,
        (setupcontrols_secondaryplayer ? "SET CONTROLS FOR SECONDARY PLAYER" :
                                         "PRESS ENTER TO CHANGE, BACKSPACE TO CLEAR") );

    for(i=0;i<currentMenu->numitems;i++)
    {
        mip = & currentMenu->menuitems[i];
        if (mip->status != IT_CONTROL)
            continue;

        // alphaKey is the control to be changed
        keys[0] = setupcontrols[mip->alphaKey][0];
        keys[1] = setupcontrols[mip->alphaKey][1];

        tmp[0]='\0';
        if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
        {
            strcpy(tmp, "---");
        }
        else
        {
            if( keys[0] != KEY_NULL )
                strcat (tmp, G_KeynumToString (keys[0]));

            if( keys[0] != KEY_NULL && keys[1] != KEY_NULL )
                strcat(tmp," or ");

            if( keys[1] != KEY_NULL )
                strcat (tmp, G_KeynumToString (keys[1]));


        }
        V_DrawString(ControlDef.x+220-V_StringWidth(tmp), ControlDef.y + i*8,V_WHITEMAP, tmp);
    }

}

static int controltochange;

void M_ChangecontrolResponse(event_t* ev)
{
    int        control;
    int        found;
    int        ch=ev->data1;

    // ESCAPE cancels
    if (ch!=KEY_ESCAPE && ch!=KEY_PAUSE)
    {

        switch (ev->type)
        {
          // ignore mouse/joy movements, just get buttons
          case ev_mouse:
               ch = KEY_NULL;      // no key
            break;

          // keypad arrows are converted for the menu in cursor arrows
          // so use the event instead of ch
          case ev_keydown:
            ch = ev->data1;
            break;

          default:
            break;
        }

        control = controltochange;

        // check if we already entered this key
        found = -1;
        if (setupcontrols[control][0]==ch)
            found = 0;
        else
        if (setupcontrols[control][1]==ch)
            found = 1;
        if (found>=0)
        {
            // replace mouse and joy clicks by double clicks
            // FIXME: first mouse only
            if (ch>=KEY_MOUSE1 && ch<=KEY_MOUSE1+MOUSEBUTTONS)
                setupcontrols[control][found] = ch-KEY_MOUSE1+KEY_DBLMOUSE1;
#ifdef DBL_JOY_BUTTONS
            // joystick doubleclicks
            // FIXME: JOY0 only
            else
//              if (ch>=KEY_JOY0BUT0 && ch<=(KEY_JOYLAST))  // all JOY
              if (ch>=KEY_JOY0BUT0 && ch<=(KEY_JOY0BUT0+JOYBUTTONS))  // one JOY
                setupcontrols[control][found] = ch + (KEY_DBLJOY0BUT0 - KEY_JOY0BUT0);
#endif
        }
        else
        {
            // check if change key1 or key2, or replace the two by the new
            found = 0;
            if (setupcontrols[control][0] == KEY_NULL)
                found++;
            if (setupcontrols[control][1] == KEY_NULL)
                found++;
            if (found==2)
            {
                found = 0;
                setupcontrols[control][1] = KEY_NULL;  //replace key 1 ,clear key2
            }
            G_CheckDoubleUsage(ch);
            setupcontrols[control][found] = ch;
        }

    }

    M_StopMessage(0);
}

void M_ChangeControl(int choice)
{
    controltochange = currentMenu->menuitems[choice].alphaKey;
    snprintf (msgtmp, MSGTMP_LEN,
              "Hit the new key for\n%s\nESC for Cancel", currentMenu->menuitems[choice].text);
    msgtmp[MSGTMP_LEN-1] = '\0';

    M_StartMessage (msgtmp, M_ChangecontrolResponse, MM_EVENTHANDLER);
}

//===========================================================================
//                        VIDEO MODE MENU
//===========================================================================
void M_DrawVideoMode(void);             //added:30-01-98:

void M_HandleVideoMode (int ch);

menuitem_t VideoModeMenu[]=
{
    {IT_KEYHANDLER | IT_NOTHING, 0, "", M_HandleVideoMode, '\0'},     // dummy menuitem for the control func
};


menu_t  VidModeDef =
{
    "M_VIDEO", // in legacy.wad
    "Video Mode",
    1,                  // # of menu items
    //sizeof(VideoModeMenu)/sizeof(menuitem_t),
    &VideoOptionsDef,   // previous menu
    VideoModeMenu,      // menuitem_t ->
    M_DrawVideoMode,    // drawing routine ->
    48,36,              // x,y
    0                   // lastOn
};

//added:30-01-98:
#define MAXCOLUMNMODES   10     //max modes displayed in one column
#define MAXMODEDESCS     (MAXCOLUMNMODES*3)

// shhh... what am I doing... nooooo!
static int vidm_testing_cnt=0;
static modenum_t  vidm_previousmode;  // modenum in format of setmodeneeded
static int vidm_current=0;  // modedesc index
static int vidm_nummodes;
static int vidm_column_size;

typedef struct
{
    modenum_t  modenum; // video mode number in format of setmodeneeded
    char    *desc;      //XXXxYYY
    int     iscur;      //1 if it is the current active mode
} modedesc_t;

static modedesc_t   modedescs[MAXMODEDESCS];


//
// Draw the video modes list, a-la-Quake
//
void M_DrawVideoMode(void)
{
    modenum_t  mode_320x200 = VID_GetModeForSize( 320, 200, MODE_fullscreen );
    byte  base_modetype = (mode_fullscreen) ? MODE_fullscreen : MODE_window;
    range_t moderange;
    modenum_t  dmode;  // draw modenum
    modedesc_t * mdp;  // modedesc
    int     i,j,dup,row,col;
    char    *desc;
    char    temp[80];

    // draw title
    M_DrawMenuTitle();

    dmode.modetype = base_modetype;
    vidm_nummodes = 0;
    moderange = VID_ModeRange( base_modetype );   // indexing
    for (i=moderange.first ; i<=moderange.last ; i++)
    {
        dmode.index = i;
        desc = VID_GetModeName (dmode);
        if (desc)
        {
            dup = 0;

            //when a resolution exists both under VGA and VESA, keep the
            // VESA mode, which is always a higher modenum
            for (j=0 ; j<vidm_nummodes ; j++)
            {
                mdp = & modedescs[j];
                if (!strcmp (mdp->desc, desc))
                {
                    // 320x200 fullscreen is always standard VGA, not vesa
                    if (mdp->modenum.modetype != mode_320x200.modetype
                        || mdp->modenum.index != mode_320x200.index)
                    {
                        // replace previous entry (VGA)
                        mdp->modenum = dmode;
                        mdp->iscur = (dmode.modetype == vid.modenum.modetype
                                      && dmode.index == vid.modenum.index );
                    }
                    dup = 1;
                    break;
                }
            }

            if (!dup)
            {
                mdp = & modedescs[vidm_nummodes];
                mdp->desc = desc;
                mdp->modenum = dmode;
                mdp->iscur = (dmode.modetype == vid.modenum.modetype
                              && dmode.index == vid.modenum.index );

                vidm_nummodes++;
                if( vidm_nummodes >= MAXMODEDESCS )  break;
            }
        }
    }

    vidm_column_size = (vidm_nummodes+2) / 3;


    row = 16;
    col = VidModeDef.y;
    for(i=0; i<vidm_nummodes; i++)
    {
        V_DrawString (row, col, modedescs[i].iscur ? V_WHITEMAP : 0, modedescs[i].desc);

        col += 8;
        if((i % vidm_column_size) == (vidm_column_size-1))
        {
            row += 8*13;
            col = 36;
        }
    }

    if (vidm_testing_cnt>0)
    {
        sprintf(temp, "TESTING MODE %s", modedescs[vidm_current].desc );
        M_CentreText(VidModeDef.y+80+24, temp );
        M_CentreText(VidModeDef.y+90+24, "Please wait 5 seconds..." );
    }
    else
    {
        M_CentreText(VidModeDef.y+60+24,"Press ENTER to set mode");

        M_CentreText(VidModeDef.y+70+24,"T to test mode for 5 seconds");

        sprintf(temp, "D to make %s the default", VID_GetModeName(vid.modenum));
        M_CentreText(VidModeDef.y+80+24,temp);

        sprintf(temp, "Current default is %dx%d (%d bits)", cv_scr_width.value, cv_scr_height.value, cv_scr_depth.value);
        M_CentreText(VidModeDef.y+90+24,temp);

        M_CentreText(VidModeDef.y+100+24,"Press ESC to exit");
    }

// Draw the cursor for the VidMode menu
    if (skullAnimCounter<4)    //use the Skull anim counter to blink the cursor
    {
        i = 16 - 10 + ((vidm_current / vidm_column_size)*8*13);
        j = VidModeDef.y + ((vidm_current % vidm_column_size)*8);
        V_DrawCharacter( i, j, '*' | 0x80);  // white
    }
}


//added:30-01-98: special menuitem key handler for video mode list
void M_HandleVideoMode (int key)
{
    if (vidm_testing_cnt>0)
    {
       // change back to the previous mode quickly
       if (key==KEY_ESCAPE)
       {
           setmodeneeded = vidm_previousmode;
           vidm_testing_cnt = 0;
       }
       return;
    }

    switch( key )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL, menu_sfx_updown);
        vidm_current++;
        if (vidm_current>=vidm_nummodes)
            vidm_current = 0;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL, menu_sfx_updown);
        vidm_current--;
        if (vidm_current<0)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_LEFTARROW:
        S_StartSound(NULL, menu_sfx_val);
        vidm_current -= vidm_column_size;
        if (vidm_current<0)
            vidm_current = (vidm_column_size*3) + vidm_current;
        if (vidm_current>=vidm_nummodes)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_RIGHTARROW:
        S_StartSound(NULL, menu_sfx_val);
        vidm_current += vidm_column_size;
        if (vidm_current>=(vidm_column_size*3))
            vidm_current %= vidm_column_size;
        if (vidm_current>=vidm_nummodes)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_ENTER:
        S_StartSound(NULL, menu_sfx_enter);
        if ( setmodeneeded.modetype == MODE_NOP ) //in case the previous setmode was not finished
            setmodeneeded = modedescs[vidm_current].modenum;
        break;

      case KEY_ESCAPE:      //this one same as M_Responder
        S_StartSound(NULL, menu_sfx_esc);
        M_Setup_prevMenu();
        return;

      case 'T':
      case 't':
        S_StartSound(NULL, menu_sfx_action);
        vidm_testing_cnt = TICRATE*5;
        vidm_previousmode = vid.modenum;
        if ( setmodeneeded.modetype == MODE_NOP ) //in case the previous setmode was not finished
            setmodeneeded = modedescs[vidm_current].modenum;
        return;

      case 'D':
      case 'd':
        // current active mode becomes the default mode.
        S_StartSound(NULL, menu_sfx_action);
        SCR_SetDefaultMode ();
        return;

      default:
        break;
    }
}

#ifdef SAVEGAMEDIR
//===========================================================================
// GAME DIR MENU
//===========================================================================
void M_DrawDir(void);

void M_DirSelect(int choice);
void M_Get_SaveDir(int choice);
void M_DirEnter(int choice);

#define NUM_DIRLINE  6
menuitem_t LoadDirMenu[]=
{
    {IT_CALL | IT_NOTHING,"",0, M_DirEnter,'/'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'1'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'2'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'3'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'4'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'5'},
    {IT_CALL | IT_NOTHING,"",0, M_DirSelect,'6'}
};

menu_t  DirDef =
{
//    "M_LOADG",	// LOAD GAME, really need SELECT DIR
    NULL,
    "Game Directory",
    NUM_DIRLINE+1,
    &LoadDef,
    LoadDirMenu,
    M_DrawDir,
//    80,54,
    (176-(SAVELINELEN*8/2)), 54-LINEHEIGHT,
    0
};


// Draw the current DIR line above list
static void draw_dir_line( int line_y )
{
    V_DrawString( DirDef.x, line_y, 0, "DIR");
    M_DrawSaveLoadBorder( DirDef.x+32, line_y, 0);
    V_DrawString( DirDef.x+32, line_y, 0, savegamedir);
}

// Draw the dir list and DIR line
void M_DrawDir(void)
{
    int i;
    int line_y = DirDef.y;

    M_DrawGenericMenu();

    if (edit_enable)
    {
        // draw string and cursor in the edit position
        V_DrawString( DirDef.x, line_y, 0, "NEW DIR");
        int line_x = DirDef.x+64;
        M_DrawSaveLoadBorder( line_x, line_y, 0);
        V_DrawString( line_x, line_y, 0, edit_buffer);
        i = V_StringWidth(edit_buffer);
        V_DrawString( line_x + i, line_y, 0, "_");
        return;
    }

    // Draw non-edit directory listing
    draw_dir_line( line_y );
    for (i = 0; i < NUM_DIRLINE; i++)
    {
        line_y += LINEHEIGHT;
        M_DrawSaveLoadBorder( DirDef.x, line_y, 0);
        V_DrawString( DirDef.x, line_y, 0, savegamedisp[i].desc);
    }
    // Put some message in the UP-TO-LEGACY dir entry.
    // The actual dir name remains blank.
    if( scroll_index == 0 )
    {
        V_DrawString( DirDef.x, DirDef.y+LINEHEIGHT, 0, "..");
    }
}

void M_ReadSaveStrings( int scroll_direction );

// Called from DIR game menu to select a directory
void M_DirSelect(int choice)
{
    // LoadDirMenu: slots 0..5 are menu 1..6
    int sgslot = choice - 1;
    if( (scroll_index == 0 && choice == 1) // UP-TO-LEGACY dir
        || ( savegamedisp[sgslot].desc[0] != '\0' ) )  // existing dir
    {
        // Existing directory selected
        strcpy( savegamedir, savegamedisp[sgslot].desc );
        scroll_index = 0; // start at top of directory
        DirDef.prevMenu->lastOn = 1;
        M_Setup_prevMenu();
    }
    else if( slotindex )
    {
        // empty entry (other than UP entry), then make new directory
        M_DirEnter(0);  // calls back here, M_DirSelect( 1 )
    }
}

// Callback after editing new directory name, setup by M_DirEnter
void M_NewDir( void )
{
    char dirname[256];
   
    // normal savegame select, set savegamedir
    M_DirSelect( 1 ); // LoadDirMenu: slot=0 is menu 1
    if( savegamedir[0] )
    {
        // make new directory
        snprintf( dirname, 255, "%s%s", legacyhome, savegamedir );
        dirname[255] = '\0';
        I_mkdir( dirname, 0700 ); // octal permissions
    }
}


// Called from DIR game menu to select a directory
void M_DirEnter(int choice)
{
    slotindex = 0; // edit
    // initiate edit of dir string, we are going to be intercepting all chars
    edit_enable = 1;
    edit_buffer[0] = '\0';
    edit_index = 0;
    edit_done_callback = M_NewDir;
    // when done editing, will goto M_NewDir
}

void M_Dir_scroll (int amount);

// Called from DIR game menu to delete a directory
void M_Dir_delete (int ch)
{
    if( ch=='y' && savegamedisp[slotindex].desc[0] )
    {
        char dirname[256];
        // if is current directory
        if( strcmp( savegamedir, savegamedisp[slotindex].desc ) == 0 )
        {
            savegamedir[0] = '\0';
        }
        // remove directory
        snprintf( dirname, 255, "%s%s", legacyhome, savegamedisp[slotindex].desc );
        dirname[255] = '\0';
        remove( dirname );
        savegamedisp[slotindex].desc[0] = '\0';
    }
    // fixup after the message undo, which does not record callbacks
    M_StopMessage(0);
    scroll_callback = M_Dir_scroll;
    delete_callback = M_Dir_delete;
}


// [smite] MinGW compatibility
#ifndef WIN32
#define USE_FTW 
#endif

#ifdef USE_FTW
#include <ftw.h>
// Callback from ftw system call
int  ftw_directory_entry( const char *file, const struct stat * sb, int flag )
{
    if( flag == FTW_D )  // only want directories
    {
        if( slotindex >= 0 )  // because of dir list scrolling
        {
            // Only want the name after legacyhome
            strncpy( savegamedisp[slotindex].desc, &file[legacyhome_len], SAVESTRINGSIZE-1 );
            savegamedisp[slotindex].desc[SAVESTRINGSIZE-1] = '\0';
        }
        slotindex++;
    }
    if( slotindex >= NUM_DIRLINE )  return 1;  // done, stop ftw
    return 0;
}

#else

#include <sys/types.h>
#include <dirent.h>

#ifndef _DIRENT_HAVE_D_TYPE
#include <sys/stat.h>
#include "m_misc.h"
#endif

#endif

// Get directories into savegamedisp, starting at skip_count.
void  get_directory_entries( int skip_count )
{
#ifdef USE_FTW
    // Use ftw
    slotindex = -skip_count;
    ftw( legacyhome, ftw_directory_entry, 1 );
#else
    // Use dirent

#ifndef _DIRENT_HAVE_D_TYPE
    // Have to use stat to identify directories.
    char dentfile[MAX_WADPATH];
    struct stat dentstat;
#endif
   
    struct dirent * dent;
    DIR * legdir;

    legdir = opendir( legacyhome );
    if( legdir == NULL )  return;

    slotindex = -skip_count;
    for (;;)
    {
        dent = readdir( legdir );  // Read directory entry
        if( dent == NULL )  break;
        // Ignore the self reference.
        if( strcmp( dent->d_name, "." ) == 0 )   continue;
#ifdef _DIRENT_HAVE_D_TYPE
        // Unix systems have the D_TYPE, but others are unlikely.
        if( dent->d_type != DT_DIR )  continue;  // Only want directories
#else
        // Get status to check if is a directory.
        cat_filename( dentfile, legacyhome, dent->d_name );       
        stat( dentfile, &dentstat );
        if( ! S_ISDIR( dentstat.st_mode ))  continue;  // Only want directories
#endif

        if( slotindex >= 0 )  // because of dir list scrolling
        {
            // Only want the name after legacyhome
            strncpy( savegamedisp[slotindex].desc, dent->d_name, SAVESTRINGSIZE-1 );
            savegamedisp[slotindex].desc[SAVESTRINGSIZE-1] = '\0';
            // The up-dir is passed as an empty string.
            if( strcmp( savegamedisp[slotindex].desc, ".." ) == 0 )
                savegamedisp[slotindex].desc[0] = 0;
        }
        slotindex++;
        if( slotindex >= NUM_DIRLINE )  break;  // full
    }
    closedir( legdir );
#endif
}


void M_Dir_scroll (int amount)
{
    // Do not scroll if at end of list
    if( (amount > 0) && ( savegamedisp[SAVEGAME_NUM_MSLOT-1].desc[0] == '\0' ))
        return;  // at end of dir list

    clear_remaining_savegamedisp( 0 );
    // countdown reading dir entries
    scroll_index += amount;
    if( scroll_index < 0 )   scroll_index = 0;
    get_directory_entries( scroll_index );
}

// Called from menu
void M_Get_SaveDir (int choice)
{
    // Any mode, directory is personal choice
    // Directory menu with choices
    DirDef.prevMenu = currentMenu;	// return to Load or Save
    M_SetupNextMenu(&DirDef);
    scroll_callback = M_Dir_scroll;
    delete_callback = M_Dir_delete;

    clear_remaining_savegamedisp( 0 );
    scroll_index = 0;  // start at top of dir list
    get_directory_entries( 0 );
}
   
#endif

//===========================================================================
//LOAD GAME MENU
//===========================================================================
void M_DrawLoad(void);

void M_LoadSelect(int choice);

// SAVEGAME_NUM_MSLOT dependent
#ifdef SAVEGAMEDIR
#define SAVEGAME_MSLOT_0      1
#define SAVEGAME_MSLOT_LAST   6
#else
#define SAVEGAME_MSLOT_0      0
#define SAVEGAME_MSLOT_LAST   5
#endif


// Has SAVEGAME_NUM_MSLOT entries, and dir entry
menuitem_t LoadGameMenu[]=
{
#ifdef SAVEGAMEDIR
    {IT_CALL | IT_NOTHING,"",0, M_Get_SaveDir,'/'},
#endif   
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'1'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'2'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'3'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'4'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'5'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    "M_LOADG",
    "Load Game",
    sizeof(LoadGameMenu)/sizeof(menuitem_t),
    &MainDef,
    LoadGameMenu,
    M_DrawLoad,
//    80,54,
#ifdef SAVEGAMEDIR
    (176-(SAVELINELEN*8/2)),54-LINEHEIGHT,
#else
    (176-(SAVELINELEN*8/2)),54,
#endif   
    0
};

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int i;
    int line_y = LoadDef.y;

    M_DrawGenericMenu();

#ifdef SAVEGAMEDIR
    draw_dir_line( line_y );
    for (i = 0; i < SAVEGAME_NUM_MSLOT; i++)
    {
        line_y += LINEHEIGHT;
        M_DrawSaveLoadBorder( LoadDef.x, line_y, 1);
#ifdef SAVEGAME_MTLEFT
        V_DrawString( LoadDef.x, line_y, 0, savegamedisp[i].levtime);
        V_DrawString( LoadDef.x+SAVE_DESC_XPOS, line_y, 0, savegamedisp[i].desc);
#else
        V_DrawString( LoadDef.x, line_y, 0, savegamedisp[i].desc);
        V_DrawString( LoadDef.x+(SAVE_MT_POS*8), line_y, 0, savegamedisp[i].levtime);
#endif
    }
#else
    for (i = 0; i < SAVEGAME_NUM_MSLOT; i++)
    {
        M_DrawSaveLoadBorder( LoadDef.x, line_y, 0);
#ifdef SAVEGAME_MTLEFT
        V_DrawString( LoadDef.x, line_y, 0, savegamedisp[i].levtime);
        V_DrawString( LoadDef.x+SAVE_DESC_XPOS, line_y, 0, savegamedisp[i].desc);
#else
        V_DrawString( LoadDef.x, line_y, 0, savegamedisp[i].desc);
        V_DrawString( LoadDef.x+(SAVE_MT_POS*8), line_y, 0, savegamedisp[i].levtime);
#endif
        line_y += LINEHEIGHT;
    }
#endif
}

//
// User wants to load this game
//
// Called from load game menu to load selected save game
void M_LoadSelect(int choice)
{
    // Issue command to save game
    // SAVEGAMEDIR: slots 0..5 are menu 1..6
    short sgslot = choice - SAVEGAME_MSLOT_0;
#ifdef SAVEGAME99
    if( savegamedisp[sgslot].savegameid <= 99 )
      G_LoadGame ( savegamedisp[sgslot].savegameid );  // slot id
#else
    G_LoadGame (sgslot);
#endif
    M_ClearMenus (true);
}


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//  and put it in savegame global variable
//
#ifdef SAVEGAME99
void M_ReadSaveStrings( int scroll_direction )
{
    // [WDJ] saves considerable size and hassle having this test here
    boolean skip_unloadable = (currentMenu != &SaveDef);
    int     sgslot, nameid, slot_status, i;
    int     first_nameid = 0;
    int     last_nameid = 0;  // disable unless searching
    int     handle;
    char  * slot_str;
    savegame_disp_t *sgdp;
    savegame_info_t  sginfo;
    char    name[256];

    P_Alloc_savebuffer( 0 );  // header only
    // savegamedisp is statically alloc

    if( scroll_direction < 0 )
    {
        // Because unused slots are skipped, cannot predict what id will be
        // at top when paging backwards, so must start from 0 and read
        // forward (while scrolling) until the last_nameid test is statisfied.
        // The top of previous display will be the last item in next display.
        if((scroll_index > 0) && (savegamedisp[0].savegameid <= 99))
           last_nameid = savegamedisp[0].savegameid;
        scroll_index = first_nameid = 0;
    }
    else if( scroll_direction > 0 )
    {
        // The bottom of previous display becomes top of next display.
        if( savegamedisp[SAVEGAME_NUM_MSLOT-1].savegameid <= 99 )
           first_nameid = savegamedisp[SAVEGAME_NUM_MSLOT-1].savegameid;
        else if( savegamedisp[0].savegameid <= 99 )
           first_nameid = savegamedisp[0].savegameid;
        scroll_index = first_nameid;
    }
    else
    {
        // no scroll
        if( scroll_index >= 0 && scroll_index <= 99 )
        {
            // redisplay from last usage
            first_nameid = scroll_index;
        }
    }

    // read savegame headers into savegame slots 0..5
    sgslot = 0;
    for (nameid = first_nameid; nameid <= 99; nameid++)
    {
        if( sgslot >= SAVEGAME_NUM_MSLOT ) break;
        sgdp = &savegamedisp[sgslot];
        sgdp->levtime[0] = '\0';

        G_Savegame_Name( name, nameid );

        handle = open (name, O_RDONLY | 0, 0666);
        if (handle == -1)
        {
            // read error
            if( skip_unloadable )  continue;
            slot_str = EMPTYSTRING;
            sprintf( &sgdp->levtime[0], "%2i", nameid );
            slot_status = IT_SPACE | IT_NOTHING;
        }
        else
        {
            // read the savegame header and react
            read( handle, savebuffer, savebuffer_size );
            close (handle);
            if( P_Read_Savegame_Header( &sginfo ) )
            {
                if( sginfo.map == NULL ) sginfo.map = " -  ";
                if( sginfo.levtime == NULL ) sginfo.levtime = "";
                // info from a valid legacy save game
                snprintf( &sgdp->levtime[0], SAVEGAME_MTLEN,
                          "%s %s", sginfo.map, sginfo.levtime);
                sgdp->levtime[SAVEGAME_MTLEN-1] = '\0';
                slot_str = sginfo.name;
                slot_status = IT_CALL | IT_NOTHING | 1;
            }
            else
            {
                // bad header, not a valid legacy savegame, or an old one
                if( skip_unloadable )  continue;
                slot_str = sginfo.msg;	// error message
                slot_status = IT_SPACE | IT_NOTHING;
            }
        }
        // fill in savegame strings for menu display
        strncpy( &sgdp->desc[0], slot_str, SAVESTRINGSIZE );
        sgdp->desc[SAVESTRINGSIZE-1] = '\0';
        sgdp->levtime[SAVEGAME_MTLEN-1] = '\0';
        sgdp->savegameid = nameid;
        LoadGameMenu[sgslot + SAVEGAME_MSLOT_0].status = slot_status;
        sgslot++; // uses savegamedisp[0..5]
        // When scroll_direction < 0 only, until last test satisfied
        if((sgslot >= SAVEGAME_NUM_MSLOT) && (nameid < last_nameid))
        {
            // Display is full and still searching for last_nameid.
            // Scroll them to make room at last slot for next read

            // [WDJ] Fix bug: upon scroll down and up, EMPTY SLOT message remained
            // Must scroll the status too.
            for( i = SAVEGAME_MSLOT_0; i<SAVEGAME_MSLOT_LAST; i++ )
               LoadGameMenu[i].status = LoadGameMenu[i+1].status;
            LoadGameMenu[SAVEGAME_MSLOT_LAST].status = IT_SPACE | IT_NOTHING;

            memmove( &savegamedisp[0], &savegamedisp[1],
                         sizeof( savegame_disp_t ) * (SAVEGAME_NUM_MSLOT-1));
            sgslot --;  // read one more, come back here
            scroll_index = savegamedisp[0].savegameid;
        }
    }
    free( savebuffer );

    clear_remaining_savegamedisp( sgslot );  // if sgslot < 5, upto [5]
    // clear remaining menu slot status, to prevent attempts to load
    for( i = sgslot+SAVEGAME_MSLOT_0; i<=SAVEGAME_MSLOT_LAST; i++ )
       LoadGameMenu[i].status = IT_SPACE | IT_NOTHING;
}

#else

void M_ReadSaveStrings(void)
{
    int     handle;
    int     i;
    savegame_disp_t *sgdp;
    savegame_info_t  sginfo;
    char    name[256];

    P_Alloc_savebuffer( 0 );  // header only
    // savegamedisp is statically alloc

    for (i = 0; i < SAVEGAME_NUM_MSLOT; i++)
    {
        sgdp = &savegamedisp[i];
        sgdp->levtime[0] = '\0';

        G_Savegame_Name( name, i );

        handle = open (name, O_RDONLY | 0, 0666);
        if (handle == -1)
        {
            // read error
            strcpy(&sgdp->desc[0], EMPTYSTRING);
            LoadGameMenu[i].status = IT_SPACE | IT_NOTHING;
            continue;
        }
        read( handle, savebuffer, savebuffer_size );
        close (handle);
        if( P_Read_Savegame_Header( &sginfo ) )
        {
            // info from a valid legacy save game
            strncpy( &sgdp->desc[0], sginfo.name, SAVESTRINGSIZE );
            if( sginfo.map == NULL ) sginfo.map = " -  ";
            if( sginfo.levtime == NULL ) sginfo.levtime = "";
            snprintf( &sgdp->levtime[0], SAVEGAME_MTLEN,
                      "%s %s", sginfo.map, sginfo.levtime);
            sgdp->levtime[SAVEGAME_MTLEN-1] = '\0';
            LoadGameMenu[i].status = IT_CALL | IT_NOTHING | 1;
        }
        else
        {
            strncpy( &sgdp->desc[0], sginfo.msg, SAVESTRINGSIZE );
            LoadGameMenu[i].status = IT_SPACE | IT_NOTHING;
        }
        sgdp->desc[SAVESTRINGSIZE-1] = '\0';
    }
    free( savebuffer );
}
#endif


#ifdef SAVEGAME99
// scroll_callback
void M_Savegame_scroll (int amount)
{
    M_ReadSaveStrings( amount ); // skip unloadable
}

//void M_Save_scroll (int amount);

// delete_callback
void M_Savegame_delete (int ch)
{
    if( ch=='y' && (savegamedisp[slotindex].savegameid <= 99) )
    {
        char savename[256];
        G_Savegame_Name( savename, savegamedisp[slotindex].savegameid );
        // remove savegame
        remove( savename );
        savegamedisp[slotindex].desc[0] = '\0';
        savegamedisp[slotindex].levtime[0] = '\0';
        savegamedisp[slotindex].savegameid = 255;
        // slot no longer loadable
        LoadGameMenu[slotindex + SAVEGAME_MSLOT_0].status = IT_SPACE | IT_NOTHING;
    }
    // fixup after the message undo, which does not record callbacks
    M_StopMessage(0);
    scroll_callback = M_Savegame_scroll;
    delete_callback = M_Savegame_delete;
}
#endif

//
// Selected from DOOM menu
//
// Called from menu, and key F3
void M_LoadGame (int choice)
{
// change can't load message to can't load in server mode
    if (netgame && !server)
    {
        // running network game and am not the server, cannot load
        M_SimpleMessage(LOADNET);
        return;
    }

    // Save game load menu with slot choices
    M_SetupNextMenu(&LoadDef);
#ifdef SAVEGAME99
    scroll_callback = M_Savegame_scroll;
    delete_callback = M_Savegame_delete;
    M_ReadSaveStrings( 0 ); // skip unloadable
#else
    M_ReadSaveStrings();
#endif
}


//===========================================================================
//                                SAVE GAME MENU
//===========================================================================
void M_DrawSave(void);

void M_SaveSelect(int choice);

// Must have SAVEGAME_NUM_MSLOT entries, plus dir
menuitem_t SaveGameMenu[]=
{
#ifdef SAVEGAMEDIR
    {IT_CALL | IT_NOTHING,"",0, M_Get_SaveDir,'/'},
#endif   
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'1'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'2'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'3'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'4'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'5'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    "M_SAVEG",
    "Save Game",
    sizeof(SaveGameMenu)/sizeof(menuitem_t),
    &MainDef,
    SaveGameMenu,
    M_DrawSave,
//    80,54,
#ifdef SAVEGAMEDIR
    (176-(SAVELINELEN*8/2)),54-LINEHEIGHT,
#else
    (176-(SAVELINELEN*8/2)),54,
#endif   
    0
};



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y, boolean longer )
{
    int i;

    // Draw to screen0, scaled
    if( gamemode == heretic )
    {
#ifdef SAVEGAME_MTLEFT
        V_DrawScaledPatch_Name(x-8, y-4, "M_FSLOT");
#if SAVELINELEN > 24
        if( longer )
        { 
            V_DrawScaledPatch_Name(x-8 + SAVE_DESC_XPOS, y-4, "M_FSLOT");
        }
#endif
#else
#if SAVELINELEN > 24
        if( longer )
        {
            V_DrawScaledPatch_Name(x-8 + ((SAVELINELEN-24)*8), y-4, "M_FSLOT");
        }
#endif
        V_DrawScaledPatch_Name(x-8, y-4, "M_FSLOT");
#endif
    }
    else
    {
        V_DrawScaledPatch_Name (x-8,y+7, "M_LSLEFT");
        
        for (i = (longer?SAVELINELEN:SAVESTRINGSIZE); i>0; i--)
        {
            V_DrawScaledPatch_Name (x,y+7, "M_LSCNTR");
            x += 8;
        }
        
        V_DrawScaledPatch_Name (x,y+7, "M_LSRGHT");
    }
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int line_y = LoadDef.y;

    if (edit_enable)
    {
//        M_DrawLoad();	// optional, keep other slots displayed
        // draw string and cursor in the original slot position
#ifdef SAVEGAMEDIR
        line_y = LoadDef.y+(LINEHEIGHT*slotindex)+LINEHEIGHT; // dir is 0
#else
        line_y = LoadDef.y+LINEHEIGHT*slotindex;
#endif
        M_DrawSaveLoadBorder( LoadDef.x, line_y, 1);
#ifdef SAVEGAME_MTLEFT
        V_DrawString( LoadDef.x, line_y, 0, "DESCRIPTION:");
        V_DrawString( LoadDef.x+SAVE_DESC_XPOS, line_y, 0, edit_buffer);
        int i = V_StringWidth(edit_buffer);
        V_DrawString( LoadDef.x+SAVE_DESC_XPOS + i, line_y, 0, "_");
#else
        V_DrawString( LoadDef.x, line_y, 0, edit_buffer);
        int i = V_StringWidth(edit_buffer);
        V_DrawString( LoadDef.x + i, line_y, 0, "_");
#endif
    }
    else
    {
        M_DrawLoad();
    }
}

//
// M_Responder calls this when user is finished
//
// Called from save menu by M_Responder,
// and from quick Save by M_QuickSaveResponse
#if defined SAVEGAMEDIR || defined SAVEGAME99
// slti = savegame index 0..5, or quicksave 6
void M_DoSave(int slti)
{
    if( savegamedisp[slti].savegameid > 99 )
        return;
    // Issue command to save game
    G_SaveGame (savegamedisp[slti].savegameid, savegamedisp[slti].desc);
    M_ClearMenus (true);

    // PICK QUICKSAVE SLOT YET?
    if (quicksave_slotid == -2)
    {
        quicksave_slotid = savegamedisp[slti].savegameid;  // 0..99
        savegamedisp[QUICKSAVE_INDEX] = savegamedisp[slti];  // save whole thing
    }
}
#else
// slot = game id and menu index 0..5
void M_DoSave(int slot)
{
    // Issue command to save game
    G_SaveGame (slot, savegamedisp[slot].desc);
    M_ClearMenus (true);

    // PICK QUICKSAVE SLOT YET?
    if (quicksave_slotid == -2)
    {
        quicksave_slotid = slot;
    }
}
#endif



// Called when desc editing is done
void M_SaveEditDone( void )
{
    M_DoSave(slotindex);  // index 0..5
}

//
// User wants to save. Start string input for M_Responder
//
// Called from save game menu to select save game
void M_SaveSelect(int choice)
{
#ifdef SAVEGAMEDIR   
    slotindex = choice - SAVEGAME_MSLOT_0; // menu 1..6 -> index 0..5
#else   
    slotindex = choice;  // line being edited  0..5
#endif
    if( savegamedisp[slotindex].savegameid > 99 )
        return;
    // clear out EMPTY STRING and other err msgs
    // LoadSaveStrings puts existing status in LoadGameMenu, MSLOT index
    if ( (LoadGameMenu[choice].status & 1) != 1 )  // invalid name
        savegamedisp[slotindex].desc[0] = 0;
    // [WDJ] edit_enable overwrites entire line
    // initiate edit of desc string, we are going to be intercepting all chars
    strcpy(edit_buffer, savegamedisp[slotindex].desc);
    edit_index = strlen(edit_buffer);
    edit_done_callback = M_SaveEditDone;
    edit_enable = 1;
}


//
// Selected from DOOM menu
//
// Called from menu, and key F2, and quicksave
void M_SaveGame (int choice)
{
    if(demorecording)
    {
        M_SimpleMessage("You cannot save while recording demos\n\nPress a key\n");
        return;
    }

    if (demoplayback || demorecording)
    {
        M_SimpleMessage(SAVEDEAD);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (netgame && !server)
    {
        M_SimpleMessage("You are not the server");
        return;
    }

    // Save game menu with slot choices
    M_SetupNextMenu(&SaveDef);
#ifdef SAVEGAME99
    delete_callback = M_Savegame_delete;
    scroll_callback = M_Savegame_scroll;
    M_ReadSaveStrings( 0 ); // show unloadable
#else
    M_ReadSaveStrings();
#endif
}

//===========================================================================
//                            QuickSAVE & QuickLOAD
//===========================================================================

//
// M_QuickSave
//

// Handles quick save ack from M_QuickSave, and initiates the save.
void M_QuickSaveResponse(int ch)
{
    if (ch == 'y')
    {
        M_DoSave( QUICKSAVE_INDEX ); // initiate game save, network message
        S_StartSound(NULL, menu_sfx_action);
    }
    else
    {
        // response was "No"
        // Give opportunity to pick a new slot
        quicksave_slotid = -2;     // means to pick a slot now
        M_SaveGame( -2 );
    }
}

// Invoked by key F6
void M_QuickSave(void)
{
    if (demoplayback || demorecording)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quicksave_slotid < 0)   goto pick_slot; // No slot yet.
    // Show save name, ask for quick save ack.
    snprintf(msgtmp, MSGTMP_LEN, QSPROMPT, savegamedisp[QUICKSAVE_INDEX].desc);
    msgtmp[MSGTMP_LEN-1] = '\0';
    M_StartMessage(msgtmp, M_QuickSaveResponse, MM_YESNO);
    return;

pick_slot:   
    // have not selected a quick save slot yet
    M_StartControlPanel();
    quicksave_slotid = -2;     // signal to save as a quicksave slot
    M_SaveGame( -2 );
    return;
}



//
// M_QuickLoad
//
// Handles quick load ack from M_QuickLoad, and initiates the save.
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y')
    {
        // quicksave_slotid is known valid, slot id
        G_LoadGame( quicksave_slotid ); // initiate game load, network message
        M_ClearMenus (true);
        S_StartSound(NULL, menu_sfx_action);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_SimpleMessage(QLOADNET);
        return;
    }

    if (quicksave_slotid < 0)
    {
        // No save slot selected
        M_SimpleMessage(QSAVESPOT);
        return;
    }
    // Show load name, ask for quick load ack.
    snprintf(msgtmp, MSGTMP_LEN, QLPROMPT, savegamedisp[QUICKSAVE_INDEX].desc);
    msgtmp[MSGTMP_LEN-1] = '\0';
    M_StartMessage(msgtmp, M_QuickLoadResponse, MM_YESNO);
}


//===========================================================================
//                                 END GAME
//===========================================================================

//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
    if (ch != 'y')
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (true);
    COM_BufAddText("exitgame\n");
}

void M_EndGame(int choice)
{
    choice = 0;
    if (demoplayback || demorecording)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }
/*
    if (netgame)
    {
        M_SimpleMessage(NETEND);
        return;
    }
*/
    M_StartMessage(ENDGAME, M_EndGameResponse, MM_YESNO);
}

//===========================================================================
//                                 Quit Game
//===========================================================================

//
// M_QuitDOOM
//
int     quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int     quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};



void M_QuitResponse(int ch)
{
    tic_t   dlyd_time;
    if (ch != 'y')
        return;

    if (!netgame)
    {
#ifdef USE_QUITSOUNDS2
        //added:12-02-98: quitsounds are much more fun than quisounds2
        if (gamemode == doom2_commercial)
            S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
        else
#endif
            S_StartSound(NULL,quitsounds[(gametic>>2)&7]);

        //added:12-02-98: do that instead of I_WaitVbl which does not work
        if(!nosoundfx)
        {
            dlyd_time = I_GetTime() + TICRATE*2;
            while (dlyd_time > I_GetTime()) ;
        }
    }
    I_Quit();  // No return
}




void M_QuitDOOM(int choice)
{
  // We pick index 0 which is language sensitive,
  //  or one at random, between 1 and maximum number.
  snprintf(msgtmp, MSGTMP_LEN,
           text[DOSY_NUM], text[ QUITMSG_NUM+(gametic%NUM_QUITMESSAGES)]);
  msgtmp[MSGTMP_LEN-1] = '\0';
  M_StartMessage( msgtmp, M_QuitResponse, MM_YESNO);
}


//===========================================================================
//                              Some Draw routine
//===========================================================================

//
//      Menu Functions
//
void M_DrawThermo ( int   x,
                    int   y,
                    consvar_t *cv)
{
    int xx,i, cursory;
    int leftlump,rightlump,centerlump[2],cursorlump;

    // Draw to screen0, scaled
    xx = x;
    if( raven_heretic_hexen )
    {
        xx -= 32-8;
        leftlump      = W_GetNumForName("M_SLDLT");
        rightlump     = W_GetNumForName("M_SLDRT"); 
        centerlump[0] = W_GetNumForName("M_SLDMD1"); 
        centerlump[1] = W_GetNumForName("M_SLDMD2"); 
        cursorlump    = W_GetNumForName("M_SLDKB");  
        cursory = y+7;
    }
    else
    {
        leftlump      = W_GetNumForName("M_THERML");
        rightlump     = W_GetNumForName("M_THERMR"); 
        centerlump[0] = W_GetNumForName("M_THERMM"); 
        centerlump[1] = W_GetNumForName("M_THERMM"); 
        cursorlump    = W_GetNumForName("M_THERMO");  
        cursory = y;
    }
    { // temp use of left thermo patch
      patch_t *pt = W_CachePatchNum(leftlump,PU_CACHE);  // endian fix
      V_DrawScaledPatch (xx,y,pt);
      xx += pt->width - pt->leftoffset;  // add width to offset
    }
    for (i=0;i<16;i++)
    {
        // alternate center patches (raven_heretic_hexen)
        V_DrawScaledPatch_Num (xx,y, centerlump[i & 1] );
        xx += 8;
    }
    V_DrawScaledPatch_Num (xx,y, rightlump );

    xx = (cv->value - cv->PossibleValue[0].value) * (15*8) /
         (cv->PossibleValue[1].value - cv->PossibleValue[0].value);

    V_DrawScaledPatch_Num ((x+8) + xx, cursory, cursorlump );
}


void M_DrawEmptyCell( menu_t*       menu,
                      int           item )
{
    V_DrawScaledPatch_Name (menu->x - 10,  menu->y+item*LINEHEIGHT - 1,
                       "M_CELL1" );
}

void M_DrawSelCell ( menu_t*       menu,
                     int           item )
{
    V_DrawScaledPatch_Name (menu->x - 10,  menu->y+item*LINEHEIGHT - 1,
                       "M_CELL2" );
}


//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
//added:06-02-98:
//  x, y : position (320,200)
void M_DrawTextBox (int x, int y, int width, int lines)
{
    fontinfo_t * fip = V_FontInfo();
    patch_t  *p;
    int      cx, cy;
    int      n;
    int      step,boff;

    // Draw to screen0, scaled
    V_SetupDraw( 0 | V_SCALESTART | V_SCALEPATCH | V_CENTERHORZ );

    if( gamemode == heretic )
    {
        // humf.. border will stand if we do not adjust size ...
        x+=4;
        y+=4;
        lines = (lines+1)/2;
        width = (width+1)/2;
        step = 16;
        boff = 4; // borderoffset
    }
    else
    {
        step = fip->yinc;
        boff = 8;
    }
    if( ! viewborderlump[0] )   goto grey_bar;

    // draw left side
    cx = x;
    cy = y;
    V_DrawScaledPatch_Num (cx, cy, viewborderlump[BRDR_TL] );
    cy += boff;
   
    // temp use patch in loop
    p = W_CachePatchNum (viewborderlump[BRDR_L],PU_CACHE);  // endian fix
    for (n = 0; n < lines; n++)
    {
        V_DrawScaledPatch (cx, cy, p);
        cy += step;
    }
   
    V_DrawScaledPatch_Num (cx, cy, viewborderlump[BRDR_BL] );

    // draw background
    // Flat fill per drawinfo, centering.
    // Reduce scale of Doom background to (0..2) so text is easier to read.
    V_DrawFlatFill (x+boff, y+boff, width*step, lines*step,
                   (raven_heretic_hexen ? 15:0), st_borderflat_num);

    // draw top and bottom
    cx += boff;
    cy = y;
    while (width > 0)
    {
        V_DrawScaledPatch_Num (cx, cy, viewborderlump[BRDR_T] );

        V_DrawScaledPatch_Num (cx, y+boff+lines*step, viewborderlump[BRDR_B] );
        width --;
        cx += step;
    }

    // draw right side
    cy = y;
    V_DrawScaledPatch_Num (cx, cy, viewborderlump[BRDR_TR] );
    cy += boff;
   
    // temp use patch in loop
    p = W_CachePatchNum (viewborderlump[BRDR_R],PU_CACHE);  // endian fix
    for (n = 0; n < lines; n++)
    {
        V_DrawScaledPatch (cx, cy, p);
        cy += step;
    }
   
    V_DrawScaledPatch_Num (cx, cy, viewborderlump[BRDR_BR] );
    

  done:   
    V_SetupDraw( drawinfo.prev_screenflags );  // restore
   
    return;

  grey_bar:
    // Message box centers string, using 8 bit width assumption.
    // For now, correct the position.
//    V_DrawFill(x,y, width*fip->xinc, lines*fip->yinc, ci_grey );
    V_DrawFill(x/8, y, width*fip->xinc, lines*fip->yinc, ci_grey );
    goto done;
}

//==========================================================================
//                        Message is now a (hackable) Menu
//==========================================================================
void M_DrawMessageMenu(void);

menuitem_t MessageMenu[]=
{
    // TO HACK
    {0 ,NULL , 0, NULL ,0}
};

menu_t MessageDef =
{
    NULL,               // title
    NULL,
    1,                  // # of menu items
    NULL,               // previous menu       (TO HACK)
    MessageMenu,        // menuitem_t ->
    M_DrawMessageMenu,  // drawing routine ->
    0,0,                // x,y                 (TO HACK)
    0                   // lastOn, flags       (TO HACK)
};


void M_StartMessage ( const char*       string,
                      void*             routine,
                      menumessagetype_t itemtype )
{
    int   maxlen, i, lines;
    char * chp;
   
    msgtmp[MSGTMP_LEN] = '\0';  // make sure it is terminated
#define message MessageDef.menuitems[0].text
    if( message )
        Z_Free( message );
    message = Z_StrDup(string);
    DEBFILE(message);

    M_StartControlPanel(); // can't put menuactiv to true
    MessageDef.prevMenu = currentMenu;
    MessageDef.menuitems[0].text     = message;
    MessageDef.menuitems[0].alphaKey = itemtype;
    switch(itemtype) {
        case MM_NOTHING:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = M_StopMessage;
             break;
        case MM_YESNO:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = routine;
             break;
        case MM_EVENTHANDLER:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = routine;
             break;
    }
    //added:06-02-98: now draw a textbox around the message
    // compute length max and the numbers of lines
    maxlen = 4; // minimum box
    chp = message;
    for (lines=0;  ; lines++)
    {
        for (i = 0;  ; i++)
        {
            if (*chp == 0)   // end of line escape
                break;

            if (*chp == '\n')
            {
                chp ++;
                break;
            }
            chp ++;
        }
        if (i > maxlen)
            maxlen = i;
        if (*chp == 0)   // end of line counts
            break;
    }
    if((i > 0) || (lines==0))  // missing \n or empty string
        lines++;  // count as a line

    MessageDef.x=(BASEVIDWIDTH - (8*maxlen) - 16)/2;
    MessageDef.y=(BASEVIDHEIGHT - M_StringHeight(message))/2;

    MessageDef.lastOn = (lines<<8)+maxlen;

//    M_SetupNextMenu();
    currentMenu = &MessageDef;
    itemOn=0;
}

void M_SimpleMessage ( const char * string )
{
    M_StartMessage ( string, NULL, MM_NOTHING );
}


#define MAXMSGLINELEN 256

void M_DrawMessageMenu(void)
{
    int    y;
    short  i,max;
    char   string[MAXMSGLINELEN];
    int    start,lines;
    char   *msg=currentMenu->menuitems[0].text;

    // Draw to screen0, scaled
    y=currentMenu->y;
    start = 0;
    lines = currentMenu->lastOn>>8;
    max = (currentMenu->lastOn & 0xFF)*8;
    M_DrawTextBox (currentMenu->x,y-8,(max+7)>>3,lines);

    while(*(msg+start))
    {
        for (i = 0; i < (int)strlen(msg+start); i++)
        {
            if (*(msg+start+i) == '\n')
            {
                memset(string,0,MAXMSGLINELEN);
                if(i >= MAXMSGLINELEN)
                {
                    CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
                    return;
                }
                else
                {
                    strncpy(string,msg+start,i);
                    start += i+1;
                    i = -1; //added:07-02-98:damned!
                }
                
                break;
            }
        }

        if (i == (int)strlen(msg+start))
        {
            if(i >= MAXMSGLINELEN)
            {
                CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
                return;
            }
            else
            {
                strcpy(string,msg+start);
                start += i;
            }
        }

        V_DrawString((BASEVIDWIDTH - V_StringWidth(string))/2,y,0,string);
        y += 8; //hu_font[0]->height;
    }
}

// default message handler
void M_StopMessage(int choice)
{
    // Do not interfere with response menu changes
    if( currentMenu == &MessageDef )
    {
         M_SetupNextMenu(MessageDef.prevMenu); // NULLS callbacks, caller must fix
//         M_Setup_prevMenu();  // A little too much re-setup
         S_StartSound(NULL, menu_sfx_action);
    }
}

//==========================================================================
//                        Menu stuffs
//==========================================================================

//added:30-01-98:
//
//  Write a string centered using the hu_font
//
void M_CentreText (int y, char* string)
{
    int x;
    //added:02-02-98:centre on 320, because default option is SCALED
    x = (BASEVIDWIDTH - V_StringWidth(string))>>1;
    V_DrawString(x,y,0,string);
}


//
// CONTROL PANEL
//

void M_ChangeCvar(int choice)
{
    consvar_t *cv=(consvar_t *)currentMenu->menuitems[itemOn].itemaction;

    if(((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER )
     ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD  ))
    {
        CV_SetValue(cv,cv->value+choice*2-1);
    }
    else
    {
        if(cv->flags & CV_FLOAT)
        {
            char s[20];
            sprintf(s,"%f",(float)cv->value/FRACUNIT+(choice*2-1)*(1.0/16.0));
            CV_Set(cv,s);
        }
        else
            CV_AddValue(cv,choice*2-1);
    }
}

#define MAX_CVAR_STRING  512

static boolean M_ChangeStringCvar(int key, char ch)
{
    consvar_t *cv=(consvar_t *)currentMenu->menuitems[itemOn].itemaction;
    char buf[ MAX_CVAR_STRING + 1 ];
    int  len;

    switch (key)
    {
      case KEY_BACKSPACE :
        len=strlen(cv->string);
        if( len > MAX_CVAR_STRING )  len = MAX_CVAR_STRING;
        if( len>0 )
        {
            memcpy(buf,cv->string,len);
            buf[len-1]=0;
            CV_Set(cv, buf);
        }
        return true;
      default:
        if (is_printable(ch))
        {
            len=strlen(cv->string);
            if( len < MAX_CVAR_STRING-1 )
            {
                memcpy(buf,cv->string,len);
                buf[len++] = ch;
                buf[len] = 0;
                CV_Set(cv, buf);
            }
            return true;
        }
        break;
    }
    return false;
}

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    static  boolean button_down = 0;
    static  tic_t  mousewait = 0;
    static  int  mousey = 0;
    static  int  mousex = 0;

    menufunc_t routine;  // for some casting problem

    int i;
    int key = KEY_NULL; // key pressed (if any)
    char ch = '\0';  // ASCII char it corresponds to

    switch (ev->type )
    {
     case ev_keydown :
        key = ev->data1;  // keycode
        ch  = ev->data2;  // ASCII char
        if( key >= KEY_MOUSE1 )
        {
            if( key <= KEY_2MOUSE1 )
                button_down = 1;
        
            // added 5-2-98 remap virtual keys (mouse & joystick buttons)
            switch(key) {
             case KEY_MOUSE1:
             case KEY_JOY0BUT0:
                // [WDJ] No mouse ENTER key on the sliders, it makes them move right.
               key = ( currentMenu
                       && (currentMenu->menuitems[itemOn].status & (IT_BIGSLIDER | IT_CV_SLIDER | IT_CV_NOMOD ) )
                       ) ? KEY_NULL : KEY_ENTER;
               break;
             case KEY_MOUSE1+1:
             case KEY_JOY0BUT1:
               key = KEY_BACKSPACE;
               break;
            }
        }
        else
        {
            // on key press, inhibit menu responses to the mouse for a while
            mousewait = I_GetTime() + TICRATE*2;  // 4 sec
        }
        break;
     case ev_keyup:
        if( ev->data1 >= KEY_MOUSE1 && ev->data1 <= KEY_2MOUSE1 )
            button_down = 0;
        break;
     case ev_mouse:
        if( menuactive )
        {
            // [WDJ] This code only triggered when movement exceeded MENU_MOUSE_TRIG
            // so there is no need for mouse position recording.
            // Y movement overrides X movement, there can be ony one key.
            if( mousewait >= I_GetTime() )  break;  // delay between triggers
            mousex += ev->data2;
            mousey += ev->data3;
            if (mousey < -MENU_MOUSE_TRIG)
            {
                key = KEY_DOWNARROW;
            }
            else if (mousey > MENU_MOUSE_TRIG)
            {
                key = KEY_UPARROW;
            }
            else if (mousex < -MENU_MOUSE_TRIG)
            {
                if( button_down )   key = KEY_LEFTARROW;
            }
            else if (mousex > MENU_MOUSE_TRIG)
            {
                if( button_down )   key = KEY_RIGHTARROW;
            }

            if( key )
            {
                // On any trigger, zero everything, so cannot drift off slider.
                mousewait = I_GetTime() + TICRATE/7;
                mousex = mousey = 0;
            }
        }
        break;
     default:
        break;
    }

    if (key == KEY_NULL)
        return false;


    // Save Game string input
    if (edit_enable)
    {
        switch(key)
        {
          case KEY_BACKSPACE:
            if (edit_index > 0)
            {
                edit_index--;
                edit_buffer[edit_index] = 0;
            }
            break;

          case KEY_ESCAPE:
            edit_enable = 0;
            // restore from source
            strcpy(edit_buffer, &savegamedisp[slotindex].desc[0]);
            break;

          case KEY_ENTER:
            edit_enable = 0;
            if (edit_buffer[0])
            {
                strcpy(&savegamedisp[slotindex].desc[0], edit_buffer);
                if( edit_done_callback )   edit_done_callback();
                edit_done_callback = NULL;
            }
            break;

          default:
            if (is_printable(ch) &&
                edit_index < SAVESTRINGSIZE-1 &&
                V_StringWidth(edit_buffer) < (SAVESTRINGSIZE-2)*8)
            {
                edit_buffer[edit_index++] = ch;
                edit_buffer[edit_index] = 0;
            }
            break;
        }
        goto ret_true;
    }

    if (devparm && key == KEY_F1)
    {
        COM_BufAddText("screenshot\n");
        goto ret_true;
    }

    if( gamestate == GS_WAITINGPLAYERS )
    {
        if( D_WaitPlayer_Response( key ) )
            goto ret_true;
    }

    // when the menu is not open
    if (!menuactive)
    {
        switch(key)
        {
          case '-':         // Screen size down
            if (automapactive || chat_on || con_destlines)     // DIRTY !!!
                return false;
            CV_SetValue (&cv_viewsize, cv_viewsize.value-1);
            S_StartSound(NULL, menu_sfx_enter);
            goto ret_true;

          case '=':        // Screen size up
            if (automapactive || chat_on || con_destlines)     // DIRTY !!!
                return false;
            CV_SetValue (&cv_viewsize, cv_viewsize.value+1);
            S_StartSound(NULL, menu_sfx_enter);
            goto ret_true;

          case KEY_F1:            // Help key
            M_StartControlPanel ();

            if ( gamemode == ultdoom_retail )
              currentMenu = &ReadDef2;
            else
              currentMenu = &ReadDef1;

            itemOn = 0;
            S_StartSound(NULL, menu_sfx_open);
            goto ret_true;

          case KEY_F2:            // Save
            M_StartControlPanel();
            S_StartSound(NULL, menu_sfx_open);
            M_SaveGame(0);
            goto ret_true;

          case KEY_F3:            // Load
            M_StartControlPanel();
            S_StartSound(NULL, menu_sfx_open);
            M_LoadGame(0);
            goto ret_true;

          case KEY_F4:            // Sound Volume
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = SVM_sfx_vol;
            S_StartSound(NULL, menu_sfx_open);
            goto ret_true;

          //added:26-02-98: now F5 calls the Video Menu
          case KEY_F5:
            S_StartSound(NULL, menu_sfx_open);
            M_StartControlPanel();
            M_SetupNextMenu (&VidModeDef);
            //M_ChangeDetail(0);
            goto ret_true;

          case KEY_F6:            // Quicksave
            S_StartSound(NULL, menu_sfx_open);
            M_QuickSave();
            goto ret_true;

          //added:26-02-98: F7 changed to Options menu
          case KEY_F7:            // originally was End game
            S_StartSound(NULL, menu_sfx_open);
            M_StartControlPanel();
            M_SetupNextMenu (&OptionsDef);
            //M_EndGame(0);
            goto ret_true;

          case KEY_F8:            // Toggle messages
            CV_AddValue(&cv_showmessages,+1);
            S_StartSound(NULL, menu_sfx_open);
            goto ret_true;

          case KEY_F9:            // Quickload
            S_StartSound(NULL, menu_sfx_open);
            M_QuickLoad();
            goto ret_true;

          case KEY_F10:           // Quit DOOM
            S_StartSound(NULL, menu_sfx_open);
            M_QuitDOOM(0);
            goto ret_true;

          //added:10-02-98: the gamma toggle is now also in the Options menu
          case KEY_F11:
            S_StartSound(NULL, menu_sfx_open);
            // bring up the gamma menu
            M_StartControlPanel();
            M_SetupNextMenu (&VideoOptionsDef);
            goto ret_true;

          // Pop-up menu
          case KEY_ESCAPE:
            M_StartControlPanel ();
            S_StartSound(NULL, menu_sfx_open);
            goto ret_true;
        }
        return false;
    }

    routine = currentMenu->menuitems[itemOn].itemaction;

    //added:30-01-98:
    // Handle menuitems which need a specific key handling
    if(routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER )
    {
      input_char = ch;
      routine(key);
      goto ret_true;
    }

    if(currentMenu->menuitems[itemOn].status==IT_MSGHANDLER)
    {
        // special message menu
        if(currentMenu->menuitems[itemOn].alphaKey == true)
        {
          // [smite] just for this purpose since unraveling the IT_MSGHANDLER hack would be too harrowing
          if (tolower(ch) == 'n')
            key = 'n';
          else if (tolower(ch) == 'y')
            key = 'y';

          if(key == KEY_SPACE || key == 'n' || key == 'y' || key == KEY_ESCAPE)
          {
                if(routine) routine(key);
                M_StopMessage(0);
          }
        }
        else
        {
            //added:07-02-98:dirty hak:for the customize controls, I want only
            //      buttons/keys, not moves
            if (ev->type == ev_mouse)
                goto ret_true;
            void (*cc_action)(event_t *) = currentMenu->menuitems[itemOn].itemaction;
            if (cc_action)   cc_action(ev);
        }
        goto ret_true;
    }

    // BP: one of the more big hack i have never made
    if( routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR )
    {
        if( (currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_STRING )
        {
            if (M_ChangeStringCvar(key, ch))
                goto ret_true;
            else
                routine = NULL;
        }
        else
            routine=M_ChangeCvar;
    }

    // Keys usable within menu
    switch (key)
    {
#if defined SAVEGAMEDIR || defined SAVEGAME99
      case KEY_DELETE:	// delete directory or savegame
#ifdef SAVEGAMEDIR
        // The Dir, LoadGame, SaveGame menus all have dir at MSLOT_0
        if( delete_callback && itemOn >= SAVEGAME_MSLOT_0 )
        {
            slotindex = itemOn - SAVEGAME_MSLOT_0;
            M_StartMessage("Delete Y/N?", delete_callback, MM_YESNO);
            goto ret_action;
        }
#else
        if( delete_callback && itemOn >= 0 )
        {
            slotindex = itemOn;
            M_StartMessage("Delete Y/N?", delete_callback, MM_YESNO);
            goto ret_action;
        }
#endif
        break;
        
      case '[':
      case KEY_PGUP:
        if( scroll_callback && (scroll_index > 0))
        {
            scroll_callback( -6 );  // some functions need to correct
            goto ret_updown;
        }
        break;

      case ']':
      case KEY_PGDN:
        if( scroll_callback && (scroll_index < (99-6)))
        {
            scroll_callback( 6 );
            goto ret_updown;
        }
        break;
#endif

      case KEY_DOWNARROW:
#if defined SAVEGAMEDIR || defined SAVEGAME99
        if( scroll_callback && (scroll_index < 99) && (itemOn >= SAVEGAME_MSLOT_LAST))
        {
            // scrolling menu scrolls preferentially
            scroll_index ++;
            scroll_callback( 1 );
            goto ret_updown;
        }
#endif
        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
            {
                if( scroll_callback )  // only wrap when not scrolling
                    goto ret_updown;
                itemOn = 0;
            }
            else itemOn++;
        } while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
        goto ret_updown;

      case KEY_UPARROW:
#if defined SAVEGAMEDIR || defined SAVEGAME99
        if( scroll_callback && (scroll_index > 0) && (itemOn < (SAVEGAME_MSLOT_0+1)))
        {
            // scrolling menu scrolls preferentially
            scroll_index --;
            if( scroll_index < 0 )   scroll_index = 0;
            scroll_callback( -1 );  // some functions need to correct
            goto ret_updown;
        }
#endif       
        do
        {
            if (!itemOn)
                itemOn = currentMenu->numitems-1;
            else itemOn--;
        } while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
        goto ret_updown;

      case KEY_LEFTARROW:
        if (  routine &&
            ( (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
            ||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
        {
            S_StartSound(NULL, menu_sfx_val);
            routine(0);
        }
        goto ret_true;

      case KEY_RIGHTARROW:
        if ( routine &&
            ( (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
            ||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
        {
            S_StartSound(NULL, menu_sfx_val);
            routine(1);
        }
        goto ret_true;

      case KEY_ENTER:
        currentMenu->lastOn = itemOn;
        if ( routine )
        {
            switch (currentMenu->menuitems[itemOn].status & IT_TYPE)  {
                case IT_CVAR:
                case IT_ARROWS:
                    routine(1);            // right arrow
                    S_StartSound(NULL, menu_sfx_val);
                    break;
                case IT_CALL:
                    routine(itemOn);
                    S_StartSound(NULL, menu_sfx_enter);
                    break;
                case IT_SUBMENU:
                    currentMenu->lastOn = itemOn;
                    M_SetupNextMenu((menu_t *)currentMenu->menuitems[itemOn].itemaction);
                    S_StartSound(NULL, menu_sfx_enter);
                    break;
            }
        }
        goto ret_true;

      case KEY_ESCAPE:
        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            M_Setup_prevMenu();
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, menu_sfx_open); // it�s a matter of taste which sound to choose
        }
        else
        {
            M_ClearMenus (true);
            S_StartSound(NULL, menu_sfx_esc);
            // Exit menus, return to demo or game
            if( ! Game_Playing() )
                D_StartTitle();  // restart title screen and demo
        }
        goto ret_true;

      case KEY_BACKSPACE:
        if((currentMenu->menuitems[itemOn].status)==IT_CONTROL)
        {
            S_StartSound(NULL, menu_sfx_val);
            // detach any keys associated to the game control
            G_ClearControlKeys (setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
            goto ret_true;
        }
        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
            S_StartSound(NULL, menu_sfx_open);
        }
        goto ret_true;

      default:
#if 1
        // any other key: if a letter, try to find the corresponding menuitem
        if (!isalpha(ch))
#else
        if( ch == 0 )
#endif
          goto ret_true;

        // from itemOn to bottom
        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if( (currentMenu->menuitems[i].alphaKey == ch)
                 && ((currentMenu->menuitems[i].status & IT_OPTION) == 0) )
            {
                itemOn = i;
                goto ret_action;
            }
        }
        // search from top to itemOn
        for (i = 0;i <= itemOn;i++)
        {
            if( (currentMenu->menuitems[i].alphaKey == ch)
                 && ((currentMenu->menuitems[i].status & IT_OPTION) == 0) )
            {
                itemOn = i;
                goto ret_action;
            }
        }
        break;

    }
   
ret_true:   
    return true;

ret_action:
    S_StartSound(NULL, menu_sfx_action);
    return true;

ret_updown:
    S_StartSound(NULL, menu_sfx_updown);
    return true;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    int      i;
    int      h;
    int      height = 8; //(hu_font[0]->height);

    h = height;
    for (i = 0;i < (int)strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    if (!menuactive)
        return;

    // center the scaled graphics for the menu,
    V_SetupDraw( 0 | V_SCALEPATCH | V_SCALESTART | V_CENTERMENU );

    // now that's more readable with a faded background (yeah like Quake...)
    if ( reg_colormaps )  // not before colormaps loaded
        V_DrawFadeScreen ();

    // menu drawing
    if (currentMenu->drawroutine)
        currentMenu->drawroutine();      // call current menu Draw routine

    V_SetupDraw( 0 | V_SCALEPATCH | V_SCALESTART | V_CENTERHORZ );  // restore

}

//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC

    if(demoplayback)  // menus without the demo interference
    { 
        G_StopDemo();
        R_FillBackScreen ();
    }

    CON_ToggleOff ();   // dirty hack : move away console

    I_StartupMouse( false );  // menu mode
}

//
// M_ClearMenus
//
void M_ClearMenus (boolean callexitmenufunc)
{
    if(!menuactive)
        return;

    if( currentMenu->quitroutine && callexitmenufunc)
    {
        if( !currentMenu->quitroutine())
            return; // we can't quit this menu (also used to set parameter from the menu)
    }

    menuactive = 0;
    I_StartupMouse( !paused );  // play mode if not paused
}


//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    if( currentMenu->quitroutine )
    {
        if( !currentMenu->quitroutine())
            return; // we can't quit this menu (also used to set parameter from the menu)
    }
    // Menu history
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;

    // in case of...
    if (itemOn >= currentMenu->numitems)
        itemOn = currentMenu->numitems - 1;

    // the curent item can be disabled,
    // this code go up until a enabled item found
    while(currentMenu->menuitems[itemOn].status==IT_DISABLED && itemOn)
        itemOn--;
    delete_callback = NULL;
    scroll_callback = NULL;
}

// Go back to the previous menu, reloading data if necessary
void  M_Setup_prevMenu( void )
{
    if (currentMenu->prevMenu)
    {
       M_SetupNextMenu (currentMenu->prevMenu);
       // refresh data
       if( currentMenu == &LoadDef )   M_LoadGame(0);
       if( currentMenu == &SaveDef )   M_SaveGame(0);
    }
    else
    {
       M_ClearMenus (true);
       // Exit menus, return to demo or game
       if( ! Game_Playing() )
           D_StartTitle();  // restart title screen and demo
    }
}


//
// M_Ticker
//
// Call once per tic.
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
        whichSkull ^= 1;
        skullAnimCounter = 8 * NEWTICRATERATIO;
    }

    //added:30-01-98:test mode for five seconds
    if( vidm_testing_cnt>0 )
    {
        // restore the previous video mode
        if (--vidm_testing_cnt==0)
            setmodeneeded = vidm_previousmode;
    }
}


//
// M_Init
//
// Called once, very early
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;

    whichSkull = 0;
    skullAnimCounter = 10;

    quicksave_slotid = -1;
   
    CV_RegisterVar(&cv_skill);
    CV_RegisterVar(&cv_monsters);
    CV_RegisterVar(&cv_nextmap );
    CV_RegisterVar(&cv_nextepmap );
    CV_RegisterVar(&cv_newdeathmatch);
    CV_RegisterVar(&cv_wait_players);
    CV_RegisterVar(&cv_wait_timeout);
    CV_RegisterVar(&cv_serversearch);
    CV_RegisterVar(&cv_downloadfiles);
    CV_RegisterVar(&cv_menusound);
}

// Called once, game dependent
void M_Configure (void)
{
    int i;
    int cval;

    if(dedicated)
        return;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    // Reversible
    // remove the inventory key from the menu !
    cval = ( have_inventory )? (IT_CONTROL) : IT_LITLSPACE;
    for( i=0; i<ControlDef2.numitems; i++)
    {
        if( ControlMenu2[i].alphaKey == gc_invprev ||
            ControlMenu2[i].alphaKey == gc_invnext ||
            ControlMenu2[i].alphaKey == gc_invuse )
            ControlMenu2[i].status = cval;
    }

    // remove the fly down key from the menu !
    cval = ( gamemode == heretic )? (IT_CONTROL) : IT_LITLSPACE;
    for( i=0; i<ControlDef2.numitems; i++)
    {
        if( ControlMenu2[i].alphaKey == gc_flydown )
            ControlMenu2[i].status = cval;
    }

    // irreversible
    if( W_CheckNumForName("E2M1")<0 )
    {
        exmy_cons_t[9].value = 0;
        exmy_cons_t[9].strvalue = NULL;
    }
    else
    if( W_CheckNumForName("E3M1")<0 )
    {
        exmy_cons_t[18].value = 0;
        exmy_cons_t[18].strvalue = NULL;
    }
    else
    if( W_CheckNumForName("E4M1")<0 )
    {
        exmy_cons_t[27].value = 0;
        exmy_cons_t[27].strvalue = NULL;
    }
    else
    if( W_CheckNumForName("E5M1")<0 )
    {
        exmy_cons_t[36].value = 0;
        exmy_cons_t[36].strvalue = NULL;
    }

    switch ( gamemode )
    {
      case doom2_commercial:
        // This is used because DOOM 2 had only one HELP
        //  page. I use CREDIT as second page now, but
        //  kept this hack for educational purposes.
        MainMenu[readthis] = MainMenu[quitdoom];
        MainDef.numitems--;
        MainDef.y += 8;
        NewDef.prevMenu = &MainDef;
        ReadDef1.drawroutine = M_DrawReadThis1;
        ReadDef1.x = 330;
        ReadDef1.y = 165;
        ReadMenu1[0].itemaction = &MainDef;
        break;
      case doom_shareware:
        // Episode 2 and 3 are handled,
        //  branching to an ad screen.
      case doom_registered:
          // We need to remove the fourth episode.
          EpiDef.numitems--;
      case ultdoom_retail:
          // We are fine.
          cv_nextmap.PossibleValue = exmy_cons_t;
          cv_nextmap.defaultvalue = "11";
          // We need to remove the fifth episode.
          EpiDef.numitems--;
          break;
      case heretic:
          cv_nextmap.PossibleValue = exmy_cons_t;
          cv_nextmap.defaultvalue = "11";

          MainDef.menutitlepic = "M_HTIC";
          MainDef.drawroutine = HereticMainMenuDrawer;
          SkullBaseLump = W_GetNumForName("M_SKL00");
          strcpy(skullName[0], "M_SLCTR1");
          strcpy(skullName[1], "M_SLCTR2");

          EpisodeMenu[0].text = "CITY OF THE DAMNED";
          EpisodeMenu[1].text = "HELL'S MAW";
          EpisodeMenu[2].text = "THE DOME OF D'SPARIL";
          EpisodeMenu[3].text = "THE OSSUARY";
          EpisodeMenu[4].text = "THE STAGNANT DEMESNE";

          NewGameMenu[0].text = "THOU NEEDETH A WET-NURSE";
          NewGameMenu[1].text = "YELLOWBELLIES-R-US";
          NewGameMenu[2].text = "BRINGEST THEM ONETH";
          NewGameMenu[3].text = "THOU ART A SMITE-MEISTER";
          NewGameMenu[4].text = "BLACK PLAGUE POSSESSES THEE";
          break;

      case chexquest1:
          if( Chex_safe_pictures( "M_EPI1", NULL ) == NULL )
          {
              // Found Doom episode title in wad.
#if 1
              // [WDJ] Coverup for Doom episode titles in chexquest1 wad
              // According to Chexquest3.
              EpisodeMenu[0].text = "Rescue on Baziok";
              EpisodeMenu[1].text = "Terror in Chex-City"; // Avoid needing (R)
              EpisodeMenu[2].text = "Invasion";
              EpisodeMenu[3].text = "Episode 4";
              EpisodeMenu[4].text = "Episode 5";
              EpisodeMenu[0].status = IT_CALL | IT_STRING2;
              EpisodeMenu[1].status = IT_CALL | IT_STRING2;
              EpisodeMenu[2].status = IT_CALL | IT_STRING2;
              EpisodeMenu[3].status = IT_CALL | IT_STRING2;
              EpisodeMenu[4].status = IT_CALL | IT_STRING2;
#else
              NewDef.prevMenu = &MainDef;  // disable access to episodes menu
#endif
          }
          break;

       
      default:
        break;
    }

    CV_menusound_OnChange();
}


//======================================================================
// OpenGL specifics options
//======================================================================

#ifdef HWRENDER

void M_DrawOpenGLMenu(void);
void M_OGL_DrawFogMenu(void);
void M_OGL_DrawColorMenu(void);
void M_HandleFogColor (int choice);

menu_t OGL_LightingDef, OGL_FogDef, OGL_ColorDef, OGL_DevDef;

#define QUALITY_ITEM   2
menuitem_t OpenGLOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Mouse look"          , &cv_grmlook_extends_fov ,  0},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Field of view"       , &cv_grfov             , 10},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Quality"             , &cv_scr_depth         , 20},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Texture Filter"      , &cv_grfiltermode      , 30},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER | IT_YOFFSET, 0, "Translucent HUD", &cv_grtranslucenthud  , 40},

    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0, "Lighting..."       , &OGL_LightingDef   , 65},
    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0, "Fog..."            , &OGL_FogDef        , 75},
    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0, "Gamma..."          , &OGL_ColorDef      , 85},
    {IT_SUBMENU | IT_WHITESTRING | IT_YOFFSET, 0, "Development..."    , &OGL_DevDef        , 95},
};

menuitem_t OGL_LightingMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Coronas"                 , &cv_grcoronas         ,  0},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Coronas size"            , &cv_grcoronasize      , 10},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Dynamic lighting"        , &cv_grdynamiclighting , 20},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Static lighting"         , &cv_grstaticlighting  , 30},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0, "Monsters' balls lighting", &cv_grmblighting      , 40},
};

#define FOG_COLOR_ITEM  1
menuitem_t OGL_FogMenu[]=
{
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0,"Fog"             , &cv_grfog              ,  0},
    {IT_STRING | IT_KEYHANDLER| IT_YOFFSET, 0, "Fog color" , M_HandleFogColor       , 10},
    {IT_STRING | IT_CVAR | IT_YOFFSET, 0,"Fog density"     , &cv_grfogdensity       , 20},
};                                         

menuitem_t OGL_ColorMenu[]=
{
    //{IT_STRING | NOTHING, "Gamma correction", NULL                   ,  0},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER | IT_YOFFSET, 0,"red"  , &cv_grgammared     , 10},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER | IT_YOFFSET, 0,"green", &cv_grgammagreen   , 20},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER | IT_YOFFSET, 0,"blue" , &cv_grgammablue    , 30},
    //{IT_STRING | IT_CVAR | IT_CV_SLIDER, "Constrast", &cv_grcontrast , 50},
};

menuitem_t OGL_DevMenu[]=
{
//    {IT_STRING | IT_CVAR, "Polygon smooth"  , &cv_grpolygonsmooth    ,  0},
    {IT_STRING | IT_CVAR, 0, "MD2 models"      , &cv_grmd2              , 10},
#ifdef TRANSWALL_CHOICE
    {IT_STRING | IT_CVAR, 0, "Translucent walls", &cv_grtranswall       , 20},
#endif
    {IT_STRING | IT_CVAR, 0, "Polygon shape"  , &cv_grpolyshape         , 30},
};

menu_t  OpenGLOptionDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OpenGLOptionsMenu)/sizeof(menuitem_t),
    &VideoOptionsDef,
    OpenGLOptionsMenu,
    M_DrawOpenGLMenu,
    60,40,
    0
};

menu_t  OGL_LightingDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_LightingMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_LightingMenu,
    M_DrawGenericMenu,
    60,40,
    0,
};

menu_t  OGL_FogDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_FogMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_FogMenu,
    M_OGL_DrawFogMenu,
    60,40,
    0,
};

menu_t  OGL_ColorDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_ColorMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_ColorMenu,
    M_OGL_DrawColorMenu,
    60,40,
    0,
};

menu_t  OGL_DevDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_DevMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_DevMenu,
    M_DrawGenericMenu,
    60,40,
    0,
};


//======================================================================
// M_DrawOpenGLMenu()
//======================================================================
void M_DrawOpenGLMenu(void)
{
    int             mx,my;

    mx = OpenGLOptionDef.x;
    my = OpenGLOptionDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    // Draw the number in the Quality menu item.
    V_DrawString(BASEVIDWIDTH-mx-V_StringWidth(cv_scr_depth.string),
                 my+currentMenu->menuitems[QUALITY_ITEM].alphaKey,
                 V_WHITEMAP,
                 cv_scr_depth.string);
}


//======================================================================
// M_OGL_DrawFogMenu()
//======================================================================
void M_OGL_DrawFogMenu(void)
{
    int             mx,my;

    mx = OGL_FogDef.x;
    my = OGL_FogDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    // Draw the fog color number in the menu.
    V_DrawString(BASEVIDWIDTH-mx-V_StringWidth (cv_grfogcolor.string),
                 my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey,
                 V_WHITEMAP,
                 cv_grfogcolor.string);
    if (itemOn==FOG_COLOR_ITEM && skullAnimCounter<4) //blink cursor on FOG_COLOR_ITEM if selected
        V_DrawCharacter( BASEVIDWIDTH-mx, my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey, '_' | 0x80);  // white
}


//======================================================================
// M_OGL_DrawColorMenu()
//======================================================================
void M_OGL_DrawColorMenu(void)
{
    int             mx,my;

    mx = OGL_ColorDef.x;
    my = OGL_ColorDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    V_DrawString(mx, my+currentMenu->menuitems[0].alphaKey-10,
                 V_WHITEMAP,"Gamma correction");
}


//======================================================================
// M_OpenGLOption()
//======================================================================
void M_OpenGLOption(int choice)
{
    if (rendermode != render_soft )
        M_SetupNextMenu(&OpenGLOptionDef);
    else
        M_SimpleMessage("You are in software mode\nYou cannot change GL options\n");
}


//======================================================================
// M_HandleFogColor()
//======================================================================
void M_HandleFogColor(int key)
{
    int      i, l;
    char     temp[8];
    boolean  exitmenu = false;  // exit to previous menu and send name change

    switch( key )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL, menu_sfx_updown);
        itemOn++;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL, menu_sfx_updown);
        itemOn--;
        break;

      case KEY_ESCAPE:
        S_StartSound(NULL, menu_sfx_esc);
        exitmenu = true;
        break;

      case KEY_BACKSPACE:
        S_StartSound(NULL, menu_sfx_val);
        strcpy(temp, cv_grfogcolor.string);
        strcpy(cv_grfogcolor.string, "000000");
        l = strlen(temp)-1;
        for (i=0; i<l; i++)
            cv_grfogcolor.string[i+6-l] = temp[i];
        break;

      default:
        input_char = tolower(input_char);
        if ((input_char >= '0' && input_char <= '9') ||
            (input_char >= 'a' && input_char <= 'f'))
        {
            S_StartSound(NULL, menu_sfx_val);
            strcpy(temp, cv_grfogcolor.string);
            strcpy(cv_grfogcolor.string, "000000");
            l = strlen(temp);
            for (i=0; i<l; i++)
                cv_grfogcolor.string[5-i] = temp[l-i];
            cv_grfogcolor.string[5] = input_char;
        }
        break;
    }
    if (exitmenu)
    {
        M_Setup_prevMenu();
    }
}

#endif


#ifdef LAUNCHER
//===========================================================================
//                        LAUNCH MENU
//===========================================================================

#if 0
enum
{
    config_ent,
    doomdir_ent,
    game_ent,
    iwad_ent,
    file_ent,
    switches_ent,
    launch_ent,
    quit_ent,
    launch_end
} launch_e;
#endif

// cv_ menu items
// out only
consvar_t cv_switch = {"switches", "", CV_HIDEN, NULL};
consvar_t cv_config = {"config", "", CV_HIDEN, NULL};

static void CV_game_OnChange(void);
CV_PossibleValue_t game_cons_t[] = {{-1,"MIN"},{64,"MAX"},{0,NULL}};
consvar_t cv_game = {"game", "-1", CV_HIDEN|CV_CALL, NULL, CV_game_OnChange};

static void CV_game_OnChange(void)
{
    // Strings come from GameDesc, not a CV_PossibleValue_t
    // Cannot call CV_Set within CV_CALL routine
    char * rs;
    game_desc_t * gamedesc = D_GameDesc( cv_game.value );
    if( gamedesc )
    {
        rs = gamedesc->gname;  // const string from table
    }
    else
    {
        rs = "Auto";
        cv_game.value = (cv_game.value <1)? -1 : GDESC_other;
    }
    if( cv_game.string )
       Z_Free( cv_game.string );  // remove the string from CV_Set
    cv_game.string = Z_StrDup( rs );
    return;
}



static void M_LaunchCont( void )
{
    init_sequence = 2;
    M_ClearMenus(true);
}

menuitem_t LaunchMenu[]=
{
    {IT_WHITESTRING | IT_CVAR | IT_CV_STRING, NULL, "Home", &cv_home,  0},
    {IT_WHITESTRING | IT_CVAR | IT_CV_STRING, NULL, "Doomwaddir", &cv_doomwaddir, 0},
    {IT_WHITESTRING | IT_CVAR | IT_CV_STRING, NULL, "Config", &cv_config,  0},
    {IT_WHITESTRING | IT_CVAR | IT_CV_STRING, NULL, "Switch", &cv_switch, 0},
    {IT_WHITESTRING | IT_CVAR | IT_CV_STRING, NULL, "Iwad", &cv_iwad, 0},
 // the CVAR strings will intercept the 'g', 'c', and 'q'
    {IT_WHITESTRING | IT_CVAR, NULL, "Game", &cv_game,  'g'},
    {IT_WHITESTRING | IT_CALL, NULL, "Continue", M_LaunchCont, 'c'},
    {IT_WHITESTRING | IT_CALL, NULL, "QUIT GAME", M_QuitDOOM, 'q'}
};

menu_t  LaunchDef =
{
    NULL,
    "Launch",
    sizeof(LaunchMenu)/sizeof(menuitem_t),
    NULL,
    LaunchMenu,
    M_DrawGenericMenu,
    10,10,  // x, y  (cursor at x-10)
    0
};

// call only after memory, video, command, and menu inits
void M_LaunchMenu( void )
{
    M_StartControlPanel();
    M_SetupNextMenu (&LaunchDef);

    M_Clear_Add_Param();  // clear previous param from this routine
    if( cv_game.string == NULL ) // first time inits
    {
//        CV_RegisterVar(&cv_iwad);
        CV_RegisterVar(&cv_config);
        CV_RegisterVar(&cv_switch);
        CV_RegisterVar(&cv_game);
    }
    skullAnimCounter = 0;

    do
    {
        V_ClearDisplay();
        I_OsPolling();
        D_ProcessEvents ();  // menu responder
        M_Drawer(); // menu drawer
        I_UpdateNoBlit();
        I_FinishUpdate();       // page flip or blit buffer
    } while( menuactive );

    // add home
    if(cv_home.flags & CV_MODIFIED)
        M_Change_2Param( "-home", cv_home.string );
    // add doomwaddir
    if( (cv_doomwaddir.flags & CV_MODIFIED) && cv_doomwaddir.string )
        doomwaddir[0] = cv_doomwaddir.string;
    // add config
    if(cv_config.flags & CV_MODIFIED)
        M_Change_2Param( "-config", cv_config.string );
    // add iwad
    if(cv_iwad.flags & CV_MODIFIED)
        M_Change_2Param( "-iwad", cv_iwad.string );
    // add switches
    if( (cv_switch.flags & CV_MODIFIED) && cv_switch.string[0] )
        M_Add_Param( cv_switch.string, NULL );
    // add game
    if( (cv_game.flags & CV_MODIFIED)
        && cv_game.value >= 0 && cv_game.value < GDESC_other)
    {
        game_desc_t * gamedesc = D_GameDesc( cv_game.value );
        if( gamedesc )
            M_Add_Param( "-game", gamedesc->idstr );
    }
}

#endif
