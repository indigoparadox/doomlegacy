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
// Revision 1.13  2004/11/18 20:30:14  smite-meister
// tnt, plutonia
//
// Revision 1.12  2004/07/05 16:53:29  smite-meister
// Netcode replaced
//
// Revision 1.11  2004/03/28 15:16:14  smite-meister
// Texture cache.
//
// Revision 1.10  2003/05/30 13:34:48  smite-meister
// Cleanup, HUD improved, serialization
//
// Revision 1.9  2003/05/05 00:24:49  smite-meister
// Hexen linedef system. Pickups.
//
// Revision 1.8  2003/04/19 17:38:47  smite-meister
// SNDSEQ support, tools, linedef system...
//
// Revision 1.7  2003/04/04 00:01:57  smite-meister
// bugfixes, Hexen HUD
//
// Revision 1.6  2003/03/15 20:07:20  smite-meister
// Initial Hexen compatibility!
//
// Revision 1.5  2003/03/08 16:07:13  smite-meister
// Lots of stuff. Sprite cache. Movement+friction fix.
//
// Revision 1.4  2003/02/16 16:54:51  smite-meister
// L2 sound cache done
//
// Revision 1.3  2003/01/18 20:17:41  smite-meister
// HUD fixed, levelchange crash fixed.
//
// Revision 1.2  2002/12/23 23:19:37  smite-meister
// Weapon groups, MAPINFO parser, WAD2+WAD3 support added!
//
// Revision 1.1.1.1  2002/11/16 14:18:22  hurdler
// Initial C++ version of Doom Legacy
//
//-----------------------------------------------------------------------------

/// \file
/// \brief Game items: keys, armor, artifacts, weapons, ammo.

#ifndef d_items_h
#define d_items_h 1

#include "doomdef.h"
#include "doomtype.h"
#include "info.h"


/// Key types
enum keycard_t
{
  // Hexen
  it_key_1 = 0x0001,
  it_key_2 = 0x0002,
  it_key_3 = 0x0004,
  it_key_4 = 0x0008,
  it_key_5 = 0x0010,
  it_key_6 = 0x0020,
  it_key_7 = 0x0040,
  it_key_8 = 0x0080,
  it_key_9 = 0x0100,
  it_key_A = 0x0200,
  it_key_B = 0x0400,

  // Doom and Heretic keys are merged
  it_bluecard   = 0x0800,
  it_yellowcard = 0x1000,
  it_redcard    = 0x2000,
  it_blueskull  = 0x4000,
  it_yellowskull= 0x8000,
  it_redskull   = 0x10000,

  NUMKEYS = 17,
  it_allkeys = 0x1ffff,
};


/// Armor locations
enum armortype_t
{
  armor_field, // old doom style armor
  armor_armor,
  armor_shield,
  armor_helmet,
  armor_amulet,
  NUMARMOR
};


/// Inventory artifact types
enum artitype_t
{
  arti_none,
  arti_invulnerability,
  arti_invisibility,
  arti_health,
  arti_superhealth,
  arti_tomeofpower,
  arti_torch,
  arti_firebomb,
  arti_egg,
  arti_fly,
  arti_teleport,

  // hexen
  arti_healingradius,
  arti_summon,
  arti_pork,
  arti_blastradius,
  arti_poisonbag,
  arti_teleportother,
  arti_speed,
  arti_boostmana,
  arti_boostarmor,

  // Puzzle artifacts
  arti_firstpuzzitem,
  arti_puzzskull = arti_firstpuzzitem,
  arti_puzzgembig,
  arti_puzzgemred,
  arti_puzzgemgreen1,
  arti_puzzgemgreen2,
  arti_puzzgemblue1,
  arti_puzzgemblue2,
  arti_puzzbook1,
  arti_puzzbook2,
  arti_puzzskull2,
  arti_puzzfweapon,
  arti_puzzcweapon,
  arti_puzzmweapon,
  arti_puzzgear1,
  arti_puzzgear2,
  arti_puzzgear3,
  arti_puzzgear4,

  // weapon pieces
  arti_fsword1,
  arti_fsword2,
  arti_fsword3,
  arti_choly1,
  arti_choly2,
  arti_choly3,
  arti_mstaff1,
  arti_mstaff2,
  arti_mstaff3,

  NUMARTIFACTS
};


/// PlayerPawn inventory entry
struct inventory_t
{
  byte type;
  byte count;

  inventory_t();
  inventory_t(byte t, byte c);
};

#define NUMINVENTORYSLOTS  14
#define MAXARTECONT        16 


/// Powerups
enum powertype_t
{
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,

  // heretic
  pw_weaponlevel2,
  pw_flight,
  
  // hexen
  pw_shield,
  pw_health2,
  pw_speed,
  pw_minotaur,

  NUMPOWERS
};


/// Powerup durations,
enum powerduration_t
{
  INVULNTICS  = (30*TICRATE),
  INVISTICS   = (60*TICRATE),
  INFRATICS   = (120*TICRATE),
  IRONTICS    = (60*TICRATE),
  WPNLEV2TICS = (40*TICRATE),
  FLIGHTTICS  = (60*TICRATE),
  MAULATORTICS = (25*TICRATE),
  SPEEDTICS   = (45*TICRATE)
};



/// Weapons
enum weapontype_t
{
  wp_none = -1,

  wp_fist = 0, wp_doom = wp_fist,
  wp_chainsaw,
  wp_pistol,
  wp_shotgun,
  wp_supershotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,

  wp_staff, wp_heretic = wp_staff,
  wp_gauntlets,
  wp_goldwand,
  wp_crossbow,
  wp_blaster,
  wp_phoenixrod,
  wp_skullrod,
  wp_mace,
  wp_beak,

  wp_fpunch, wp_hexen = wp_fpunch,
  wp_cmace,
  wp_mwand,
  wp_timons_axe,
  wp_serpent_staff,
  wp_cone_of_shards,
  wp_hammer_of_retribution,
  wp_firestorm,
  wp_arc_of_death,
  wp_quietus,
  wp_wraithverge,
  wp_bloodscourge,
  wp_snout,

  NUMWEAPONS,
  wp_barrel    // barrel explosion
};


/// Ammunition types
enum ammotype_t
{
  am_noammo = -1, // unlimited

  am_clip = 0, am_doom = am_clip, // Pistol / chaingun ammo.
  am_shell,   // Shotgun / double barreled shotgun.
  am_cell,    // Plasma rifle, BFG.
  am_misl,    // Missile launcher.

  am_goldwand, am_heretic = am_goldwand,
  am_crossbow,
  am_blaster,
  am_skullrod,
  am_phoenixrod,
  am_mace,

  // Hexen
  am_mana1, am_hexen = am_mana1,
  am_mana2,

  NUMAMMO,
  am_manaboth  // both mana types
};

/// Ammo limits for players
extern int maxammo1[NUMAMMO]; ///< without backpack
extern int maxammo2[NUMAMMO]; ///< with backpack


/// "volatile" weapon info (changes with tome of power etc...)
struct weaponinfo_t
{
  ammotype_t ammo;
  int        ammopershoot;
  weaponstatenum_t upstate;
  weaponstatenum_t downstate;
  weaponstatenum_t readystate;
  weaponstatenum_t atkstate;
  weaponstatenum_t holdatkstate;
  weaponstatenum_t flashstate;
};

extern weaponinfo_t wpnlev1info[NUMWEAPONS];
extern weaponinfo_t wpnlev2info[NUMWEAPONS];


/// Weapon grouping
struct weapon_group_t
{
  int group;   // in which group it resides
  weapontype_t next; // next weapon in group
};

extern weapon_group_t weapongroup[NUMWEAPONS];

#endif
