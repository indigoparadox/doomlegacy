// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
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
// Revision 1.12  2004/11/28 18:02:23  smite-meister
// RPCs finally work!
//
// Revision 1.10  2004/11/09 20:38:53  smite-meister
// added packing to I/O structs
//
// Revision 1.9  2004/10/27 17:37:10  smite-meister
// netcode update
//
// Revision 1.8  2004/09/03 16:28:51  smite-meister
// bugfixes and ZDoom linedef types
//
// Revision 1.7  2004/08/18 14:35:22  smite-meister
// PNG support!
//
// Revision 1.6  2004/08/06 18:54:39  smite-meister
// netcode update
//
// Revision 1.5  2004/07/14 16:13:13  smite-meister
// cleanup, commands
//
// Revision 1.4  2004/07/13 20:23:39  smite-meister
// Mod system basics
//
// Revision 1.3  2004/07/09 19:43:40  smite-meister
// Netcode fixes
//
// Revision 1.1  2004/07/05 16:53:30  smite-meister
// Netcode replaced
//
//-----------------------------------------------------------------------------

/// \file
/// \brief Server/client console commands

#include "command.h"
#include "cvars.h"
#include "console.h"
#include "parser.h"

#include "n_interface.h"
#include "n_connection.h"

#include "g_game.h"
#include "g_level.h"
#include "g_map.h"
#include "g_mapinfo.h"
#include "g_player.h"
#include "g_actor.h"
#include "g_pawn.h"

#include "w_wad.h"
#include "hud.h"
#include "i_system.h"



//========================================================================
//    Informational commands
//========================================================================

//  Returns program version.
void Command_Version_f()
{
  CONS_Printf("Doom Legacy version %d.%d %s ("__TIME__" "__DATE__")\n",
	      VERSION/100, VERSION%100, VERSIONSTRING);
}


// prints info about current game and server
void Command_GameInfo_f()  // TODO
{
  CONS_Printf("Server: %d\n", game.server);
  CONS_Printf("Netgame: %d\n", game.netgame);
  CONS_Printf("Multiplayer: %d\n", game.multiplayer);
  CONS_Printf("Network state: %d\n", game.net->netstate);
}


// prints info about a map
void Command_MapInfo_f()
{
  MapInfo *m;
  if (COM_Argc() > 1)
    {
      m = game.FindMapInfo(strupr(COM_Argv(1)));
      if (!m)
	{
	  CONS_Printf("No such map.\n");
	  return;
	}
    }
  else
    {
      if (!com_player || !com_player->mp)
	return;
      m = com_player->mp->info;
    }

  CONS_Printf("Map %d: %s (%s)\n", m->mapnumber, m->nicename.c_str(), m->lumpname.c_str());
  if (!m->version.empty())
    CONS_Printf("Version: %s\n", m->version.c_str());
  CONS_Printf("Par: %d s\n", m->partime);
  if (!m->author.empty())
    CONS_Printf("Author: %s\n", m->author.c_str());
  if (!m->hint.empty())
    CONS_Printf("%s\n", m->hint.c_str());
}


// prints player roster
void Command_Players_f()
{
  CONS_Printf("Num             Name Score Ping\n");
  for (GameInfo::player_iter_t t = game.Players.begin(); t != game.Players.end(); t++)
    {
      PlayerInfo *p = (*t).second;
      // TODO highlight serverplayers
      CONS_Printf("%3d %16s  %4d %4d\n", p->number, p->name.c_str(), 1, 20);
    }
}


// prints scoreboard
void Command_Frags_f() // TODO
{
  if (COM_Argc() > 2)
    {
      CONS_Printf("Usage: frags [team]\n");
      return;
    }
  bool team = (COM_Argc() == 2);

  //int n = GI::GetFrags(&table, team);
  /*
    for (int i=0; i<n; i++)
      {
	  CONS_Printf("%-16s", game.FindPlayer(i)->name.c_str());
	  for(j=0;j<NETMAXPLAYERS;j++)
	    if(game.FindPlayer(j))
	      CONS_Printf(" %3d",game.FindPlayer(i)->Frags[j]);
	  CONS_Printf("\n");
    }
	  CONS_Printf("%-8s",game.teams[i]->name.c_str());
	  for(j=0;j<11;j++)
	    if(teamingame(j))
	      CONS_Printf(" %3d",fragtbl[i][j]);
	  CONS_Printf("\n");
        }
    */
}





//========================================================================
//    Chat commands
//========================================================================

// helper function
static void PasteMsg(char *buf, int i)
{
  int j = COM_Argc();

  strcpy(buf, COM_Argv(i++));

  for ( ; i<j; i++)
    {
      strcat(buf, " ");
      strcat(buf, COM_Argv(i));
    }
}


void Command_Say_f()
{
  char buf[255];

  if (COM_Argc() < 2)
    {
      CONS_Printf ("say <message> : send a message\n");
      return;
    }

  if (!com_player)
    return;

  PasteMsg(buf, 1);
  game.SendChat(com_player->number, 0, buf);
}


void Command_Sayto_f()
{
  char buf[255];

  if (COM_Argc() < 3)
    {
      CONS_Printf ("sayto <playername|playernum> <message> : send a message to a player\n");
      return;
    }

  if (!com_player)
    return;

  PlayerInfo *p = game.FindPlayer(COM_Argv(1));
  if (!p)
    return;

  PasteMsg(buf, 2);
  game.SendChat(com_player->number, p->number, buf);
}


void Command_Sayteam_f()
{
  char buf[255];

  if (COM_Argc() < 2)
    {
      CONS_Printf ("sayteam <message> : send a message to your team\n");
      return;
    }

  if (!com_player)
    return;

  PasteMsg(buf, 1);
  game.SendChat(com_player->number, -com_player->team, buf);
}


// the chat message "router"
void GameInfo::SendChat(int from, int to, const char *msg)
{
  if (!netgame)
    return;

  if (from == 0 && com_player)
    from = com_player->number;

  PlayerInfo *sender = FindPlayer(from);
  if (!sender)
    return;

  if (server)
    {
      int n = net->client_con.size();
      for (int i = 0; i < n; i++)
	{
	  LConnection *c = net->client_con[i];
	  int np = c->player.size();
	  for (int j = 0; j < np; j++)
	    {
	      PlayerInfo *p = c->player[j];
	      if (to == 0 || p->number == to || p->team == -to)
		{
		  // We could also use PlayerInfo::SetMessage to trasmit chat messages from server to client,
		  // but it is mainly for messages which can easily use TNL::StringTableEntry.
		  c->rpcChat(from, to, msg);
		  if (p->number == to)
		    return; // nobody else should get it

		  break; // one rpc per connection
		}
	    }
	}

      // TODO how to handle messages for local (server) players?
      CONS_Printf("\3%s: %s\n", sender->name.c_str(), msg);
    }
  else if (net->server_con)
    net->server_con->rpcChat(from, to, msg);
}




//========================================================================
//    Basic game commands
//========================================================================

// temporarily pauses the game, or in netgame, requests pause from server
void Command_Pause_f()
{
  if (!game.server && !cv_allowpause.value)
    {
      CONS_Printf("Server allows no pauses.\n");
      return;
    }

  bool on;

  if (COM_Argc() > 1)
    on = atoi(COM_Argv(1));
  else
    on = !game.paused;

  game.Pause(on, 0);
}


static void PauseMsg(bool on, int pnum)
{
  const char *guilty = "server";
  if (pnum > 0)
    {
      PlayerInfo *p = game.FindPlayer(pnum);
      if (p)
	guilty = p->name.c_str();
    }

  if (on)
    CONS_Printf("Game paused by %s.\n", guilty);
  else
    CONS_Printf("Game unpaused by %s.\n", guilty);
}




/// pauses or unpauses the game
void GameInfo::Pause(bool on, int playernum)
{
  if (server)
    {
      // server can pause the game anytime
      paused = on;

      // send rpc event to all clients
      if (netgame && net->client_con.size())
	{
	  int n = net->client_con.size();
	  NetEvent *e = TNL_RPC_CONSTRUCT_NETEVENT(net->client_con[0], rpcPause, (on, playernum));

	  for (int i = 0; i < n; i++)
	    net->client_con[i]->postNetEvent(e);
	}

      PauseMsg(on, playernum);
    }
  else if (net->server_con)
    {
      // client must request a pause from the server
      net->server_con->rpcPause(on, 0);

      if (on)
	CONS_Printf("Pause request sent.\n");
      else
	CONS_Printf("Unpause request sent.\n");
    }
}


LCONNECTION_RPC(rpcPause, (bool on, U8 playernum), RPCGuaranteedOrdered, RPCDirAny, 0)
{
  if (isConnectionToServer())
    {
      // server orders a pause
      game.paused = on;
      PauseMsg(on, playernum);
    }
  else if (cv_allowpause.value)
    {
      // got a pause request from a client
      game.Pause(on, player[0]->number);
    }
}



// quit the game immediately
void Command_Quit_f()
{
  I_Quit();
}


// connect to a remote server
void Command_Connect_f()
{
  if (COM_Argc() < 2)
    {
      CONS_Printf("connect <serveraddress> : connect to a server\n"
		  "connect ANY : connect to the first lan server found\n");
      return;
    }

  if (game.Playing())
    {
      CONS_Printf("End the current game first.\n");
      return;
    }

  CONS_Printf("connecting...\n");

  if (!strcasecmp(COM_Argv(1), "any"))
    game.net->CL_StartPinging(true);
  else
    game.net->CL_Connect(Address(COM_Argv(1)));
}



// shuts down the current game
void Command_Reset_f()
{
  game.SV_Reset();

  if (!game.dedicated)
    game.StartIntro();
}



//========================================================================
//    Game management (server only)
//========================================================================



// load a game
void Command_Load_f() // TODO
{
  if (COM_Argc() != 2)
    {
      CONS_Printf("load <slot>: load a saved game\n");
      return;
    }

  if (!game.server)
    {
      CONS_Printf("Only the server can load a game.\n");
      return;
    }

  game.LoadGame(atoi(COM_Argv(1)));
}


// save the game
void Command_Save_f()
{
  if (COM_Argc() != 3)
    {
      CONS_Printf("save <slot> <desciption>: save game\n");
      return;
    }

  if (!game.server)
    {
      CONS_Printf("Only server can save a game\n");
      return;
    }

  int slot = atoi(COM_Argv(1));
  char *desc = COM_Argv(2);

  game.SaveGame(slot, desc);
}






//  play a demo, add .lmp for external demos
//  eg: playdemo demo1 plays the internal game demo
void Command_Playdemo_f ()
{
  char    name[256];

  if (COM_Argc() != 2)
    {
      CONS_Printf ("playdemo <demoname> : playback a demo\n");
      return;
    }

  // disconnect from server here ?
  if(game.netgame)
    {
      CONS_Printf("\nYou can't play a demo while in net game\n");
      return;
    }

  // open the demo file
  strcpy (name, COM_Argv(1));
  // dont add .lmp so internal game demos can be played
  //FIL_DefaultExtension (name, ".lmp");

  CONS_Printf ("Playing back demo '%s'.\n", name);

  game.PlayDemo(name);
}

//  stop current demo
//
void Command_Stopdemo_f ()
{
  game.CheckDemoStatus ();
  CONS_Printf ("Stopped demo.\n");
}






//  Add a pwad at run-time
//  Search for sounds, maps, musics, etc..
void Command_Addfile_f() // TODO
{
  // FIXME rewrite the "late adding of wadfiles to the resource list"-system
  if (COM_Argc() != 2)
    {
      CONS_Printf("addfile <wadfile.wad> : load wad file\n");
      return;
    }
  /*
      // here check if file exist !!!
      if( !findfile(MAPNAME,NULL,false) )
        {
	  CONS_Printf("\2File %s' not found\n",MAPNAME);
	  return;
        }
  */

  //P_AddWadFile(COM_Argv(1), NULL);
}



/// Initialize a new game using a MAPINFO lump
void Command_NewGame_f()
{
  if (COM_Argc() < 3 || COM_Argc() > 5)
    {
      CONS_Printf("Usage: newgame <MAPINFOlump> local | server [episode] [skill]\n");
      return;
    }

  if (game.Playing())
    {
      CONS_Printf("First end the current game.\n");
      return;
    }

  int sk = sk_medium;
  int epi = 1;
  if (COM_Argc() >= 4)
    {
      epi = atoi(COM_Argv(3));

      if (COM_Argc() >= 5)
	{
	  sk = atoi(COM_Argv(4));
	  sk = (sk > sk_nightmare) ? sk_nightmare : ((sk < 0) ? 0 : sk);
	}
    }

  int lump = fc.FindNumForName(COM_Argv(1));
  if (lump < 0)
    {
      CONS_Printf("MAPINFO lump '%s' not found.\n", COM_Argv(1));
      return;
    }

  if (!game.SV_SpawnServer(lump))
    return;

  if (!strcasecmp(COM_Argv(2), "server"))
    game.SV_SetServerState(true);

  if (!game.dedicated)
    {
      // add local players
      Consoleplayer.push_back(game.AddPlayer(new PlayerInfo(localplayer)));
      if (cv_splitscreen.value)
	Consoleplayer.push_back(game.AddPlayer(new PlayerInfo(localplayer2)));

      hud.ST_Start(Consoleplayer[0]);
    }

  game.StartGame(skill_t(sk), epi);
}


// starts or restarts the game
void Command_StartGame_f()
{
  if (!game.server)
    {
      CONS_Printf("Only the server can restart the game.\n");
      return;
    }

  if (!game.StartGame(game.skill, 1))
    CONS_Printf("You must first set the levelgraph!\n");
}



/// Warp to a new map.
/// Called either from map <mapname> console command, or idclev cheat.
void Command_Map_f()
{
  if (COM_Argc() < 2 || COM_Argc() > 3)
    {
      CONS_Printf("Usage: map <number|name> [<entrypoint>]: warp players to a map.\n");
      return;
    }

  if (!game.server)
    {
      CONS_Printf("Only the server can change the map.\n");
      return;
    }

  if (!game.currentcluster)
    {
      CONS_Printf("No game running.\n");
      return;
    }

  MapInfo *m = game.FindMapInfo(COM_Argv(1));
  if (!m)
    {
      CONS_Printf("No such map.\n");
      return;
    }

  int ep = atoi(COM_Argv(2));

  CONS_Printf("Warping to %s (%s)...\n", m->nicename.c_str(), m->lumpname.c_str());
  game.currentcluster->Finish(m->mapnumber, ep);
}



void Command_RestartLevel_f()
{
  if (!game.server)
    {
      CONS_Printf("Only the server can restart the level.\n");
      return;
    }


  if (game.state == GameInfo::GS_LEVEL)
    ; // game.StartLevel(true, true); FIXME
  else
    CONS_Printf("You should be in a level to restart it!\n");
}



// throws out a remote client
void Command_Kick_f()
{
  if (COM_Argc() != 2)
    {
      CONS_Printf("kick <playername> or <playernum> : kick a player\n");
      return;
    }

  if (game.server)
    {
      PlayerInfo *p = game.FindPlayer(COM_Argv(1));
      if (!p)
	CONS_Printf("there is no player with that name/number\n");
      // TODO kick him! inform others so they can remove him.
    }
  else
    CONS_Printf("You are not the server\n");
}



/*
void Got_KickCmd(char **p,int playernum)
{
  CONS_Printf("\2%s ", game.FindPlayer(pnum)->name.c_str());
  
  switch(msg)
    {
    case KICK_MSG_GO_AWAY:
      CONS_Printf("has been kicked (Go away)\n");
      break;
    case KICK_MSG_CON_FAIL:
      CONS_Printf("has been kicked (Consistency failure)\n");
      break;
    case KICK_MSG_TIMEOUT:
      CONS_Printf("left the game (Connection timeout)\n");
      break;
    case KICK_MSG_PLAYER_QUIT:
      CONS_Printf("left the game\n");
      break;
    }
  if(pnum == consoleplayer->number - 1)
    {
      CL_Reset();
      game.StartIntro();
      M_StartMessage("You have been kicked by the server\n\nPress ESC\n",NULL,MM_NOTHING);
    }
  else
    CL_RemovePlayer(pnum);
}
*/





// helper function for Command_Kill_f
static void Kill_pawn(Actor *v, Actor *k)
{
  if (v && v->health > 0)
    {
      v->flags |= MF_SHOOTABLE;
      v->flags2 &= ~(MF2_NONSHOOTABLE | MF2_INVULNERABLE);
      v->Damage(k, k, 10000, dt_always);
    }
}


// Kills just about anything
void Command_Kill_f()
{
  if (COM_Argc() < 2)
    {
      CONS_Printf ("Usage: kill me | <playernum> | monsters\n");
      // TODO extend usage: kill team
      return;
    }

  if (!game.server)
    {
      // client players can only commit suicide
      if (COM_Argc() > 2 || strcmp(COM_Argv(1), "me"))
	CONS_Printf("Only the server can kill others thru console!\n");
      else if (com_player)
	Kill_pawn(com_player->pawn, com_player->pawn);
      return;
    }

  PlayerInfo *p;
  for (int i=1; i<COM_Argc(); i++)
    {
      char *s = COM_Argv(i);
      Actor *m = NULL;

      if (!strcasecmp(s, "me") && com_player)
	m = com_player->pawn; // suicide
      else if (!strcasecmp(s, "monsters") && com_player && com_player->mp)
	{
	  // monsters
	  CONS_Printf("%d monsters killed.\n", com_player->mp->Massacre());
	  continue;
	}
      else if ((p = game.FindPlayer(s)))
	m = p->pawn; // another player by number or name
      else
	{
	  CONS_Printf("Player %s is not in the game.\n", s);
	  continue;
	}

      Kill_pawn(m, NULL); // server does the killing
    }
}
