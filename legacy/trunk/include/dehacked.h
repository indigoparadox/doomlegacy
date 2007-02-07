// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2007 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------

/// \file
/// \brief DeHackEd and BEX support

#ifndef dehacked_h
#define dehacked_h 1

#include "parser.h"
#include "info.h"

class dehacked_t
{
private:
  Parser p;
  int  num_errors;

  int  FindValue();
  int  FindState();
  bool ReadFlags(struct mobjinfo_t *m);

  void Read_Thing(const char *str);
  void Read_Frame(const char *str);
  void Read_Sound(int num);
  void Read_Text(int len1, int len2);
  void Read_Weapon(int num);
  void Read_Ammo(int num);
  void Read_Misc();
  void Read_Cheat();
  void Read_CODEPTR();
  void Read_STRINGS();

public:
  bool loaded;

  dehacked_t();
  bool LoadDehackedLump(int lump);
  void error(char *first, ...);

  int   idfa_armor;
  float idfa_armorfactor;
  int   idkfa_armor;
  float idkfa_armorfactor;
  int   god_health;

  int max_health;
  int max_soul_health;
};

extern dehacked_t DEH;


/// [CODEPTR] DActor action function mnemonics
struct dactor_mnemonic_t
{
  char       *name;
  actionf_p1  ptr;
};

extern dactor_mnemonic_t BEX_DActorMnemonics[];


/// [CODEPTR] Weapon action function mnemonics
struct weapon_mnemonic_t
{
  char       *name;
  actionf_p2  ptr;
};

extern weapon_mnemonic_t BEX_WeaponMnemonics[];


/// \brief BEX/DECORATE flag mnemonics.
struct flag_mnemonic_t
{
  char *name;
  int   flag;
  int   flagword;
};

extern flag_mnemonic_t BEX_FlagMnemonics[];

#endif
