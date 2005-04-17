// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2005 by DooM Legacy Team.
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
// $Log$
// Revision 1.23  2005/04/17 18:36:32  smite-meister
// netcode
//
// Revision 1.21  2005/03/16 21:16:05  smite-meister
// menu cleanup, bugfixes
//
// Revision 1.20  2004/11/04 21:12:51  smite-meister
// save/load fixed
//
// Revision 1.19  2004/10/27 17:37:06  smite-meister
// netcode update
//
// Revision 1.18  2004/09/24 21:19:59  jussip
// Joystick axis unbinding.
//
// Revision 1.16  2004/09/23 23:21:16  smite-meister
// HUD updated
//
// Revision 1.15  2004/09/20 22:42:48  jussip
// Joystick axis binding works. New joystick code ready for use.
//
// Revision 1.14  2004/09/13 20:43:29  smite-meister
// interface cleanup, sp map reset fixed
//
// Revision 1.13  2004/09/10 19:59:15  jussip
// Joystick code almost ready for use.
//
// Revision 1.12  2004/09/09 22:04:38  jussip
// New joy code a bit more finished. Button binding works.
//
// Revision 1.11  2004/09/09 17:15:18  jussip
// Cleared out old joystick crap in preparation for brand new code.
//
// Revision 1.7  2004/07/05 16:53:24  smite-meister
// Netcode replaced
//
// Revision 1.6  2004/01/10 16:02:59  smite-meister
// Cleanup and Hexen gameplay -related bugfixes
//
// Revision 1.5  2003/12/09 01:02:00  smite-meister
// Hexen mapchange works, keycodes fixed
//
// Revision 1.4  2003/02/08 21:43:50  smite-meister
// New Memzone system. Choose your pawntype! Cyberdemon OK.
//
// Revision 1.3  2003/01/25 21:33:05  smite-meister
// Now compiles with MinGW 2.0 / GCC 3.2.
// Builder can choose between dynamic and static linkage.
//
// Revision 1.2  2003/01/12 12:56:40  smite-meister
// Texture bug finally fixed! Pickup, chasecam and sw renderer bugs fixed.
//
// Revision 1.1.1.1  2002/11/16 14:17:51  hurdler
// Initial C++ version of Doom Legacy
//-----------------------------------------------------------------------------

/// \file
/// \brief Handles keyboard/mouse/joystick inputs,
/// maps inputs to game controls (forward, use, fire...).

#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "command.h"
#include "cvars.h"

#include "d_event.h"
#include "g_input.h"

#include "g_game.h"
#include "g_player.h"
#include "g_pawn.h"

#include "i_system.h"
#include "i_video.h"

#include "tables.h"

#include <SDL/SDL.h>

extern vector<SDL_Joystick*> joysticks;
vector<joybinding_t> joybindings;


CV_PossibleValue_t onecontrolperkey_cons_t[]={{1,"One"},{2,"Several"},{0,NULL}};
CV_PossibleValue_t usemouse_cons_t[]={{0,"Off"},{1,"On"},{2,"Force"},{0,NULL}};

consvar_t cv_controlperkey = {"controlperkey","1",CV_SAVE,onecontrolperkey_cons_t};

consvar_t cv_usemouse[2] = {{"use_mouse","1", CV_SAVE | CV_CALL,usemouse_cons_t,I_StartupMouse},
			    {"use_mouse2","0", CV_SAVE | CV_CALL,usemouse_cons_t,I_StartupMouse2}};

#define MAXMOUSESENSITIVITY 40 // sensitivity steps
CV_PossibleValue_t mousesens_cons_t[]={{1,"MIN"},{MAXMOUSESENSITIVITY,"MAX"},{0,NULL}};

consvar_t cv_mousesensx[2]  = {{"mousesensx","10",  CV_SAVE, mousesens_cons_t},
			       {"mousesensx2","10", CV_SAVE, mousesens_cons_t}};
consvar_t cv_mousesensy[2]  = {{"mousesensy","10",  CV_SAVE, mousesens_cons_t},
			       {"mousesensy2","10", CV_SAVE, mousesens_cons_t}};
consvar_t cv_automlook[2]   = {{"automlook",  "0",   CV_SAVE, CV_OnOff},
			       {"automlook2", "0",   CV_SAVE, CV_OnOff}};
consvar_t cv_mousemove[2]   = {{"mousemove",  "1",   CV_SAVE, CV_OnOff},
			       {"mousemove2", "1",   CV_SAVE, CV_OnOff}};
consvar_t cv_invertmouse[2] = {{"invertmouse",  "0", CV_SAVE, CV_OnOff},
			       {"invertmouse2", "0", CV_SAVE, CV_OnOff}};

#ifdef LMOUSE2
 CV_PossibleValue_t mouse2port_cons_t[]={{0,"/dev/gpmdata"},{1,"/dev/ttyS0"},{2,"/dev/ttyS1"},{3,"/dev/ttyS2"},{4,"/dev/ttyS3"},{0,NULL}};
 consvar_t cv_mouse2port  = {"mouse2port","/dev/gpmdata", CV_SAVE, mouse2port_cons_t };
 consvar_t cv_mouse2opt = {"mouse2opt","0", CV_SAVE, NULL};
#else
 CV_PossibleValue_t mouse2port_cons_t[]={{1,"COM1"},{2,"COM2"},{3,"COM3"},{4,"COM4"},{0,NULL}};
 consvar_t cv_mouse2port  = {"mouse2port","COM2", CV_SAVE, mouse2port_cons_t };
#endif


//========================================================================
//   Input status
//========================================================================

static int mousex[2], mousey[2];

/// current state of the keys : true if down
bool gamekeydown[NUMINPUTS];

/// Releases all game keys.
void G_ReleaseKeys()
{
  for (int i=0; i<NUMINPUTS; i++)
    gamekeydown[i] = false;
}


/// Two key (or virtual key) codes per game control
short gamecontrol[2][num_gamecontrols][2];

/// Control keys common to all local players (talk, console etc)
short commoncontrols[num_commoncontrols][2];

//========================================================================

static char forwardspeed[2] = {50, 100};
static char sidespeed[2]    = {48, 80};

#define MAXPLMOVE (forwardspeed[1])
// Stored in a signed char, but min = -100, max = 100, mmmkay?
// There is no more turbo cheat, you should modify the pawn's max speed instead

// fixed_t MaxPlayerMove[NUMCLASSES] = { 60, 50, 45, 49 }; // implemented as pawn speeds in info_m.cpp
// Otherwise OK, but fighter should also run sideways almost as fast as forward,
// (59/60) instead of (80/100). Weird. Affects straferunning.

static fixed_t angleturn[3] = {640, 1280, 320};  // + slow turn

//========================================================================


// clips the pitch angle to avoid ugly renderer effects
angle_t G_ClipAimingPitch(angle_t pitch)
{
  int p = pitch;
  int limitangle;

  //note: the current software mode implementation doesn't have true perspective
  if (rendermode == render_soft)
    limitangle = 732<<ANGLETOFINESHIFT;
  else
    limitangle = ANG90 - 1;

  if (p > limitangle)
    pitch = limitangle;
  else if (p < -limitangle)
    pitch = -limitangle;
  
  return pitch;
}


static byte NextWeapon(PlayerPawn *p, int step)
{
  // Kludge. TODO when netcode is redone, fix me.
  int w = p->readyweapon;
  do
    {
      w = (w + step + NUMWEAPONS) % NUMWEAPONS;
      if (p->weaponowned[w] && p->ammo[p->weaponinfo[w].ammo] >= p->weaponinfo[w].ammopershoot)
        return w;
    } while (w != p->readyweapon);

  return 0;
}



void ticcmd_t::Clear()
{
  buttons = 0;
  item = 0;
  forward = side = 0;
  yaw = pitch = 0;
}


//! Builds a ticcmd from all of the available inputs
void ticcmd_t::Build(LocalPlayerInfo *pref, int realtics)
{
#define KB_LOOKSPEED    (1<<25)
#define SLOWTURNTICS    (6*NEWTICRATERATIO)

  int i;
  int c = pref->controlkeyset; // atm must be 0 or 1
  short (*gc)[2]   = gamecontrol[c];
  PlayerPawn *pawn = pref->info ? pref->info->pawn : NULL;

  int  speed = (gamekeydown[gc[gc_speed][0]] || gamekeydown[gc[gc_speed][1]]) ^ pref->autorun;
  bool mouseaiming = (gamekeydown[gc[gc_mouseaiming][0]] || gamekeydown[gc[gc_mouseaiming][1]]) ^ cv_automlook[c].value;
  bool strafe = gamekeydown[gc[gc_strafe][0]] || gamekeydown[gc[gc_strafe][1]];

  bool turnright = gamekeydown[gc[gc_turnright][0]] || gamekeydown[gc[gc_turnright][1]];
  bool turnleft  = gamekeydown[gc[gc_turnleft][0]] || gamekeydown[gc[gc_turnleft][1]];

  // initialization
  //memcpy(this, I_BaseTiccmd(), sizeof(ticcmd_t)); // FIXME dangerous
  Clear();

  if (pawn)
    {
      yaw   = pawn->angle >> 16;
      pitch = pawn->aiming >> 16;
    }

  // use two stage accelerative turning on the keyboard (and joystick)
  static int turnheld[NUM_LOCALHUMANS]; // for accelerative turning

  // strafing and yaw
  if (strafe)
    {
      if (turnright)
        side += sidespeed[speed];
      if (turnleft)
        side -= sidespeed[speed];
    }
  else
    {
      if (turnleft || turnright)
	turnheld[c] += realtics;
      else
	turnheld[c] = 0;

      int tspeed;
      if (turnheld[c] < SLOWTURNTICS)
	tspeed = 2;             // slow turn
      else
	tspeed = speed;

      if (turnright)
        yaw -= angleturn[tspeed];
      if (turnleft)
        yaw += angleturn[tspeed];
    }


  // forwards/backwards, strafing

  if (gamekeydown[gc[gc_forward][0]] || gamekeydown[gc[gc_forward][1]])
    forward += forwardspeed[speed];

  if (gamekeydown[gc[gc_backward][0]] || gamekeydown[gc[gc_backward][1]])
    forward -= forwardspeed[speed];

  if (gamekeydown[gc[gc_straferight][0]] || gamekeydown[gc[gc_straferight][1]])
    side += sidespeed[speed];

  if (gamekeydown[gc[gc_strafeleft][0]] || gamekeydown[gc[gc_strafeleft][1]])
    side -= sidespeed[speed];


  // buttons

  if (gamekeydown[gc[gc_fire][0]] || gamekeydown[gc[gc_fire][1]])
    buttons |= BT_ATTACK;

  if (gamekeydown[gc[gc_use][0]] || gamekeydown[gc[gc_use][1]])
    buttons |= BT_USE;

  if (gamekeydown[gc[gc_jump][0]] || gamekeydown[gc[gc_jump][1]])
    buttons |= BT_JUMP;

  if (gamekeydown[gc[gc_flydown][0]] || gamekeydown[gc[gc_flydown][1]])
    buttons |= BT_FLYDOWN;

  if (pawn)
    {
      // impulse-type controls
      if (gamekeydown[gc[gc_nextweapon][0]] || gamekeydown[gc[gc_nextweapon][1]])
	buttons |= ((NextWeapon(pawn, 1) + 1) << WEAPONSHIFT);
      else if (gamekeydown[gc[gc_prevweapon][0]] || gamekeydown[gc[gc_prevweapon][1]])
	buttons |= ((NextWeapon(pawn, -1) + 1) << WEAPONSHIFT);
      else for (i = gc_weapon1; i <= gc_weapon8; i++)
	if (gamekeydown[gc[i][0]] || gamekeydown[gc[i][1]])
	  {
	    buttons |= ((pawn->FindWeapon(i - gc_weapon1) + 1) << WEAPONSHIFT);
	    break;
	  }
    }

  // pitch
  static bool keyboard_look[NUM_LOCALHUMANS]; // true if lookup/down using keyboard

  // spring back if not using keyboard neither mouselookin'
  if (!keyboard_look[c] && !mouseaiming)
    pitch = 0;

  if (gamekeydown[gc[gc_lookup][0]] || gamekeydown[gc[gc_lookup][1]])
    {
      pitch += KB_LOOKSPEED;
      keyboard_look[c] = true;
    }
  else if (gamekeydown[gc[gc_lookdown][0]] || gamekeydown[gc[gc_lookdown][1]])
    {
      pitch -= KB_LOOKSPEED;
      keyboard_look[c] = true;
    }
  else if (gamekeydown[gc[gc_centerview][0]] || gamekeydown[gc[gc_centerview][1]])
    {
      pitch = 0;
      keyboard_look[c] = false;
    }

  // Mice. Only two supported for now:)
  if (c < 2)
    {
      if (mouseaiming)
	{
	  keyboard_look[c] = false;

	  // looking up/down
	  if (cv_invertmouse[c].value)
	    pitch -= mousey[c] << 3;
	  else
	    pitch += mousey[c] << 3;
	}
      else if (cv_mousemove[c].value)
	forward += mousey[c];

      if (strafe)
	side += mousex[c] << 1;
      else
	yaw -= mousex[c] << 3;

      mousex[c] = mousey[c] = 0;
    }

  // Finally the joysticks.
  for (i=0; i < int(joybindings.size()); i++)
    {
      joybinding_t &j = joybindings[i];

      if (j.playnum != c)
	continue;

      int value = int(j.scale * SDL_JoystickGetAxis(joysticks[j.joynum], j.axisnum));
      switch (j.action)
	{
	case ja_pitch  : pitch = value; break;
	case ja_move   : forward += value; break;
	case ja_turn   : yaw += value; break;
	case ja_strafe : side += value; break;
	default: break;
	}
    }


  if (forward > MAXPLMOVE)
    forward = MAXPLMOVE;
  else if (forward < -MAXPLMOVE)
    forward = -MAXPLMOVE;
  if (side > MAXPLMOVE)
    side = MAXPLMOVE;
  else if (side < -MAXPLMOVE)
    side = -MAXPLMOVE;

  //26/02/2000: added by Hurdler: accept no mlook for network games
  if (!cv_allowmlook.value)
    pitch = 0;
  else
    pitch = G_ClipAimingPitch(pitch << 16) >> 16;
  //CONS_Printf("Move: %d, %d, %d\n", yaw, pitch, buttons);

  // HACK finally, we must release some keys manually (see G_MapEventsToControls)
  gamekeydown[KEY_MOUSEWHEELUP] = false;
  gamekeydown[KEY_MOUSEWHEELDOWN] = false;
  gamekeydown[KEY_2MOUSEWHEELUP] = false;
  gamekeydown[KEY_2MOUSEWHEELDOWN] = false;
}



struct dclick_t
{
  int time;
  int state;
  int clicks;
};

static  dclick_t  mousedclicks[MOUSEBUTTONS];
//static  dclick_t  joydclicks[JOYBUTTONS];

//
//  General double-click detection routine for any kind of input.
//
static bool G_CheckDoubleClick(int state, dclick_t *dt)
{
  if (state != dt->state && dt->time > 1 )
    {
      dt->state = state;
      if (state)
	dt->clicks++;
      if (dt->clicks == 2)
        {
	  dt->clicks = 0;
	  return true;
        }
      else
	dt->time = 0;
    }
  else
    {
      dt->time ++;
      if (dt->time > 20)
        {
	  dt->clicks = 0;
	  dt->state = 0;
        }
    }
  return false;
}

//
//  Remaps the inputs to game controls.
//  A game control can be triggered by one or more keys/buttons.
//  Each key/mousebutton/joybutton triggers ONLY ONE game control.
//
bool G_MapEventsToControls(event_t *ev)
{
  if (game.inventory)
    for (unsigned i=0; i < NUM_LOCALHUMANS; i++)
      if (LocalPlayers[i].info &&
	  LocalPlayers[i].info->InventoryResponder(gamecontrol[LocalPlayers[i].controlkeyset], ev))
	return true;

  switch (ev->type)
    {
    case ev_keydown:
      if (ev->data1 < NUMINPUTS)
	gamekeydown[ev->data1] = true;
      break;

    case ev_keyup:
      if (ev->data1 < NUMINPUTS)
	{
	  switch (ev->data1)
	    {
	    case KEY_MOUSEWHEELUP:
	    case KEY_MOUSEWHEELDOWN:
	    case KEY_2MOUSEWHEELUP:
	    case KEY_2MOUSEWHEELDOWN:
	      // these can only be used for impulse controls,
	      // since a keydown event is almost immediately followed by a keyup.
	      break;

	    default:
	      gamekeydown[ev->data1] = false;
	      break;
	    }
	}
      break;

    case ev_mouse:           // buttons are virtual keys
      mousex[0] = int(ev->data2*((cv_mousesensx[0].value*cv_mousesensx[0].value)/110.0f + 0.1));
      mousey[0] = int(ev->data3*((cv_mousesensy[0].value*cv_mousesensy[0].value)/110.0f + 0.1));
      break;

    case ev_mouse2:           // buttons are virtual keys
      mousex[1] = int(ev->data2*((cv_mousesensx[1].value*cv_mousesensx[1].value)/110.0f + 0.1));
      mousey[1] = int(ev->data3*((cv_mousesensy[1].value*cv_mousesensy[1].value)/110.0f + 0.1));
      break;

    default:
      break;
    }

  int i, flag;

  // ALWAYS check for mouse & joystick double-clicks
  // even if no mouse event
  for (i=0;i<MOUSEBUTTONS;i++)
    {
      flag = G_CheckDoubleClick(gamekeydown[KEY_MOUSE1+i], &mousedclicks[i]);
      gamekeydown[KEY_DBLMOUSE1+i] = flag;
    }

  /* FIXME, removed. Do we even want to have joystick double clicks?
  for (i=0;i<JOYBUTTONS;i++)
    {
      flag = G_CheckDoubleClick(gamekeydown[KEY_JOY1+i], &joydclicks[i]);
      gamekeydown[KEY_DBLJOY1+i] = flag;
    }
  */
  return false; // let them filter through
}



struct keyname_t
{
  int  keynum;
  char name[20];
};

static keyname_t keynames[] =
{
  {KEY_BACKSPACE, "backspace"},
  {KEY_TAB,       "tab"},
  {KEY_ENTER,     "enter"},
  {KEY_PAUSE,     "pause"},  // irrelevant, since this key cannot be remapped...
  {KEY_ESCAPE,    "escape"}, // likewise
  {KEY_SPACE,     "space"},

  {KEY_CONSOLE,    "console"},

  {KEY_NUMLOCK,    "num lock"},
  {KEY_CAPSLOCK,   "caps lock"},
  {KEY_SCROLLLOCK, "scroll lock"},
  {KEY_RSHIFT,     "right shift"},
  {KEY_LSHIFT,     "left shift"},
  {KEY_RCTRL,      "right ctrl"},
  {KEY_LCTRL,      "left ctrl"},
  {KEY_RALT,       "right alt"},
  {KEY_LALT,       "left alt"},
  {KEY_LWIN, "left win"},
  {KEY_RWIN, "right win"},
  {KEY_MODE, "AltGr"},
  {KEY_MENU, "menu"},

  // keypad keys
  {KEY_KEYPAD0, "keypad 0"},
  {KEY_KEYPAD1, "keypad 1"},
  {KEY_KEYPAD2, "keypad 2"},
  {KEY_KEYPAD3, "keypad 3"},
  {KEY_KEYPAD4, "keypad 4"},
  {KEY_KEYPAD5, "keypad 5"},
  {KEY_KEYPAD6, "keypad 6"},
  {KEY_KEYPAD7, "keypad 7"},
  {KEY_KEYPAD8, "keypad 8"},
  {KEY_KEYPAD9, "keypad 9"},
  {KEY_KPADPERIOD,"keypad ."},
  {KEY_KPADSLASH, "keypad /"},
  {KEY_KPADMULT,  "keypad *"},
  {KEY_MINUSPAD,  "keypad -"},
  {KEY_PLUSPAD,   "keypad +"},

  // extended keys (not keypad)
  {KEY_UPARROW,   "up arrow"},
  {KEY_DOWNARROW, "down arrow"},
  {KEY_RIGHTARROW,"right arrow"},
  {KEY_LEFTARROW, "left arrow"},
  {KEY_INS,       "ins"},
  {KEY_DELETE,    "del"},
  {KEY_HOME,      "home"},
  {KEY_END,       "end"},
  {KEY_PGUP,      "pgup"},
  {KEY_PGDN,      "pgdown"},

  // other keys
  {KEY_F1, "F1"},
  {KEY_F2, "F2"},
  {KEY_F3, "F3"},
  {KEY_F4, "F4"},
  {KEY_F5, "F5"},
  {KEY_F6, "F6"},
  {KEY_F7, "F7"},
  {KEY_F8, "F8"},
  {KEY_F9, "F9"},
  {KEY_F10,"F10"},
  {KEY_F11,"F11"},
  {KEY_F12,"F12"},

  // virtual keys for mouse buttons and joystick buttons
  {KEY_MOUSE1,  "mouse 1"},
  {KEY_MOUSE1+1,"mouse 2"},
  {KEY_MOUSE1+2,"mouse 3"},
  {KEY_MOUSEWHEELUP, "mwheel up"},
  {KEY_MOUSEWHEELDOWN,"mwheel down"},
  {KEY_MOUSE1+5,"mouse 6"},
  {KEY_MOUSE1+6,"mouse 7"},
  {KEY_MOUSE1+7,"mouse 8"},
  {KEY_2MOUSE1,  "2nd mouse 2"},    //BP: sorry my mouse handler swap button 1 and 2
  {KEY_2MOUSE1+1,"2nd mouse 1"},
  {KEY_2MOUSE1+2,"2nd mouse 3"},
  {KEY_2MOUSEWHEELUP,"2nd mwheel up"},
  {KEY_2MOUSEWHEELDOWN,"2nd mwheel down"},
  {KEY_2MOUSE1+5,"2nd mouse 6"},
  {KEY_2MOUSE1+6,"2nd mouse 7"},
  {KEY_2MOUSE1+7,"2nd mouse 8"},

  {KEY_DBLMOUSE1,   "mouse 1 d"},
  {KEY_DBLMOUSE1+1, "mouse 2 d"},
  {KEY_DBLMOUSE1+2, "mouse 3 d"},
  {KEY_DBLMOUSE1+3, "mouse 4 d"},
  {KEY_DBLMOUSE1+4, "mouse 5 d"},
  {KEY_DBLMOUSE1+5, "mouse 6 d"},
  {KEY_DBLMOUSE1+6, "mouse 7 d"},
  {KEY_DBLMOUSE1+7, "mouse 8 d"},
  {KEY_DBL2MOUSE1,  "2nd mouse 2 d"},  //BP: sorry my mouse handler swap button 1 and 2
  {KEY_DBL2MOUSE1+1,"2nd mouse 1 d"},
  {KEY_DBL2MOUSE1+2,"2nd mouse 3 d"},
  {KEY_DBL2MOUSE1+3,"2nd mouse 4 d"},
  {KEY_DBL2MOUSE1+4,"2nd mouse 5 d"},
  {KEY_DBL2MOUSE1+5,"2nd mouse 6 d"},
  {KEY_DBL2MOUSE1+6,"2nd mouse 7 d"},
  {KEY_DBL2MOUSE1+7,"2nd mouse 8 d"},

  {KEY_JOY0BUT0, "Joy 0 btn 0"},
  {KEY_JOY0BUT1, "Joy 0 btn 1"},
  {KEY_JOY0BUT2, "Joy 0 btn 2"},
  {KEY_JOY0BUT3, "Joy 0 btn 3"},
  {KEY_JOY0BUT4, "Joy 0 btn 4"},
  {KEY_JOY0BUT5, "Joy 0 btn 5"},
  {KEY_JOY0BUT6, "Joy 0 btn 6"},
  {KEY_JOY0BUT7, "Joy 0 btn 7"},
  {KEY_JOY0BUT8, "Joy 0 btn 8"},
  {KEY_JOY0BUT9, "Joy 0 btn 9"},
  {KEY_JOY0BUT10, "Joy 0 btn 10"},
  {KEY_JOY0BUT11, "Joy 0 btn 11"},
  {KEY_JOY0BUT12, "Joy 0 btn 12"},
  {KEY_JOY0BUT13, "Joy 0 btn 13"},
  {KEY_JOY0BUT14, "Joy 0 btn 14"},
  {KEY_JOY0BUT15, "Joy 0 btn 15"},

  {KEY_JOY1BUT0, "Joy 1 btn 0"},
  {KEY_JOY1BUT1, "Joy 1 btn 1"},
  {KEY_JOY1BUT2, "Joy 1 btn 2"},
  {KEY_JOY1BUT3, "Joy 1 btn 3"},
  {KEY_JOY1BUT4, "Joy 1 btn 4"},
  {KEY_JOY1BUT5, "Joy 1 btn 5"},
  {KEY_JOY1BUT6, "Joy 1 btn 6"},
  {KEY_JOY1BUT7, "Joy 1 btn 7"},
  {KEY_JOY1BUT8, "Joy 1 btn 8"},
  {KEY_JOY1BUT9, "Joy 1 btn 9"},
  {KEY_JOY1BUT10, "Joy 1 btn 10"},
  {KEY_JOY1BUT11, "Joy 1 btn 11"},
  {KEY_JOY1BUT12, "Joy 1 btn 12"},
  {KEY_JOY1BUT13, "Joy 1 btn 13"},
  {KEY_JOY1BUT14, "Joy 1 btn 14"},
  {KEY_JOY1BUT15, "Joy 1 btn 15"},

  {KEY_JOY2BUT0, "Joy 2 btn 0"},
  {KEY_JOY2BUT1, "Joy 2 btn 1"},
  {KEY_JOY2BUT2, "Joy 2 btn 2"},
  {KEY_JOY2BUT3, "Joy 2 btn 3"},
  {KEY_JOY2BUT4, "Joy 2 btn 4"},
  {KEY_JOY2BUT5, "Joy 2 btn 5"},
  {KEY_JOY2BUT6, "Joy 2 btn 6"},
  {KEY_JOY2BUT7, "Joy 2 btn 7"},
  {KEY_JOY2BUT8, "Joy 2 btn 8"},
  {KEY_JOY2BUT9, "Joy 2 btn 9"},
  {KEY_JOY2BUT10, "Joy 2 btn 10"},
  {KEY_JOY2BUT11, "Joy 2 btn 11"},
  {KEY_JOY2BUT12, "Joy 2 btn 12"},
  {KEY_JOY2BUT13, "Joy 2 btn 13"},
  {KEY_JOY2BUT14, "Joy 2 btn 14"},
  {KEY_JOY2BUT15, "Joy 2 btn 15"},

  {KEY_JOY3BUT0, "Joy 3 btn 0"},
  {KEY_JOY3BUT1, "Joy 3 btn 1"},
  {KEY_JOY3BUT2, "Joy 3 btn 2"},
  {KEY_JOY3BUT3, "Joy 3 btn 3"},
  {KEY_JOY3BUT4, "Joy 3 btn 4"},
  {KEY_JOY3BUT5, "Joy 3 btn 5"},
  {KEY_JOY3BUT6, "Joy 3 btn 6"},
  {KEY_JOY3BUT7, "Joy 3 btn 7"},
  {KEY_JOY3BUT8, "Joy 3 btn 8"},
  {KEY_JOY3BUT9, "Joy 3 btn 9"},
  {KEY_JOY3BUT10, "Joy 3 btn 10"},
  {KEY_JOY3BUT11, "Joy 3 btn 11"},
  {KEY_JOY3BUT12, "Joy 3 btn 12"},
  {KEY_JOY3BUT13, "Joy 3 btn 13"},
  {KEY_JOY3BUT14, "Joy 3 btn 14"},
  {KEY_JOY3BUT15, "Joy 3 btn 15"},
};

char *gamecontrolname[num_gamecontrols] =
{
  "nothing",        //a key/button mapped to gc_null has no effect
  "forward",
  "backward",
  "strafe",
  "straferight",
  "strafeleft",
  "speed",
  "turnleft",
  "turnright",
  "lookup",
  "lookdown",
  "centerview",
  "mouseaiming",
  "fire",
  "use",
  "jump",
  "flydown",
  "weapon1",
  "weapon2",
  "weapon3",
  "weapon4",
  "weapon5",
  "weapon6",
  "weapon7",
  "weapon8",
  "nextweapon",
  "prevweapon",
  "bestweapon",
  "inventorynext",
  "inventoryprev",
  "inventoryuse"
};

static const int NUMKEYNAMES = sizeof(keynames)/sizeof(keyname_t);

//
//  Detach any keys associated to the given game control
//  - pass the pointer to the gamecontrol table for the player being edited
void  G_ClearControlKeys(short (*setup_gc)[2], int control)
{
  setup_gc[control][0] = KEY_NULL;
  setup_gc[control][1] = KEY_NULL;
}

//
//  Returns the name of a key (or virtual key for mouse and joy)
//  the input value being an keynum
//
char *G_KeynumToString(int keynum)
{
  static char keynamestr[8];

  // return a string with the ascii char if displayable
  if (keynum > ' ' && keynum <= 'z' && keynum != KEY_CONSOLE)
    {
      keynamestr[0] = keynum;
      keynamestr[1] = '\0';
      return keynamestr;
    }

  // find a description for special keys
  for (int j=0; j<NUMKEYNAMES; j++)
    if (keynames[j].keynum == keynum)
      return keynames[j].name;

  // create a name for Unknown key
  sprintf(keynamestr, "key%d", keynum);
  return keynamestr;
}


int G_KeyStringtoNum(char *keystr)
{
  //    strupr(keystr);

  if (keystr[1] == '\0' && keystr[0] > ' ' && keystr[0] <= 'z')
    return keystr[0];

  for (int j=0; j<NUMKEYNAMES; j++)
    if (!strcasecmp(keynames[j].name, keystr))
      return keynames[j].keynum;

  if (strlen(keystr) > 3 && !strncasecmp(keystr, "key", 3))
    return atoi(&keystr[3]); // for unnamed keys (see G_KeynumToString)

  return 0;
}

void G_Controldefault()
{
  short (*gc)[2] = gamecontrol[0];
  gc[gc_forward    ][0]=KEY_UPARROW;
  gc[gc_forward    ][1]=KEY_MOUSE1+2;
  gc[gc_backward   ][0]=KEY_DOWNARROW;
  gc[gc_strafe     ][0]=KEY_RALT;
  gc[gc_strafe     ][1]=KEY_MOUSE1+1;
  gc[gc_straferight][0]='.';
  gc[gc_strafeleft ][0]=',';
  gc[gc_speed      ][0]=KEY_RSHIFT;
  gc[gc_turnleft   ][0]=KEY_LEFTARROW;
  gc[gc_turnright  ][0]=KEY_RIGHTARROW;
  gc[gc_fire       ][0]=KEY_RCTRL;
  gc[gc_fire       ][1]=KEY_MOUSE1;
  gc[gc_use        ][0]=KEY_SPACE;
  gc[gc_lookup     ][0]=KEY_PGUP;
  gc[gc_lookdown   ][0]=KEY_PGDN;
  gc[gc_centerview ][0]=KEY_END;
  gc[gc_mouseaiming][0]='s';
  gc[gc_weapon1    ][0]='1';
  gc[gc_weapon2    ][0]='2';
  gc[gc_weapon3    ][0]='3';
  gc[gc_weapon4    ][0]='4';
  gc[gc_weapon5    ][0]='5';
  gc[gc_weapon6    ][0]='6';
  gc[gc_weapon7    ][0]='7';
  gc[gc_weapon8    ][0]='8';
  gc[gc_jump       ][0]='/';
  //gc[gc_nextweapon ][1]=KEY_JOY0BUT4;
  //gc[gc_prevweapon ][1]=KEY_JOY0BUT5;
  gc[gc_invnext    ][0] = ']';
  gc[gc_invprev    ][0] = '[';
  gc[gc_invuse     ][0] = KEY_ENTER;
  gc[gc_jump       ][0] = KEY_INS;
  gc[gc_flydown    ][0] = KEY_KPADPERIOD;

  //gc[gc_nextweapon ][0]=']';
  //gc[gc_prevweapon ][0]='[';

  // common game keys
  commoncontrols[gk_talk][0]    = 't';
  commoncontrols[gk_console][0] = KEY_CONSOLE;
  commoncontrols[gk_scores][0]  = 'f';
}


void G_SaveKeySetting(FILE *f)
{
  for (int j = 0; j < 2; j++)
    for (int i = 1; i < num_gamecontrols; i++)
      //if (gamecontrol[j][i][0])
	{
	  fprintf(f,"setcontrol %d \"%s\" \"%s\"", j, gamecontrolname[i],
		  G_KeynumToString(gamecontrol[j][i][0]));

	  if (gamecontrol[j][i][1])
	    fprintf(f," \"%s\"\n", G_KeynumToString(gamecontrol[j][i][1]));
	  else
	    fprintf(f,"\n");
	}
}


//! Writes the axis binding commands to the config file.
void G_SaveJoyAxisBindings(FILE *f)
{
  for (unsigned i=0; i<joybindings.size(); i++)
    {
      joybinding_t j = joybindings[i];
      fprintf(f, "bindjoyaxis %d %d %d %d %f\n",
	      j.playnum, j.joynum, j.axisnum, int(j.action), j.scale);
    }
}


void G_CheckDoubleUsage(int keynum)
{
  if (cv_controlperkey.value == 1)
    for (int i=0; i<2; i++)
      for (int j=0; j<num_gamecontrols; j++)
	for (int k=0; k<2; k++)
	  if (gamecontrol[i][j][k] == keynum)
	    gamecontrol[i][j][k] = KEY_NULL;
}



void Command_Setcontrol_f()
{
  int na = COM_Argc();

  if (na < 4 || na > 5)
    {
      CONS_Printf ("setcontrol <playernum> <controlname> <keyname> [2nd keyname]\n");
      return;
    }

  int p = atoi(COM_Argv(1));
  short (*gc)[2] = gamecontrol[p % 2];

  char *cname = COM_Argv(2);

  int i;
  for (i = 0; i < num_gamecontrols && strcasecmp(cname, gamecontrolname[i]); i++)
    ;

  if (i == num_gamecontrols)
    {
      CONS_Printf("Control '%s' unknown\n", cname);
      return;
    }

  int keynum = G_KeyStringtoNum(COM_Argv(3));
  G_CheckDoubleUsage(keynum);
  gc[i][0] = keynum;

  if (na == 5)
    gc[i][1] = G_KeyStringtoNum(COM_Argv(4));
  else
    gc[i][1] = 0;
}



//! Magically converts a console command to a joystick axis binding.
void Command_BindJoyaxis_f()
{
  joybinding_t j;
  unsigned int i;

  int na = COM_Argc();

  if(na == 1) { // Print bindings.
    if(joybindings.size() == 0) {
      CONS_Printf("No joystick axis bindings defined.\n");
      return;
    }
    CONS_Printf("Current axis bindings.\n");
    for(unsigned int i=0; i<joybindings.size(); i++) {
      j = joybindings[i];
      CONS_Printf("%d %d %d %d %f\n", j.playnum, j.joynum, j.axisnum,
		  (int)j.action, j.scale);
    }
    return;
  }

  if (na < 5)
    {
    CONS_Printf("bindjoyaxis [playnum] [joynum] [axisnum] [action] [scale]\n");
    return;
  }

  j.playnum = atoi(COM_Argv(1));
  j.joynum  = atoi(COM_Argv(2));
  j.axisnum = atoi(COM_Argv(3));
  j.action  = joyactions_e(atoi(COM_Argv(4)));
  if (na == 6)
    j.scale = atof(COM_Argv(5));
  else
    j.scale = 1.0f;

  // Check the validity of the binding.
  if(j.joynum < 0 || j.joynum >= (int)joysticks.size()) {
    CONS_Printf("Attemting to bind non-existant joystick %d.\n", j.joynum);
    return;
  }
  if(j.axisnum < 0 || j.axisnum >= SDL_JoystickNumAxes(joysticks[j.joynum])) {
    CONS_Printf("Attemting to bind non-existant axis %d.\n", j.axisnum);
    return;
  }
  if(j.action < 0 || j.action >= num_joyactions) {
    CONS_Printf("Attemting to bind non-existant action %d.\n", int(j.action));
    return;
  }

  // Overwrite existing binding, if any. Otherwise just append.
  for(i=0; i<joybindings.size(); i++) {
    joybinding_t j2 = joybindings[i];
    if(j2.joynum == j.joynum && j2.axisnum == j.axisnum) {
      joybindings[i] = j;
      CONS_Printf("Joystick binding modified.\n");
      return;
    }
  }
  joybindings.push_back(j);
  CONS_Printf("Joystick binding added.\n");
}

//! Unbind the specified joystick axises.
/*! Takes zero to two parameters. The first one is the joystick number
  and the second is the axis number. If either is not specified, all
  values are assumed to match. When called without parameters, all
  bindings are removed.
*/

void Command_UnbindJoyaxis_f()
{
  int joynum  = -1;
  int axisnum = -1;
  int na = COM_Argc();
  vector<joybinding_t> newbind;

  if(joybindings.size() == 0) {
    CONS_Printf("No bindings to unset.\n");
    return;
  }

  if(na > 3) {
    CONS_Printf("unbindjoyaxis [joynum] [axisnum]\n");
    return;
  }

  // Does the user specify axis or joy number?
  if(na > 2)
    axisnum = atoi(COM_Argv(2));
  if(na > 1)
    joynum = atoi(COM_Argv(1));

  for(unsigned int i=0; i<joybindings.size(); i++) {
    joybinding_t j = joybindings[i];
    if((joynum == -1 || joynum == j.joynum) &&
       (axisnum == -1 || axisnum == j.axisnum))
      continue; // We have a binding to prune.
    newbind.push_back(j);
  }

  // We have the new bindings.
  if(newbind.size() == joybindings.size()) {
    CONS_Printf("No bindings matched the parameters.\n");
    return;
  }
  joybindings = newbind;
}
