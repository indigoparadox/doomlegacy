// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2004 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
//
// $Log$
// Revision 1.1  2004/10/17 01:57:05  smite-meister
// bots!
//
// Revision 1.3  2002/09/28 06:53:11  tonyd
// fixed CR problem, fixed game options crash
//
// Revision 1.2  2002/09/27 16:40:08  tonyd
// First commit of acbot

//-----------------------------------------------------------------------------

/// \file
/// \brief Pathing system for client-side bots

#ifndef b_node_h
#define b_node_h 1

#include <vector>
#include <list>
#include "m_fixed.h"

//#define SHOWBOTPATH //show the path the bot is taking in game?


enum botdirtype_t
{
 BDI_EAST,
 BDI_NORTHEAST,
 BDI_NORTH,
 BDI_NORTHWEST,
 BDI_WEST,
 BDI_SOUTHWEST,
 BDI_SOUTH,
 BDI_SOUTHEAST,
 BDI_TELEPORT,
 NUMBOTDIRS
};


/// \brief Pathnode for bots
class SearchNode_t
{
  friend class BotNodes;
protected:
  // used only during pathfinding
  bool visited;
  fixed_t cost;      ///< cumulative cost based on path length and difficulty
  fixed_t heuristic; ///< "distance to destination" 
  fixed_t f;         ///< cost + heuristic
  SearchNode_t *pprevious; ///< previous node in the path

  SearchNode_t *dir[NUMBOTDIRS]; ///< neighbor nodes
  fixed_t   costDir[NUMBOTDIRS]; ///< the cost of going from this node to a neighboring one

#ifdef SHOWBOTPATH
  class Actor *marker; ///< visible path marker for debugging
#endif
public:
  /// grid coordinates of the node
  int gx, gy;

  /// map coordinates of the node
  fixed_t mx, my; 

public:
  SearchNode_t(int gx, int gy, fixed_t mx, fixed_t my);
  ~SearchNode_t();

  void PushSuccessors(class priorityQ_t *q, int destgx, int destgy);

  void *operator new(size_t size);
  void  operator delete(void *mem);
};



/// \brief BotNode structure for Maps
class BotNodes
{
#define BOTNODEGRIDSHIFT (FRACBITS + 5)
#define BOTNODEGRIDSIZE  (1 << BOTNODEGRIDSHIFT)  // 32*FRACUNIT

  Map *mp;

  // node grid
  fixed_t xOrigin, yOrigin;
  int xSize, ySize;
  int numbotnodes;

  SearchNode_t ***botNodeArray;

public:
  BotNodes(Map *m);

  void BuildNodes(SearchNode_t *node);

  bool DirectlyReachable(Actor *a, fixed_t x, fixed_t y, fixed_t destx, fixed_t desty);

  SearchNode_t *FindClosestNode(fixed_t x, fixed_t y);
  SearchNode_t *GetClosestReachableNode(fixed_t x, fixed_t y);
  SearchNode_t *GetNodeAt(fixed_t x, fixed_t y);

  bool FindPath(list<SearchNode_t *> &path, SearchNode_t *start, SearchNode_t *dest);

  // nodes lie at the middle of their grid cell
  // map coordinates into grid coordinates
  inline int x2PosX(fixed_t x) { return (x - xOrigin) >> BOTNODEGRIDSHIFT; };
  inline int y2PosY(fixed_t y) { return (y - yOrigin) >> BOTNODEGRIDSHIFT; };

  // grid coordinates into map coordinates
  inline fixed_t posX2x(int nx) { return (nx << BOTNODEGRIDSHIFT) + xOrigin + BOTNODEGRIDSIZE/2; };
  inline fixed_t posY2y(int ny) { return (ny << BOTNODEGRIDSHIFT) + yOrigin + BOTNODEGRIDSIZE/2; };
};



/// \brief Heap-based priority queue for SearchNodes. 
class priorityQ_t
{
private:
  std::vector<SearchNode_t*> pq; // a priority queue

public:
  void Push(SearchNode_t *node);
  SearchNode_t *Pop();
  SearchNode_t *FindNode(SearchNode_t *node);
  SearchNode_t *RemoveNode(SearchNode_t *node);

  inline bool Empty() { return pq.empty(); };
};


#endif

