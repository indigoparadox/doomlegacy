// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1996 by Raven Software, Corp.
// Copyright (C) 2003-2005 by DooM Legacy Team.
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
// Revision 1.4  2005/09/11 16:23:25  smite-meister
// template classes
//
// Revision 1.3  2004/07/25 20:18:47  hurdler
// Remove old hardware renderer and add part of the new one
//
// Revision 1.2  2003/06/01 18:56:30  smite-meister
// zlib compression, partial polyobj fix
//
// Revision 1.1  2003/05/05 00:24:50  smite-meister
// Hexen linedef system. Pickups.
//
//
//
// DESCRIPTION:
//   Polyobjects
//
//-----------------------------------------------------------------------------

#ifndef r_poly_h
#define r_poly_h 1

#include "r_defs.h"

struct polyobj_t
{
  int numsegs;
  struct seg_t **segs;
  mappoint_t startSpot;
  struct vertex_t *originalPts; // used as the base for the rotations
  vertex_t *prevPts;            // use to restore the old point values
  angle_t angle;
  int tag;                      // reference tag assigned in HereticEd
  int bbox[4];      ///< bounding box in blockmap coordinates
  int validcount;
  bool crush;                   // should the polyobj attempt to crush mobjs?
  unsigned seqType;
  //fixed_t size; // polyobj size (area of POLY_AREAUNIT == size of FRACUNIT)
  class polyobject_t *specialdata; // pointer to a thinker, if the poly is moving
};

// one polyobj can be linked to many blockmap cells
struct polyblock_t
{
  polyobj_t *polyobj;
  polyblock_t *prev;
  polyblock_t *next;
};

#endif
