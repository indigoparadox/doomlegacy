// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2004 by DooM Legacy Team.
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
// Revision 1.6  2004/08/12 18:30:30  smite-meister
// cleaned startup
//
// Revision 1.5  2003/11/12 11:07:27  smite-meister
// Serialization done. Map progression.
//
// Revision 1.4  2003/06/10 22:40:01  smite-meister
// Bugfixes
//
// Revision 1.3  2003/03/15 20:07:21  smite-meister
// Initial Hexen compatibility!
//
// Revision 1.2  2002/12/29 18:57:03  smite-meister
// MAPINFO implemented, Actor deaths handled better
//
// Revision 1.1.1.1  2002/11/16 14:18:29  hurdler
// Initial C++ version of Doom Legacy
//
// Revision 1.7  2002/09/25 15:17:43  vberghol
// Intermission fixed?
//
// Revision 1.6  2002/08/21 16:58:36  vberghol
// Version 1.41 Experimental compiles and links!
//
// Revision 1.5  2002/07/18 19:16:42  vberghol
// renamed a few files
//
// Revision 1.4  2002/07/01 21:00:58  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.3  2002/07/01 15:01:59  vberghol
// HUD alkaa olla kunnossa
//
// Revision 1.3  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//-----------------------------------------------------------------------------

/// \file
/// \brief Intermission handler.

#ifndef wi_stuff_h
#define wi_stuff_h 1

#include <vector>
#include "doomdef.h"

using namespace std;


struct fragsort_t
{
  int  count;
  int  num;
  int  color;
  const char *name;
};


/// \brief Intermission handler
///
/// A one-instance class for Winning/Intermission animations etc.
/// could be done using static functions, but I happen to like classes;)
/// NOTE: during the intermission players may enter and leave the game.
/// players who enter do not show up in the stats listing
/// players who leave are not removed until the next level starts, so no problem there.

class Intermission
{
private:
  /// States for the intermission
  enum WIstate_e
  {
    StatCount,
    ShowNextLoc,
    Wait
  };

  /// specifies current state
  WIstate_e state;

  // player wants to accelerate or skip a stage
  bool acceleratestage;

  /// What animation, if any, do we show? Applicable in Doom1 and Heretic, otherwise zero.
  int episode;

  const char *interpic;
  const char *lastlevelname;
  const char *nextlevelname;

  // level numbers for old Doom style graphic levelnames and YAH's
  int next, last; 

  // used for general timing and bg animation timing
  int count, bcount;

  // blinking pointer
  bool pointeron;

  // DM stats
  int nplayers;
  fragsort_t *dm_score[4];

  // Coop stats
  int  count_stage;
  bool dofrags;
  struct statcounter_t
  {
    int kills;
    int items;
    int secrets;
    int frags;
  };

  // counters for each player
  vector<statcounter_t> cnt;
  // players that participate in the intermission
  vector<class PlayerInfo *> plrs;

  // level totals
  statcounter_t total;

  int  cnt_time, cnt_par;
  int  time, partime;

  int s_count; /// counting sound id


  void LoadData();
  void UnloadData();

  /// draw background
  void SlamBackground();

  /// background animations (Doom 1 only)
  void UpdateAnimatedBack();

  void InitCoopStats();
  void UpdateCoopStats();
  void DrawCoopStats();

  void InitDMStats();
  void UpdateDMStats();
  void DrawDMStats();

  void DrawYAH();

  void InitWait();

public:

  /// starts the intermission
  void Start();

  /// the intermission is ended when the server says so
  void End();

  /// accelerate stage?
  bool Responder(struct event_t *ev);

  /// Updates stuff each tick
  void Ticker();

  /// well.
  void Drawer();
};

extern Intermission wi;


// draw rankings
void WI_drawRanking(const char *title,int x,int y,fragsort_t *fragtable,
		    int scorelines, bool large, int white);

#endif
