// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2002 by DooM Legacy Team.
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
// Revision 1.8  2003/02/16 16:54:50  smite-meister
// L2 sound cache done
//
// Revision 1.7  2003/01/25 21:33:05  smite-meister
// Now compiles with MinGW 2.0 / GCC 3.2.
// Builder can choose between dynamic and static linkage.
//
// Revision 1.6  2003/01/18 20:17:41  smite-meister
// HUD fixed, levelchange crash fixed.
//
// Revision 1.5  2002/12/29 18:57:03  smite-meister
// MAPINFO implemented, Actor deaths handled better
//
// Revision 1.4  2002/12/23 23:15:41  smite-meister
// Weapon groups, MAPINFO parser added!
//
// Revision 1.3  2002/12/16 22:11:21  smite-meister
// Actor/DActor separation done!
//
// Revision 1.2  2002/12/03 10:11:39  smite-meister
// Blindness and missile clipping bugs fixed
//
//
// DESCRIPTION:
//      Cheat sequence checking.
//
//-----------------------------------------------------------------------------
 

#include "tables.h"
#include "dstrings.h"

#include "m_cheat.h"
#include "g_game.h"
#include "g_map.h"
#include "g_player.h"
#include "d_event.h"

#include "m_cheat.h"
#include "am_map.h"
#include "i_sound.h" // for I_PlayCD()
#include "sounds.h"
#include "s_sound.h"
#include "hu_stuff.h"
#include "w_wad.h"
#include "g_pawn.h"

// ==========================================================================
//                             CHEAT Structures
// ==========================================================================

byte   cheat_mus_seq[] =
{
  //0xb2, 0x26, 0xb6, 0xae, 0xea, 0, 0, 0xff // idmus__
  'i', 'd', 'm', 'u', 's', 0, 0, 0xff
};

byte   cheat_cd_seq[] =
{
  //0xb2, 0x26, 0xe2, 0x26, 0, 0, 0xff // idcd__
  'i', 'd', 'c', 'd', 0, 0, 0xff
};

byte   cheat_choppers_seq[] =
{
  //0xb2, 0x26, 0xe2, 0x32, 0xf6, 0x2a, 0x2a, 0xa6, 0x6a, 0xea, 0xff // idchoppers
  'i', 'd', 'c', 'h', 'o', 'p', 'p', 'e', 'r', 's', 0xff
};

byte   cheat_god_seq[] =
{
  //0xb2, 0x26, 0x26, 0xaa, 0x26, 0xff  // iddqd
  'i', 'd', 'd', 'q', 'd', 0xff
};


byte   cheat_ammo_seq[] =
{
  //0xb2, 0x26, 0xf2, 0x66, 0xa2, 0xff  // idkfa
  'i', 'd', 'k', 'f', 'a', 0xff
};

byte   cheat_ammonokey_seq[] =
{
  //0xb2, 0x26, 0x66, 0xa2, 0xff        // idfa
  'i', 'd', 'f', 'a', 0xff 
};


// Smashing Pumpkins Into Small Pieces Of Putrid Debris.
byte   cheat_noclip_seq[] =
{
  //0xb2, 0x26, 0xea, 0x2a, 0xb2,       // idspispopd
  //0xea, 0x2a, 0xf6, 0x2a, 0x26, 0xff
  'i', 'd', 's', 'p', 'i', 's', 'p', 'o', 'p', 'd', 0xff
};

byte   cheat_commercial_noclip_seq[] =
{
  //0xb2, 0x26, 0xe2, 0x36, 0xb2, 0x2a, 0xff    // idclip
  'i', 'd', 'c', 'l', 'i', 'p', 0xff
};

//added:28-02-98: new cheat to fly around levels using jump !!
byte   cheat_fly_around_seq[] =
{
  'i', 'd', 'f', 'l', 'y', 0xff // idfly
};

/*
byte   cheat_powerup_seq[7][10] =
{
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6e, 0xff },     // beholdv
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xea, 0xff },     // beholds
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xb2, 0xff },     // beholdi
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6a, 0xff },     // beholdr
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xa2, 0xff },     // beholda
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x36, 0xff },     // beholdl
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xff }            // behold
};
*/

// idbehold message
byte   cheat_powerup_seq1[] =
{
  //0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xff // idbehold
  'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 0xff
};

// actual cheat
byte   cheat_powerup_seq2[] =
{
  //0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0, 0xff // idbehold_
  'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 0, 0xff
};

byte   cheat_clev_seq[] =
{
  //0xb2, 0x26,  0xe2, 0x36, 0xa6, 0x6e, 0, 0, 0xff  // idclev__
  'i', 'd', 'c', 'l', 'e', 'v', 0, 0, 0xff
};

// my position cheat
byte   cheat_mypos_seq[] =
{
  //0xb2, 0x26, 0xb6, 0xba, 0x2a, 0xf6, 0xea, 0xff      // idmypos
  'i', 'd', 'm', 'y', 'p', 'o', 's', 0xff
};

byte cheat_amap_seq[] =
{
  //0xb2, 0x26, 0x26, 0x2e, 0xff // iddt
  'i', 'd', 'd', 't', 0xff
};



// ==========================================================================
//                        CHEAT SEQUENCE PACKAGE
// ==========================================================================

//static byte    cheat_xlate_table[256];

void cht_Init()
{
  //int i;
  //for (i=0;i<256;i++) 
  //  cheat_xlate_table[i] = SCRAMBLE(i);
}

// added 2-2-98 for compatibility with dehacked

int idfa_armor=200;
int idfa_armor_class=2;
int idkfa_armor=200;
int idkfa_armor_class=2;
int god_health=100;


// command that can be typed at the console !

void Command_CheatNoClip_f()
{
  if (game.multiplayer)
    return;

  PlayerPawn *p = consoleplayer->pawn;
  if (p == NULL) return;

  p->cheats ^= CF_NOCLIP;

  if (p->cheats & CF_NOCLIP)
    CONS_Printf (STSTR_NCON);
  else
    CONS_Printf (STSTR_NCOFF);
}

void Command_CheatGod_f()
{
  if (game.multiplayer)
    return;

  PlayerPawn *p = consoleplayer->pawn;
  if (p == NULL) return;

  p->cheats ^= CF_GODMODE;
  if (p->cheats & CF_GODMODE)
    {
      p->health = god_health;
      CONS_Printf ("%s\n", STSTR_DQDON);
    }
  else
    CONS_Printf ("%s\n", STSTR_DQDOFF);
}

void Command_CheatGimme_f()
{
  // TODO! "gimme eat" gives a little health and lots of "honor" in Doom Fortress :D
  char*     s;
  int       i,j;

  if (game.multiplayer)
    return;

  if (COM_Argc()<2)
    {
      CONS_Printf ("gimme [health] [ammo] [armor] ...\n");
      return;
    }

  PlayerPawn* p = consoleplayer->pawn;
  if (p == NULL) return;

  for (i=1; i<COM_Argc(); i++) {
    s = COM_Argv(i);

    if (!strncmp(s,"health",6))
      {
	p->health = god_health;
	CONS_Printf("got health\n");
      }
    else if (!strncmp(s,"ammo",4))
      {
	for (j=0;j<NUMAMMO;j++)
	  p->ammo[j] = p->maxammo[j];

	CONS_Printf("got ammo\n");
      }
    else if (!strncmp(s,"armor",5))
      {
	p->armorpoints = idfa_armor;
	p->armortype = idfa_armor_class;

	CONS_Printf("got armor\n");
      }
    else if (!strncmp(s,"keys",4))
      {
	p->cards = it_allkeys;

	CONS_Printf("got keys\n");
      }
    else if (!strncmp(s,"weapons",7))
      {
	for (j=0;j<NUMWEAPONS;j++)
	  p->weaponowned[j] = true;

	for (j=0;j<NUMAMMO;j++)
	  p->ammo[j] = p->maxammo[j];

	CONS_Printf("got weapons\n");
      }
    else if (!strncmp(s,"chainsaw",8))
      //
      // WEAPONS
      //
      {
	p->weaponowned[wp_chainsaw] = true;

	CONS_Printf("got chainsaw\n");
      }
    else if (!strncmp(s,"shotgun",7))
      {
	p->weaponowned[wp_shotgun] = true;
	p->ammo[am_shell] = p->maxammo[am_shell];

	CONS_Printf("got shotgun\n");
      }
    else if (!strncmp(s,"supershotgun",12))
      {
	if (game.mode == gm_doom2) // only in Doom2
	  {
	    p->weaponowned[wp_supershotgun] = true;
	    p->ammo[am_shell] = p->maxammo[am_shell];

	    CONS_Printf("got super shotgun\n");
	  }
      }
    else if (!strncmp(s,"rocket",6))
      {
	p->weaponowned[wp_missile] = true;
	p->ammo[am_misl] = p->maxammo[am_misl];

	CONS_Printf("got rocket launcher\n");
      }
    else if (!strncmp(s,"plasma",6))
      {
	p->weaponowned[wp_plasma] = true;
	p->ammo[am_cell] = p->maxammo[am_cell];

	CONS_Printf("got plasma\n");
      }
    else if (!strncmp(s,"bfg",3))
      {
	p->weaponowned[wp_bfg] = true;
	p->ammo[am_cell] = p->maxammo[am_cell];

	CONS_Printf("got bfg\n");
      }
    else if (!strncmp(s,"chaingun",8))
      {
	p->weaponowned[wp_chaingun] = true;
	p->ammo[am_clip] = p->maxammo[am_clip];

	CONS_Printf("got chaingun\n");
      }
    else if (!strncmp(s,"berserk",7))
      //
      // SPECIAL ITEMS
      //
      {
	if (!p->powers[pw_strength])
	  p->GivePower(pw_strength);
	CONS_Printf("got berserk strength\n");
      }
    //22/08/99: added by Hurdler
    else if (!strncmp(s,"map",3))
      {
	automap.am_cheating = 1;
	CONS_Printf("got map\n");
      }
    //
    else if (!strncmp(s,"fullmap",7))
      {
	automap.am_cheating = 2;
	CONS_Printf("got map and things\n");
      }
    else
      CONS_Printf ("can't give '%s' : unknown\n", s);
  }
}

// a class for handling cheat sequences
class TCheat
{
  typedef void (* fp)(PlayerPawn *p, const byte *arg);
private:
  fp func;
  byte *seq;
  byte *pos;
  byte args[2];
  byte currarg;

public:

  TCheat(fp f, byte *s);
  bool AddKey(byte key, bool *eat);

  friend bool cht_Responder(event_t* ev);
};


// constructor
TCheat::TCheat(fp f, byte *s)
{
  func = f;
  seq = pos = s;
  args[0] = args[1] = 0;
  currarg = 0;
}

// returns true if sequence is completed
bool TCheat::AddKey(byte key, bool *eat)
{
  if(*pos == 0)
    {
      // read a parameter
      *eat = true;
      args[currarg++] = key;
      pos++;
    }
  //else if (cheat_xlate_table[key] == *pos)
  else if (key == *pos)
    {
      // correct key, go on
      pos++;
    }
  else
    {
      // wrong key, reset sequence
      pos = seq;
      currarg = 0;
    }

  if(*pos == 0xff)
    {
      // sequence complete!
      pos = seq;
      currarg = 0;
      return true;
    }

  return false;
}


//static bool CheatAddKey(Cheat_t *cheat, byte key, bool *eat);
void CheatFlyFunc(PlayerPawn *p, const byte *arg);
void CheatCDFunc(PlayerPawn *p, const byte *arg);
void CheatMyPosFunc(PlayerPawn *p, const byte *arg);

static void CheatAMFunc(PlayerPawn *p, const byte *arg);
static void CheatMusFunc(PlayerPawn *p, const byte *arg);
static void CheatChopFunc(PlayerPawn *p, const byte *arg);
static void CheatPowerup1Func(PlayerPawn *p, const byte *arg);
static void CheatPowerup2Func(PlayerPawn *p, const byte *arg);

static void CheatGodFunc(PlayerPawn *p, const byte *arg);
static void CheatNoClipFunc(PlayerPawn *p, const byte *arg);
static void CheatWeaponsFunc(PlayerPawn *p, const byte *arg);
static void CheatPowerFunc(PlayerPawn *p, const byte *arg);
static void CheatHealthFunc(PlayerPawn *p, const byte *arg);
static void CheatKeysFunc(PlayerPawn *p, const byte *arg);
//static void CheatSoundFunc(PlayerPawn *p, const byte *arg);
static void CheatTickerFunc(PlayerPawn *p, const byte *arg);
static void CheatArtifact1Func(PlayerPawn *p, const byte *arg);
static void CheatArtifact2Func(PlayerPawn *p, const byte *arg);
static void CheatArtifact3Func(PlayerPawn *p, const byte *arg);
static void CheatWarpFunc(PlayerPawn *p, const byte *arg);
static void CheatChickenFunc(PlayerPawn *p, const byte *arg);
static void CheatMassacreFunc(PlayerPawn *p, const byte *arg);
static void CheatIDKFAFunc(PlayerPawn *p, const byte *arg);
static void CheatIDDQDFunc(PlayerPawn *p, const byte *arg);




// Toggle god mode
static byte CheatGodSeq[] =
{
  'q', 'u', 'i', 'c', 'k', 'e', 'n', 0xff
};

// Toggle no clipping mode
static byte CheatNoClipSeq[] =
{
  'k', 'i', 't', 't', 'y', 0xff
};

// Get all weapons and ammo
static byte CheatWeaponsSeq[] =
{
  'r', 'a', 'm', 'b', 'o', 0xff
};

// Toggle tome of power
static byte CheatPowerSeq[] =
{
  's', 'h', 'a', 'z', 'a', 'm', 0xff, 0
};

// Get full health
static byte CheatHealthSeq[] =
{
  'p', 'o', 'n', 'c', 'e', 0xff
};

// Get all keys
static byte CheatKeysSeq[] =
{
  's', 'k', 'e', 'l', 0xff, 0
};

// Toggle ticker
static byte CheatTickerSeq[] =
{
  't', 'i', 'c', 'k', 'e', 'r', 0xff, 0
};

// Get an artifact 1st stage (ask for type)
static byte CheatArtifact1Seq[] =
{
  'g', 'i', 'm', 'm', 'e', 0xff
};

// Get an artifact 2nd stage (ask for count)
static byte CheatArtifact2Seq[] =
{
  'g', 'i', 'm', 'm', 'e', 0, 0xff, 0
};

// Get an artifact final stage
static byte CheatArtifact3Seq[] =
{
  'g', 'i', 'm', 'm', 'e', 0, 0, 0xff
};

// Warp to new level
static byte CheatWarpSeq[] =
{
  'e', 'n', 'g', 'a', 'g', 'e', 0, 0, 0xff, 0
};

// Save a screenshot
static byte CheatChickenSeq[] =
{
  'c', 'o', 'c', 'k', 'a', 'd', 'o', 'o', 'd', 'l', 'e', 'd', 'o', 'o', 0xff, 0
};

// Kill all monsters
static byte CheatMassacreSeq[] =
{
  'm', 'a', 's', 's', 'a', 'c', 'r', 'e', 0xff, 0
};

static byte CheatIDKFASeq[] =
{
  'i', 'd', 'k', 'f', 'a', 0xff, 0
};

static byte CheatIDDQDSeq[] =
{
  'i', 'd', 'd', 'q', 'd', 0xff, 0
};


// Cheat lists _must_ be ended with a TCheat(NULL, ...) terminator for now

// universal cheats which work in every game mode (begin with id...)
static TCheat Basic_Cheats[] = {
  TCheat(CheatFlyFunc, cheat_fly_around_seq),
  TCheat(CheatCDFunc, cheat_cd_seq),
  TCheat(CheatMyPosFunc, cheat_mypos_seq),
  TCheat(NULL, NULL)
};

// original Doom cheats
static TCheat Doom_Cheats[] = {
  TCheat(CheatAMFunc, cheat_amap_seq),
  TCheat(CheatMusFunc, cheat_mus_seq),
  TCheat(CheatGodFunc, cheat_god_seq),
  TCheat(CheatWeaponsFunc, cheat_ammonokey_seq),
  TCheat(CheatChopFunc, cheat_choppers_seq),
  TCheat(CheatIDKFAFunc, cheat_ammo_seq),
  TCheat(CheatNoClipFunc, cheat_noclip_seq),
  TCheat(CheatNoClipFunc, cheat_commercial_noclip_seq),
  TCheat(CheatPowerup1Func, cheat_powerup_seq1),
  TCheat(CheatPowerup2Func, cheat_powerup_seq2),
  TCheat(CheatWarpFunc, cheat_clev_seq),
  TCheat(NULL, NULL)
};

// original Heretic cheats
static TCheat Heretic_Cheats[] = {
  TCheat(CheatGodFunc, CheatGodSeq),
  TCheat(CheatNoClipFunc, CheatNoClipSeq),
  TCheat(CheatWeaponsFunc, CheatWeaponsSeq),
  TCheat(CheatPowerFunc, CheatPowerSeq),
  TCheat(CheatHealthFunc, CheatHealthSeq),
  TCheat(CheatKeysFunc, CheatKeysSeq),
//      TCheat(CheatSoundFunc, CheatSoundSeq, NULL, 0, 0, 0 },
  TCheat(CheatTickerFunc, CheatTickerSeq),
  TCheat(CheatArtifact1Func, CheatArtifact1Seq),
  TCheat(CheatArtifact2Func, CheatArtifact2Seq),
  TCheat(CheatArtifact3Func, CheatArtifact3Seq),
  TCheat(CheatWarpFunc, CheatWarpSeq),
  TCheat(CheatChickenFunc, CheatChickenSeq),
  TCheat(CheatMassacreFunc, CheatMassacreSeq),
  TCheat(CheatIDKFAFunc, CheatIDKFASeq),
  TCheat(CheatIDDQDFunc, CheatIDDQDSeq),
  TCheat(NULL, NULL) // Terminator
};


//--------------------------------------------------------------------------
//
// Returns true if it eats the event
//
//--------------------------------------------------------------------------

bool cht_Responder (event_t* ev)
{
  int i;
  bool eat = false;
  TCheat *cheats = Doom_Cheats;

  if (ev->type != ev_keydown)
    return false;

  if (game.netgame || game.skill == sk_nightmare)
    { // Can't cheat in a net-game, or in nightmare mode
      return false;
    }

  PlayerPawn *p = consoleplayer->pawn;

  if (p == NULL || p->health <= 0)
    { // Dead players can't cheat
      return false;
    }

  byte key = ev->data1;

  // what about splitscreen?

  // universal cheats first
  for (i = 0; Basic_Cheats[i].func != NULL; i++)
    {
      if (Basic_Cheats[i].AddKey(key, &eat))
	{
	  Basic_Cheats[i].func(p, Basic_Cheats[i].args);
	}
    }

  // use heretic cheats instead?
  if (game.mode == gm_heretic)
    cheats = Heretic_Cheats;

  for (i = 0; cheats[i].func != NULL; i++)
    {
      if (cheats[i].AddKey(key, &eat))
	{
	  CONS_Printf("Cheating, %d\n", i);
	  cheats[i].func(p, cheats[i].args);
	  if (game.mode == gm_heretic)
	    S_StartAmbSound(sfx_dorcls);
	}
    }
  return eat;
}


//--------------------------------------------------------------------------
//
// CHEAT FUNCTIONS
//
//--------------------------------------------------------------------------

// not yet a console command, but a cheat
void CheatFlyFunc(PlayerPawn *p, const byte *arg)
{
  char *msg;

  p->cheats ^= CF_FLYAROUND;
  if (p->cheats & CF_FLYAROUND)
    msg = "FLY MODE ON : USE JUMP KEY";
  else
    msg = "FLY MODE OFF";

  p->SetMessage(msg, false);
}

void CheatCDFunc(PlayerPawn *p, const byte *arg)
{
  // 'idcd' for changing cd track quickly
  //NOTE: the cheat uses the REAL track numbers, not remapped ones

  p->SetMessage("Changing cd track...", false);
  I_PlayCD((arg[0]-'0')*10 + (arg[1]-'0'), true);
}

void CheatMusFunc(PlayerPawn *p, const byte *arg)
{
  // 'mus' cheat for changing music
  int  musnum;
  char *msg;

  msg = STSTR_MUS;

  if (game.mode == gm_doom2)
    {
      musnum = (arg[0]-'0')*10 + arg[1]-'0';

      if (musnum < 1 || musnum > 35)
	msg = STSTR_NOMUS;
      else
	S_StartMusic(musnum + mus_runnin - 1, true);
    }
  else
    {
      musnum = (arg[0]-'1')*9 + (arg[1]-'1');

      if (musnum < 0 || musnum > 31)
	msg = STSTR_NOMUS;
      else
	S_StartMusic(musnum + mus_e1m1, true);
    }
  p->SetMessage(msg, false);
}

void CheatMyPosFunc(PlayerPawn *p, const byte *arg)
{
  // 'mypos' for player position
  //extern int statusbarplayer; // FIXME! show statbarpl. coordinates, not consolepl.

  CONS_Printf(va("ang=%i;x,y=(%i,%i)\n", p->angle / ANGLE_1, p->x >> FRACBITS,
		 p->y >> FRACBITS));
}

static void CheatAMFunc(PlayerPawn *p, const byte *arg)
{
  automap.am_cheating = (automap.am_cheating+1) % 3;
}

static void CheatGodFunc(PlayerPawn *p, const byte *arg)
{
  char *msg;

  p->cheats ^= CF_GODMODE;

  if (game.mode == gm_heretic) {
    if (p->cheats & CF_GODMODE)
      {
	msg = TXT_CHEATGODON;
      }
    else
      {
	msg = TXT_CHEATGODOFF;
      }
  } else { // doom then
    if (p->cheats & CF_GODMODE)
      {
	p->health = god_health;
	msg = STSTR_DQDON;
      }
    else
      msg = STSTR_DQDOFF;
  }
  p->SetMessage(msg, false);
}

static void CheatChopFunc(PlayerPawn *p, const byte *arg)
{
  // 'choppers' invulnerability & chainsaw
  p->weaponowned[wp_chainsaw] = true;
  p->powers[pw_invulnerability] = true;

  p->SetMessage(STSTR_CHOPPERS, false);
}


static void CheatPowerup1Func(PlayerPawn *p, const byte *arg)
{
  // 'behold' power-up menu
  p->SetMessage(STSTR_BEHOLD, false);
}


static void CheatPowerup2Func(PlayerPawn *p, const byte *arg)
{
  // arg[0] = [vsiral]
  // 'behold?' power-up cheats
  int i;

  switch (arg[0]) {
  case 'v': i=0; break;
  case 's': i=1; break;
  case 'i': i=2; break;
  case 'r': i=3; break;
  case 'a': i=4; break;
  case 'l': i=5; break;
  default: return; // invalid letter
  }

  if (!p->powers[i])
    p->GivePower(i);
  else if (i != pw_strength)
    p->powers[i] = 1;
  else
    p->powers[i] = 0;

  p->SetMessage(STSTR_BEHOLDX, false);
}


static void CheatNoClipFunc(PlayerPawn *p, const byte *arg)
{
  char *msg;

  p->cheats ^= CF_NOCLIP;

  if (p->cheats & CF_NOCLIP)
    {
      if (game.mode == gm_heretic) 
	msg = TXT_CHEATNOCLIPON;
      else
	msg = STSTR_NCON;
    }
  else
    {
      if (game.mode == gm_heretic) 
	msg = TXT_CHEATNOCLIPOFF;
      else
	msg = STSTR_NCOFF;
    }

  p->SetMessage(msg, false);
}

static const bool shareware = true;

static void CheatWeaponsFunc(PlayerPawn *p, const byte *arg)
{
  char *msg;
  int i;

  p->armorpoints = idfa_armor;
  p->armortype = idfa_armor_class;

  if (game.mode == gm_heretic)
    {
      // give backpack
      if (!p->backpack)
	{
	  p->maxammo = maxammo2;
	  p->backpack = true;
	}

      for (i = wp_heretic; i <= wp_gauntlets; i++)
	p->weaponowned[i] = true;

      // FIXME shareware == true always (it is not a variable!)
      // (we do not have "heretic shareware" gametype)(yet?)
      // also elsewhere in this file
      if (shareware)
	{
	  p->weaponowned[wp_skullrod] = false;
	  p->weaponowned[wp_phoenixrod] = false;
	  p->weaponowned[wp_mace] = false;
	}

      msg = TXT_CHEATWEAPONS;
    }
  else
    {
      for (i=0; i < wp_heretic; i++)
	p->weaponowned[i] = true;

      if (game.mode != gm_doom2)
	p->weaponowned[wp_supershotgun] = false;

      msg = STSTR_FAADDED;
    }

  for (i = 0; i < NUMAMMO; i++)
    p->ammo[i] = p->maxammo[i];

  p->SetMessage(msg, false);
}

bool P_UseArtifact(PlayerPawn *p, artitype_t arti);

static void CheatPowerFunc(PlayerPawn *p, const byte *arg)
{
  if(p->powers[pw_weaponlevel2])
    {
      p->powers[pw_weaponlevel2] = 0;
      p->SetMessage(TXT_CHEATPOWEROFF, false);
    }
  else
    {
      P_UseArtifact(p, arti_tomeofpower);
      p->SetMessage(TXT_CHEATPOWERON, false);
    }
}

static void CheatHealthFunc(PlayerPawn *p, const byte *arg)
{
#define MAXCHICKENHEALTH 30
  /*
  if(p->morphTics)
    {
      p->health = MAXCHICKENHEALTH;
    }
  else
  */
    p->health = p->maxhealth;

  p->SetMessage(TXT_CHEATHEALTH, false);
}

static void CheatKeysFunc(PlayerPawn *p, const byte *arg)
{
  p->cards |= it_allkeys;
  p->SetMessage(TXT_CHEATKEYS, false);
}

static void CheatTickerFunc(PlayerPawn *p, const byte *arg)
{
  // FIXME what is this?
  /*
  cv_ticrate.value = !cv_ticrate.value;
  if(cv_ticrate.value)
    {
      p->SetMessage(TXT_CHEATTICKERON, false);
    }
  else
    {
      p->SetMessage(TXT_CHEATTICKEROFF, false);
    }
  */
}

static void CheatArtifact1Func(PlayerPawn *p, const byte *arg)
{
  p->SetMessage(TXT_CHEATARTIFACTS1, false);
}

static void CheatArtifact2Func(PlayerPawn *p, const byte *arg)
{
  p->SetMessage(TXT_CHEATARTIFACTS2, false);
}

static void CheatArtifact3Func(PlayerPawn *p, const byte *arg)
{
  int i;
  int j;
  artitype_t type;
  int count;

  type = artitype_t(arg[0]-'a'+1);
  count = arg[1]-'0';
  if (type == 26 && count == 0)
    { // All artifacts
      for(i = arti_none+1; i < NUMARTIFACTS; i++)
	{
	  if(shareware && (i == arti_superhealth
			   || i == arti_teleport))
	    {
	      continue;
	    }
	  for(j = 0; j < 16; j++)
	    {
	      p->GiveArtifact(artitype_t(i), NULL);
	    }
	}
      p->SetMessage(TXT_CHEATARTIFACTS3, false);
    }
  else if(type > arti_none && type < NUMARTIFACTS
	  && count > 0 && count < 10)
    {
      if(shareware && (type == arti_superhealth || type == arti_teleport))
	{
	  p->SetMessage(TXT_CHEATARTIFACTSFAIL, false);
	  return;
	}
      for(i = 0; i < count; i++)
	{
	  p->GiveArtifact(type, NULL);
	}
      p->SetMessage(TXT_CHEATARTIFACTS3, false);
    }
  else
    { // Bad input
      p->SetMessage(TXT_CHEATARTIFACTSFAIL, false);
    }
}

static void CheatWarpFunc(PlayerPawn *p, const byte *arg)
{
  int episode;
  int mapnum;
  char *msg;

  // "idclev" or "engage" change-level cheat
  char name[9];

  if (game.mode == gm_doom2)
    {
      episode = 0;
      mapnum = (arg[0] - '0')*10 + arg[1] - '0';
      if (mapnum < 1 || mapnum > 99)
	return;
      sprintf(name, "MAP%2d", mapnum);
    }
  else // doom1, heretic
    {
      episode = arg[0] - '0';
      mapnum = arg[1] - '0';
      if (episode < 1 || episode > 9 || mapnum < 1 || mapnum > 9)
	return;
      sprintf(name, "E%1dM%1d", episode, mapnum);
    }

  if (game.mode == gm_heretic)
    msg = TXT_CHEATWARP;
  else
    msg = STSTR_CLEV;

  p->SetMessage(msg, false);
  COM_BufAddText(va("map %s\n", name));
}

static void CheatChickenFunc(PlayerPawn *p, const byte *arg)
{
  if (p->morphTics)
    {
      if (p->UndoMorph())
	{
	  p->SetMessage(TXT_CHEATCHICKENOFF, false);
	}
    }
  else if (p->Morph())
    {
      p->SetMessage(TXT_CHEATCHICKENON, false);
    }
}

static void CheatMassacreFunc(PlayerPawn *p, const byte *arg)
{
  p->mp->Massacre();
  p->SetMessage(TXT_CHEATMASSACRE, false);
}

static void CheatIDKFAFunc(PlayerPawn *p, const byte *arg)
{
  int i;

  if (game.mode == gm_heretic)
    {
      // playing heretic, let's punish the player!
      if (p->morphTics)
	return;

      for(i = 0; i < NUMWEAPONS; i++)
	p->weaponowned[i] = false;
      p->weaponowned[wp_staff] = true;
      p->pendingweapon = wp_staff;

      p->SetMessage(TXT_CHEATIDKFA, true);
    }
  else
    {
      // doom, give stuff
      p->armorpoints = idkfa_armor;
      p->armortype = idkfa_armor_class;

      for (i=0;i<wp_heretic;i++)
	p->weaponowned[i] = true;

      if (game.mode != gm_doom2)
	p->weaponowned[wp_supershotgun] = false;

      for (i=0;i<am_heretic;i++)
	p->ammo[i] = p->maxammo[i];

      p->cards = it_allkeys;

      p->SetMessage(STSTR_KFAADDED, false);
    }
}

static void CheatIDDQDFunc(PlayerPawn *p, const byte *arg)
{
  p->Damage(p, p, 10000, dt_always);
  p->SetMessage(TXT_CHEATIDDQD, true);
}
