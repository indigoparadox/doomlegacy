// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2003 by DooM Legacy Team.
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
// Revision 1.11  2003/05/11 21:23:52  smite-meister
// Hexen fixes
//
// Revision 1.10  2003/05/05 00:24:49  smite-meister
// Hexen linedef system. Pickups.
//
// Revision 1.9  2003/04/14 08:58:30  smite-meister
// Hexen maps load.
//
// Revision 1.8  2003/03/23 14:24:13  smite-meister
// Polyobjects, MD3 models
//
// Revision 1.7  2003/03/15 20:07:20  smite-meister
// Initial Hexen compatibility!
//
// Revision 1.6  2003/03/08 16:07:14  smite-meister
// Lots of stuff. Sprite cache. Movement+friction fix.
//
// Revision 1.5  2003/02/16 16:54:52  smite-meister
// L2 sound cache done
//
// Revision 1.4  2002/12/29 18:57:03  smite-meister
// MAPINFO implemented, Actor deaths handled better
//
// Revision 1.3  2002/12/23 23:19:37  smite-meister
// Weapon groups, MAPINFO parser, WAD2+WAD3 support added!
//
// Revision 1.2  2002/12/16 22:04:48  smite-meister
// Actor / DActor separation done!
//
// Revision 1.1.1.1  2002/11/16 14:18:22  hurdler
// Initial C++ version of Doom Legacy
//
//
// DESCRIPTION:
//   Actor class definition
//
//-----------------------------------------------------------------------------


#ifndef g_actor_h
#define g_actor_h 1

#include "m_fixed.h"  // Basics.
#include "g_think.h"  // We need the Thinker stuff.
#include "g_damage.h" // and damage types

class PlayerPawn;

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

#define ONFLOORZ        MININT
#define ONCEILINGZ      MAXINT

#define TELEFOGHEIGHT  (32*FRACUNIT)
#define FOOTCLIPSIZE   (10*FRACUNIT)

//
// NOTES: Actor
//
// Actors are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the Actor.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when Actors are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The Actor->flags element has various bit flags
// used by the simulation.
//
// Every Actor is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any Actor that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable Actor that has its origin contained.
//
// A valid Actor is a Actor that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//


// Actor flags. More or less permanent attributes of the Actor.

typedef enum
{
  // physical properties
  MF_NOSECTOR         = 0x0001, // Don't link to sector (invisible but touchable)
  MF_NOBLOCKMAP       = 0x0002, // Don't link to blockmap (inert but displayable)
  MF_SOLID            = 0x0004, // Blocks
  MF_SHOOTABLE        = 0x0008, // Can be hit
  MF_NOCLIPLINE       = 0x0010, // Does not clip against lines (walls)
  MF_NOCLIPTHING      = 0x0020, // Not blocked by other Actors. (chasecam, for example)
  // game mechanics
  MF_NOGRAVITY        = 0x0040, // Does not feel gravity
  MF_FLOAT            = 0x0080, // Active floater, can move freely in air (cacodemons etc.)
  MF_NOTRIGGER        = 0x0100, // Can not trigger linedefs (mainly missiles, chasecam)
  MF_DROPOFF          = 0x0200, // This allows jumps from high places.
  // appearance
  MF_SHADOW           = 0x0400, // Partial invisibility (spectre). Makes targeting harder.
  MF_ALTSHADOW        = 0x0800, // Alternate fuzziness
  MF_NOBLOOD          = 0x1000, // Don't bleed when shot (use puff) (furniture)
  MF_NOSPLASH         = 0x2000, // Does not cause a splash when hitting water
  // spawning
  MF_SPAWNCEILING     = 0x4000, // Spawned hanging from the ceiling
  MF_NOTDMATCH        = 0x8000, // Not spawned in DM (keycards etc.)
  MF_COUNTKILL       = 0x10000, // On kill, count this enemy object towards intermission kill total. Happy gathering.
  MF_COUNTITEM       = 0x20000, // On picking up, count this item object towards intermission item total.
  MF_NORESPAWN       = 0x40000, // Will not respawn after being picked up. Pretty similar to MF_DROPPED?
  // classification
  MF_MISSILE         = 0x80000, // Player missiles as well as fireballs. Don't hit same species, explode on block.
  MF_PICKUP         = 0x100000, // Can/will pick up items. (players)
  MF_SPECIAL        = 0x200000, // Call TouchSpecialThing when touched
  MF_NOTMONSTER     = 0x400000, // *Not affected by ML_BLOCKMONSTERS lines (PlayerPawns etc.)
  // misc (usually set just once)
  MF_CORPSE         = 0x800000, // *Acts like a corpse, falls down stairs etc.
  MF_DROPPED       = 0x1000000, // *Dropped by a monster
  MF_AMBUSH        = 0x2000000, // *Not to be activated by sound, deaf monster.

  //MF_SLIDE           // Player: keep info about sliding along walls. (unused)
  //MF_FLOORHUGGER     // Can not leave the floor (unused)
} mobjflag_t;

// More semi-permanent flags. Mostly came with Heretic.

typedef enum {
  MF2_LOGRAV         =     0x00000001,      // alternate gravity setting
  MF2_WINDTHRUST     =     0x00000002,      // gets pushed around by the wind specials
  MF2_FLOORBOUNCE    =     0x00000004,      // bounces off the floor
  MF2_THRUGHOST      =     0x00000008,      // missile will pass through ghosts
  MF2_FLY            =     0x00000010,      // fly mode is active
  MF2_FOOTCLIP       =     0x00000020,      // if feet are allowed to be clipped
  MF2_SPAWNFLOAT     =     0x00000040,      // spawn random float z
  MF2_NOTELEPORT     =     0x00000080,      // does not teleport
  MF2_RIP            =     0x00000100,      // missile rips through solid targets
  MF2_PUSHABLE       =     0x00000200,      // can be pushed by other moving mobjs
  MF2_SLIDE          =     0x00000400,      // slides against walls
  MF2_ONMOBJ         =     0x00000800,      // mobj is resting on top of another mobj
  MF2_PASSMOBJ       =     0x00001000,      // Actor can move over/under other Actors 
  MF2_CANNOTPUSH     =     0x00002000,      // cannot push other pushable mobjs

  MF2_BOSS           =     0x00008000,      // mobj is a major boss
  MF2_FIREDAMAGE     =     0x00010000,      // does fire damage
  MF2_NODMGTHRUST    =     0x00020000,      // does not thrust target when damaging        
  MF2_TELESTOMP      =     0x00040000,      // mobj can stomp another
  MF2_FLOATBOB       =     0x00080000,      // use float bobbing z movement
  MF2_DONTDRAW       =     0x00100000,      // don't generate a vissprite
  MF2_IMPACT	     =     0x00200000,      // an MF_MISSILE mobj can activate SPAC_IMPACT
  MF2_PUSHWALL	     =     0x00400000,      // mobj can push walls
  MF2_MCROSS		 = 0x00800000,	    // can activate monster cross lines
  MF2_PCROSS		 = 0x01000000,	    // can activate projectile cross lines
  MF2_CANTLEAVEFLOORPIC  = 0x02000000,	    // stay within a certain floor type
  MF2_NONSHOOTABLE	 = 0x04000000,      // mobj is totally non-shootable, but still considered solid
  MF2_INVULNERABLE	 = 0x08000000,	    // mobj is invulnerable
  MF2_DORMANT		 = 0x10000000,	    // thing is dormant
  MF2_ICEDAMAGE		 = 0x20000000,	    // does ice damage
  MF2_SEEKERMISSILE	 = 0x40000000,	    // is a seeker (for reflection)
  MF2_REFLECTIVE	 = 0x80000000
} mobjflag2_t;

// Extra flags. They describe the transient state of the Actor.

typedef enum
{
  // location
  MFE_ONGROUND      = 0x0001,  // The mobj stands on solid floor (not on another mobj or in air)
  MFE_JUSTHITFLOOR  = 0x0002,  // just hit the floor while falling, cleared on next frame
  // (instant damage in lava/slime sectors to prevent jump cheat..)
  MFE_TOUCHWATER    = 0x0004,  // touches water.
  MFE_UNDERWATER    = 0x0008,  // waist below water surface (swimming is possible)
  // active physics mode
  MFE_SWIMMING      = 0x0010,  // swimming physics used (different gravity)
  MFE_INFLOAT       = 0x0020,  // Floating move in progress, don't auto float to target's height.
  MFE_SKULLFLY      = 0x0040,  // A charging skull.
  MFE_BLASTED       = 0x0080,  // uncontrollably thrown by a blast wave

  // combat
  MFE_JUSTHIT       = 0x0100,  // Got hit, will try to attack right back.
  MFE_JUSTATTACKED  = 0x0200,  // Will take at least one step before attacking again.

  MFE_TELEPORT      = 0x1000, // *Don't cross lines or check heights in teleport move. (unused?)
} mobjeflag_t;


struct subsector_t;
struct mapthing_t;
struct msecnode_t;
class  LArchive;

// Actor class. Basis class for all things.
class Actor : public Thinker
{
public:
  // graphic presentation
  class presentation_t *pres;

  // sector links
  Actor *snext;
  Actor *sprev;

  // Blockmap links
  Actor *bnext;
  Actor *bprev;

  subsector_t *subsector; // location

  // The closest interval over all contacted Sectors (or Things).
  fixed_t floorz;
  fixed_t ceilingz;

  // If == validcount, already checked.
  //int     validcount;
  // a linked list of sectors where this object appears
  msecnode_t* touching_sectorlist;

  // For nightmare and itemrespawn respawn.
  mapthing_t *spawnpoint;

public:
  // position.
  fixed_t x, y, z;

  // was: angle_t angle, aiming, (nothing)
  // will be angle_t roll, pitch, yaw;  //rotation around x, y and z axes 
  angle_t  angle;  // orientation left-right
  angle_t  aiming; // up-down, updated with cmd->aiming.

  // Momentums, used to update position. Actually velocities.
  fixed_t px, py, pz;

  // For movement checking.
  fixed_t mass;
  fixed_t radius;
  fixed_t height;

  int  health;

  int  flags;
  int  flags2;
  int  eflags;

  // general storage, can hold pointers, ints etc. 
  int  special1;
  int  special2;

  // Hexen fields
  short	tid;     // thing identifier
  byte	special; // special type
  byte	args[5]; // special arguments

  // new: owner of this Actor. For example, for missiles this is the shooter.
  Actor *owner;

  // Thing being chased/attacked (or NULL),
  // also the target for missiles.
  Actor *target;

  // Reaction time: if non 0, don't attack yet.
  // Used by player to freeze a bit after teleporting.
  int  reactiontime;

  fixed_t floorclip; // cut this amount from legs (deep water illusion) (Hexen)

public:
  // sounds TODO these could be replaced with SNDINFO
  static int s_pickup, s_keypickup, s_weaponpickup, s_artipickup, s_teleport, s_respawn, s_gibbed;

  virtual thinkertype_e Type() {return tt_actor;}; // "name-tag" function

  // in g_actor.cpp
  // construct a new Actor
  Actor();
  Actor(fixed_t nx, fixed_t ny, fixed_t nz);
  void Remove();   // delayed destruction

  virtual int Serialize(LArchive & a);

  virtual void Think();

  void CheckWater(); // set some eflags if sector contains water

  float GetMoveFactor();
  virtual void XYMovement();
  virtual void ZMovement();

  virtual void XYFriction(fixed_t oldx, fixed_t oldy);
  virtual void Thrust(angle_t angle, fixed_t move);

  int  HitFloor();

  // in p_inter.cpp
  virtual bool Touch(Actor *a); // Actor touches another Actor
  virtual void Die(Actor *inflictor, Actor *source);
  virtual void Killed(PlayerPawn *victim, Actor *inflictor);
  virtual bool Morph();
  virtual bool Damage(Actor *inflictor, Actor *source, int damage, int dtype = dt_normal);

  // in p_telept.cpp
  virtual bool Teleport(fixed_t nx, fixed_t ny, angle_t nangle, bool silent = false);

  // in p_map.cpp
  bool TeleportMove(fixed_t nx, fixed_t ny);
  bool TryMove(fixed_t nx, fixed_t ny, bool allowdropoff);
  void CheckMissileImpact();
  bool TestLocation();
  bool CheckPosition(fixed_t nx, fixed_t ny);
  void FakeZMovement();
  Actor *CheckOnmobj();

  fixed_t AimLineAttack(angle_t ang, fixed_t distance);
  void    LineAttack(angle_t ang, fixed_t distance, fixed_t slope,
		     int damage, int dtype = dt_normal);
  void  RadiusAttack(Actor *culprit, int damage, int distance = -1,
		     int dtype = dt_normal, bool downer = true);
  // in p_maputl.cpp
  void SetPosition();
  void UnsetPosition();
};


//========================================================
// Doom Actor. An Actor with the standard Doom/Heretic AI
// (uses the A_* routines and the states table in info.cpp)
class DActor : public Actor
{
public:
  mobjtype_t  type;       // what kind of thing is it?
  const mobjinfo_t *info; // basic properties    

  // state machine variables
  const state_t *state;
  int            tics;   // state tic counter

  // Movement direction, movement generation (zig-zagging).
  int  movedir;        // 0-7
  int  movecount;      // when 0, select a new dir

  int  threshold;  // If >0, the target will be chased no matter what (even if shot)
  int  lastlook;   // Player number last looked for.

public:
  virtual thinkertype_e Type() {return tt_dactor;}; // "name-tag" function

  // create a nonfunctional special actor (LavaInflictor etc...)
  DActor(mobjtype_t t);
  // create a new DActor of type t
  DActor(fixed_t nx, fixed_t ny, fixed_t nz, mobjtype_t t);

  virtual int Serialize(LArchive & a);

  virtual void Think();

  // in p_inter.cpp
  virtual bool Touch(Actor *a);
  virtual void Die(Actor *inflictor, Actor *source);
  virtual void Killed(PlayerPawn *victim, Actor *inflictor);
  virtual bool Morph();
  virtual bool Damage(Actor *inflictor, Actor *source, int damage, int dtype = dt_normal);

  bool SetState(statenum_t ns, bool call = true);

  void NightmareRespawn();

  DActor *SpawnMissile(Actor *dest, mobjtype_t type);
  bool   CheckMissileSpawn();
  void   ExplodeMissile();
  void   FloorBounceMissile();

  DActor *SpawnMissileAngle(mobjtype_t type, angle_t angle, fixed_t vz = 0);

  // in p_enemy.cpp
  bool CheckMeleeRange();
  bool CheckMissileRange();
  bool LookForPlayers(bool allaround);
  bool LookForMonsters();

  // in p_hpspr.cpp
  void BlasterMissileThink();
  void XBlasterMissileThink();

  // in p_henemy.cpp
  void DSparilTeleport();
  bool UpdateMorph(int tics);

  // in p_heretic.cpp
  bool SeekerMissile(angle_t thresh, angle_t turnMax);

  // in p_things.cpp
  bool Activate();
  bool Deactivate();
};

#endif
