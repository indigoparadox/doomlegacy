// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
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
// $Log: hw_main.c,v $
// Revision 1.115  2004/05/16 19:11:44  hurdler
// that should fix issues some people were having in 1280x1024 mode (and now support up to 1600x1200)
//
// Revision 1.114  2003/08/12 12:29:59  hurdler
// better translucent hud
//
// Revision 1.113  2003/07/13 13:16:15  hurdler
// go RC1
//
// Revision 1.112  2003/07/10 20:51:43  bock
// Small types fix
//
// Revision 1.111  2003/06/10 21:48:05  ssntails
// Variable flat size support (32x32 to 2048x2048)
//
// Revision 1.110  2002/05/19 19:37:27  hurdler
// adding some math functions to FraggleScript
//
// Revision 1.109  2002/01/12 02:26:47  stroggonmeth
// Uh, commit again heh
//
// Revision 1.108  2002/01/05 01:08:51  hurdler
// fix a crashing bug with phobx in splitwall (is it ok ?)
//
// Revision 1.107  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.106  2001/12/28 13:27:54  hurdler
// Manage translucent walls
//
// Revision 1.105  2001/12/27 22:50:26  hurdler
// fix a colormap bug, add scrolling floor/ceiling in hw mode
//
// Revision 1.104  2001/12/26 17:24:47  hurdler
// Update Linux version
//
// Revision 1.103  2001/12/26 15:56:11  hurdler
// Manage transparent wall a little better
//
// Revision 1.102  2001/12/15 18:41:36  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.101  2001/09/03 21:06:52  hurdler
// bug fix
//
// Revision 1.100  2001/08/27 19:59:35  hurdler
// Fix colormap in heretic + opengl, fixedcolormap and NEWCORONA
//
// Revision 1.99  2001/08/26 15:27:29  bpereira
// added fov for glide and fixed newcoronas code
//
// Revision 1.98  2001/08/19 20:41:04  hurdler
// small changes
//
// Revision 1.97  2001/08/19 15:40:07  bpereira
// added Treansform (and lighting) to glide
//
// Revision 1.96  2001/08/14 00:36:26  hurdler
// Small update
//
// Revision 1.95  2001/08/13 22:53:54  stroggonmeth
// Small commit
//
// Revision 1.94  2001/08/13 17:23:17  hurdler
// Little modif
//
// Revision 1.93  2001/08/13 17:01:40  hurdler
// Fix some coloured sector issue
//
// Revision 1.92  2001/08/13 16:27:45  hurdler
// Added translucency to linedef 300 and colormap to 3d-floors
//
// Revision 1.91  2001/08/12 22:08:40  hurdler
// Add alpha value for 3d water
//
// Revision 1.90  2001/08/12 19:05:56  hurdler
// fix a small bug with 3d water
//
// Revision 1.89  2001/08/12 17:57:16  hurdler
// Beter support of sector coloured lighting in hw mode
//
// Revision 1.88  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.87  2001/08/11 16:43:07  hurdler
// Add sector colormap in hw mode (3rd attempt)
//
// Revision 1.86  2001/08/11 15:36:56  hurdler
// Add sector colormap in hw mode (2nd attempt)
//
// Revision 1.85  2001/08/11 15:18:02  hurdler
// Add sector colormap in hw mode (first attempt)
//
// Revision 1.84  2001/08/11 00:51:59  hurdler
// Sort 3D water for better alpha blending
//
// Revision 1.83  2001/08/09 21:35:23  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.82  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.81  2001/08/07 00:44:05  hurdler
// MD2 implementation is getting better but still need lots of work
//
// Revision 1.80  2001/08/06 23:55:02  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.79  2001/08/06 14:13:46  hurdler
// Crappy MD2 implementation (still need lots of work)
//
// Revision 1.78  2001/08/03 15:16:11  hurdler
// Fix small bugs (win2k timer + status bar)
//
// Revision 1.77  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.76  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.75  2001/05/16 21:21:15  bpereira
// no message
//
// Revision 1.74  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.73  2001/05/07 20:27:16  stroggonmeth
// no message
//
// Revision 1.72  2001/05/03 21:52:23  hurdler
// fix bis
//
// Revision 1.71  2001/05/03 20:05:56  hurdler
// small opengl fix
//
// Revision 1.70  2001/05/01 20:38:34  hurdler
// some fix/hack for the beta release
//
// Revision 1.69  2001/04/30 21:00:10  stroggonmeth
// Another HWR fix
//
// Revision 1.68  2001/04/30 17:19:25  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.67  2001/04/27 13:32:14  bpereira
// no message
//
// Revision 1.66  2001/04/18 23:22:00  hurdler
// Until SoM fix it more properly than me ;)
//
// Revision 1.65  2001/04/18 22:53:55  hurdler
// Until SoM fix it more properly than me ;)
//
// Revision 1.64  2001/04/16 21:40:06  hurdler
// Fix misaligned midtexture problem (to verify!)
//
// Revision 1.63  2001/04/16 15:16:45  hurdler
// minor changes
//
// Revision 1.62  2001/04/09 23:26:05  hurdler
// clean up
//
// Revision 1.61  2001/04/09 20:24:28  metzgermeister
// turned it on again :)
//
// Revision 1.60  2001/04/09 14:18:21  hurdler
// no message
//
// Revision 1.59  2001/04/08 10:15:54  bpereira
// no message
//
// Revision 1.58  2001/04/03 23:15:39  hurdler
// FIXME: this code adds basic 3D-floors support in hw mode
//
// Revision 1.57  2001/03/25 18:11:24  metzgermeister
//   * SDL sound bug with swapped stereo channels fixed
//   * separate hw_trick.c now for HW_correctSWTrick(.)
//
// Revision 1.56  2001/03/20 21:26:21  hurdler
// use PI instead of M_PI as it  is defined in hw_defs.h (M_PI undefined under win32 by default)
//
// Revision 1.55  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.54  2001/03/19 18:15:59  hurdler
// fix a fog problem
//
// Revision 1.53  2001/03/13 22:14:21  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.52  2001/02/24 13:35:22  bpereira
// no message
//
// Revision 1.51  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.50  2001/01/31 17:15:09  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.49  2001/01/25 18:56:27  bpereira
// no message
//
// Revision 1.48  2000/11/18 15:51:25  bpereira
// no message
//
// Revision 1.47  2000/11/12 21:04:28  hurdler
// Fix a bug with validcount and boom code (dyn.light looked ugly with boom water)
//
// Revision 1.46  2000/11/11 13:59:47  bpereira
// no message
//
// Revision 1.45  2000/11/02 22:14:02  bpereira
// no message
//
// Revision 1.44  2000/11/02 21:54:26  bpereira
// no message
//
// Revision 1.43  2000/11/02 19:49:39  bpereira
// no message
//
// Revision 1.42  2000/10/22 14:16:41  hurdler
// Prepare code for TANDL
//
// Revision 1.41  2000/10/21 08:43:32  bpereira
// no message
//
// Revision 1.40  2000/10/04 16:21:57  hurdler
// small clean-up
//
// Revision 1.39  2000/10/02 18:25:46  bpereira
// no message
//
// Revision 1.38  2000/10/01 15:18:38  hurdler
// no message
//
// Revision 1.37  2000/10/01 10:18:23  bpereira
// no message
//
// Revision 1.36  2000/10/01 09:10:19  hurdler
// Put the md2 code in #ifdef TANDL
//
// Revision 1.35  2000/10/01 01:22:35  hurdler
// remove polysky, enable PF_Environment for sprites
//
// Revision 1.34  2000/09/28 20:57:20  bpereira
// no message
//
// Revision 1.33  2000/09/21 16:45:11  bpereira
// no message
//
// Revision 1.32  2000/08/31 14:30:57  bpereira
// no message
//
// Revision 1.31  2000/08/21 21:13:58  metzgermeister
// crash on polys>256verts fixed
//
// Revision 1.30  2000/08/11 19:11:57  metzgermeister
// *** empty log message ***
//
// Revision 1.29  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.28  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.27  2000/06/08 19:40:34  hurdler
// my changes before splitting (can be reverted in development branch)
//
// Revision 1.26  2000/05/30 18:03:22  kegetys
// Wall, floor and ceiling lighting is now done by changing only the RGB, not the alpha
//
// Revision 1.25  2000/05/10 17:45:35  kegetys
// no message
//
// Revision 1.24  2000/05/05 18:00:05  bpereira
// no message
//
// Revision 1.23  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.22  2000/04/27 23:41:16  hurdler
// better splitscreen support in OpenGL mode
//
// Revision 1.21  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.20  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.19  2000/04/24 15:46:34  hurdler
// Support colormap for text
//
// Revision 1.18  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.17  2000/04/23 12:50:32  hurdler
// support filter mode in OpenGL
//
// Revision 1.16  2000/04/22 21:08:23  hurdler
// I like it better like that
//
// Revision 1.15  2000/04/22 16:09:14  hurdler
// support skin color in hardware mode
//
// Revision 1.14  2000/04/18 16:07:16  hurdler
// better support of decals
//
// Revision 1.13  2000/04/18 12:52:21  hurdler
// join with Boris' code
//
// Revision 1.11  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.10  2000/04/12 16:03:51  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.9  2000/04/09 01:59:06  hurdler
// removed warnings
//
// Revision 1.8  2000/04/08 11:28:46  hurdler
// added boom water support
//
// Revision 1.7  2000/03/29 19:39:49  bpereira
// no message
//
// Revision 1.6  2000/03/08 17:02:05  hurdler
// fix the joiningame problem under Linux
//
// Revision 1.5  2000/03/07 14:22:48  hurdler
// no message
//
// Revision 1.4  2000/03/06 16:52:06  hurdler
// hack for OpenGL / Open Entry problem
//
// Revision 1.3  2000/03/05 17:10:56  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      hardware renderer, using the standard HardWareRender driver DLL for Doom Legacy
//
//-----------------------------------------------------------------------------

#include <math.h>

#include "hw_glob.h"
#include "hw_light.h"

#include "../doomstat.h"
#include "../i_video.h" // added by Hurdler for rendermode == render_glide
#include "../v_video.h"
#include "../p_local.h"
#include "../p_setup.h"
#include "../r_local.h"
#include "../d_clisrv.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../r_splats.h"
#include "../t_func.h"

#define ZCLIP_PLANE     4.0f
#define R_FAKEFLOORS

// ==========================================================================
// the hardware driver object
// ==========================================================================
struct hwdriver_s hwdriver;

// ==========================================================================
//                                                                     PROTOS
// ==========================================================================
#define SPRITE_LIGHTLEVEL

#ifdef SPRITE_LIGHTLEVEL
static void HWR_AddSprites(sector_t* sec, int lightlevel);
#else  
static void HWR_AddSprites(sector_t * sec);
#endif
static void HWR_ProjectSprite(mobj_t * thing);
static void HWR_Add3DWater(int lumpnum, extrasubsector_t * xsub, fixed_t fixedheight, int lightlevel, int alpha);
static void HWR_Render3DWater();
static void HWR_RenderTransparentWalls();

static unsigned int atohex(char *s);

// ==========================================================================
//                                          3D ENGINE COMMANDS & CONSOLE VARS
// ==========================================================================

CV_PossibleValue_t grmlook_extends_fov_cons_t[] = { {0, "Off"}, {1, "On"}, {2, "Full"}, {0, NULL} };
CV_PossibleValue_t grgamma_cons_t[] = { {1, "MIN"}, {255, "MAX"}, {0, NULL} };
CV_PossibleValue_t grfov_cons_t[] = { {0, "MIN"}, {179, "MAX"}, {0, NULL} };
CV_PossibleValue_t grfiltermode_cons_t[] = { {HWD_SET_TEXTUREFILTER_POINTSAMPLED, "Nearest"},
{HWD_SET_TEXTUREFILTER_BILINEAR, "Bilinear"},
{HWD_SET_TEXTUREFILTER_TRILINEAR, "Trilinear"},
{HWD_SET_TEXTUREFILTER_MIXED1, "Linear_Nearest"},
{HWD_SET_TEXTUREFILTER_MIXED2, "Nearest_Linear"},
{0, NULL}
};

// BP: change directely the palette to see the change
void CV_Gammaxxx_ONChange(void)
{
    V_SetPalette(0);
}

void CV_filtermode_ONChange(void);
void CV_FogDensity_ONChange(void);
//static void CV_grFogColor_OnChange (void);
static void CV_grFov_OnChange(void);
//static void CV_grMonsterDL_OnChange (void);
static void CV_grPolygonSmooth_OnChange(void);

consvar_t cv_grrounddown = { "gr_rounddown", "Off", 0, CV_OnOff };
consvar_t cv_grmlook_extends_fov = { "gr_mlook", "Full", CV_SAVE, grmlook_extends_fov_cons_t };
consvar_t cv_grfov = { "gr_fov", "90", CV_SAVE | CV_CALL, grfov_cons_t, CV_grFov_OnChange };
consvar_t cv_grsky = { "gr_sky", "On", 0, CV_OnOff };
	// cv_grsky not registered, cannot use from console
consvar_t cv_grfog = { "gr_fog", "On", CV_SAVE, CV_OnOff };
consvar_t cv_grfogcolor = { "gr_fogcolor", "000000", CV_SAVE, NULL };
consvar_t cv_grfogdensity = { "gr_fogdensity", "100", CV_SAVE | CV_CALL | CV_NOINIT, CV_Unsigned, CV_FogDensity_ONChange };
consvar_t cv_grgammared = { "gr_gammared", "127", CV_SAVE | CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grgammagreen = { "gr_gammagreen", "127", CV_SAVE | CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grgammablue = { "gr_gammablue", "127", CV_SAVE | CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grfiltermode = { "gr_filtermode", "Bilinear", CV_SAVE | CV_CALL, grfiltermode_cons_t, CV_filtermode_ONChange };
consvar_t cv_grzbuffer = { "gr_zbuffer", "On", 0, CV_OnOff };
	// cv_grzbuffer not registered, cannot use from console
consvar_t cv_grcorrecttricks = { "gr_correcttricks", "On", 0, CV_OnOff };
consvar_t cv_grsolvetjoin = { "gr_solvetjoin", "On", 0, CV_OnOff };

// console variables in development
consvar_t cv_grpolygonsmooth = { "gr_polygonsmooth", "Off", CV_CALL, CV_OnOff, CV_grPolygonSmooth_OnChange };
consvar_t cv_grmd2 = { "gr_md2", "Off", 0, CV_OnOff };
consvar_t cv_grtranswall = { "gr_transwall", "Off", 0, CV_OnOff };
	// cv_grtranswall not registered, cannot use from console

// faB : needs fix : walls are incorrectly clipped one column less
const consvar_t cv_grclipwalls = { "gr_clipwalls", "Off", 0, CV_OnOff };
	// cv_grclipwalls not registered, cannot use from console

//development variables for diverse uses
consvar_t cv_gralpha = { "gr_alpha", "160", 0, CV_Unsigned };
consvar_t cv_grbeta = { "gr_beta", "0", 0, CV_Unsigned };
consvar_t cv_grgamma = { "gr_gamma", "0", 0, CV_Unsigned };

void CV_FogDensity_ONChange(void)
{
    HWD.pfnSetSpecialState(HWD_SET_FOG_DENSITY, cv_grfogdensity.value);
}

void CV_filtermode_ONChange(void)
{
    HWD.pfnSetSpecialState(HWD_SET_TEXTUREFILTERMODE, cv_grfiltermode.value);
}

// ==========================================================================
//                                                               VIEW GLOBALS
// ==========================================================================
// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW      ANG90

angle_t gr_clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int gr_viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t gr_xtoviewangle[MAXVIDWIDTH + 1];

// ==========================================================================
//                                                                    GLOBALS
// ==========================================================================

// uncomment to remove the plane rendering
#define DOPLANES
//#define DOWALLS

// BP: test of draw sky by polygone like in sofware with visplane, unfortunately
// this don't work since we must have z for pixel and z for texture (not like now with z=oow)
//#define POLYSKY

// BP: test change fov when looking up/down but bsp projection messup :(
//#define NO_MLOOK_EXTENDS_FOV

#define drawtextured  true
//static  boolean     drawtextured = false;

// base values set at SetViewSize
float gr_basecentery;
float gr_baseviewwindowy;
float gr_basewindowcentery;

float gr_viewwidth;             // viewport clipping boundaries (screen coords)
float gr_viewheight;
float gr_centerx;
float gr_centery;
float gr_viewwindowx;           // top left corner of view window
float gr_viewwindowy;
float gr_windowcenterx;         // center of view window, for projection
float gr_windowcentery;

float gr_pspritexscale;
float gr_pspriteyscale;

seg_t *gr_curline;
side_t *gr_sidedef;
line_t *gr_linedef;
sector_t *gr_frontsector;
sector_t *gr_backsector;
//FRGBAFloat  gr_cursectorlight;      // colour & intensity of current sector's lighting

// --------------------------------------------------------------------------
//                                              STUFF FOR THE PROJECTION CODE
// --------------------------------------------------------------------------

FTransform atransform;
// duplicates of the main code, set after R_SetupFrame() passed them into sharedstruct,
// copied here for local use
static fixed_t dup_viewx;
static fixed_t dup_viewy;
static fixed_t dup_viewz;
static angle_t dup_viewangle;

static float gr_viewx;
static float gr_viewy;
float gr_viewz;
static float gr_viewsin;
static float gr_viewcos;

//04/01/00: Maybe not necessary with the new T&L code (need to be checked!)
static float gr_viewludsin;     //look up down kik test
static float gr_viewludcos;

static float gr_fovlud;

// ==========================================================================
//                                    Transforms
// ==========================================================================

#define TERM_TRANSFORM
#ifdef TERM_TRANSFORM
// The only terms needed, the other terms are 0.
// Don't try to make this a matrix, this is much easier to understand and maintain.
static float world_trans_x_to_x, world_trans_y_to_x,
  world_trans_x_to_y, world_trans_y_to_y, world_trans_z_to_y,
  world_trans_x_to_z, world_trans_y_to_z, world_trans_z_to_z;
static float sprite_trans_x_to_x, sprite_trans_y_to_y, sprite_trans_z_to_y,
  sprite_trans_z_to_z, sprite_trans_y_to_z;

void HWR_set_view_transform( void )
{
    // Combined transforms for position, direction, look up/down, and scaling
    // translation is separate and done first
    // In order:
    //   rotation around vertical y axis
    //   look up/down
    //   scale y before frustum so that frustum can be scaled to screen height
    world_trans_x_to_x = gr_viewsin * gr_fovlud;
    world_trans_y_to_x = -gr_viewcos * gr_fovlud;
    world_trans_x_to_y = gr_viewcos * gr_viewludcos * ORIGINAL_ASPECT * gr_fovlud;
    world_trans_y_to_y = gr_viewsin * gr_viewludcos * ORIGINAL_ASPECT * gr_fovlud;
    world_trans_z_to_y = gr_viewludsin * ORIGINAL_ASPECT * gr_fovlud;
    world_trans_x_to_z = gr_viewcos * gr_viewludsin;
    world_trans_y_to_z = gr_viewsin * gr_viewludsin;
    world_trans_z_to_z = -gr_viewludcos;

    // look up/down and scaling
    sprite_trans_x_to_x = gr_fovlud;
    sprite_trans_y_to_y = gr_viewludsin * ORIGINAL_ASPECT * gr_fovlud;
    sprite_trans_z_to_y = gr_viewludcos * ORIGINAL_ASPECT * gr_fovlud;
    sprite_trans_z_to_z = gr_viewludsin;
    sprite_trans_y_to_z = -gr_viewludcos;
}

#if 0
// unused because of confused wall drawing
// wx,wy,wz in world coord, to screen FOutVector
void HWR_transform_world_FOut(float wx, float wy, float wz, FOutVector * fovp)
{
    // Combined transforms for position, direction, look up/down, and scaling.
    // translation
    // x world, to x screen
    // vert z world, to vert y screen
    // y world, to screen depth
    float tr_x = wx - gr_viewx;
    float tr_y = wy - gr_viewy;
    float tr_z = wz - gr_viewz;
    fovp->x = (tr_x * world_trans_x_to_x)
       + (tr_y * world_trans_y_to_x);
    fovp->y = (tr_x * world_trans_x_to_y )
       + (tr_y * world_trans_y_to_y )
       + (tr_z * world_trans_z_to_y );
    fovp->z = (tr_x * world_trans_x_to_z )
       + (tr_y * world_trans_y_to_z )
       + (tr_z * world_trans_z_to_z );
}
#endif

void HWR_transform_sprite_FOut(float cx, float cy, float cz, FOutVector * fovp)
{
    // Combined transforms for look up/down, and scaling
   fovp->y = (cy * sprite_trans_y_to_y) + (cz * sprite_trans_z_to_y);
   fovp->z = (cy * sprite_trans_y_to_z) + (cz * sprite_trans_z_to_z);
   fovp->x = (cx * sprite_trans_x_to_x);
}
#endif

// ==========================================================================
//                                    LIGHT stuffs
// ==========================================================================

static byte lightleveltonumlut[256];

// added to Doom's sector lightlevel to make things a bit brighter (sprites/walls/planes)
byte LightLevelToLum(int l)
{
    if (fixedcolormap)
        return 255;
    l = lightleveltonumlut[l];
    l += (extralight << 4);	// from guns
    if (l > 255)
        l = 255;
    return l;
}

// Need to select extra
byte LightLevelToLum_extra(int l, int extra)
{
    if (fixedcolormap)
        return 255;
    l = lightleveltonumlut[l];
    l += (extra << 4);	// from guns, etc..
    if (l > 255)
        l = 255;
    return l;
}


static void InitLumLut()
{
    int i, k;
    for (i = 0; i < 256; i++)
    {
        // this polygone is the solution of equ : f(0)=0, f(1)=1 f(.5)=.5, f'(0)=0, f'(1)=0), f'(.5)=K
//#define K   2
// [WDJ] Reduce to match software renderer brightness
#define K   1.2f
#define A  (-24+16*K)
#define B  ( 60-40*K)
#define C  (32*K-50)
#define D  (-8*K+15)
        float x = (float) i / 255;
        float xx, xxx;
        xx = x * x;
        xxx = x * xx;
        k = 255 * (A * xx * xxx + B * xx * xx + C * xxx + D * xx);

        lightleveltonumlut[i] = min(255, k);
    }
}

// ==========================================================================
//                                   FLOOR/CEILING GENERATION FROM SUBSECTORS
// ==========================================================================

#ifdef  DOPLANES

//what is the maximum number of verts around a convex floor/ceiling polygon? FIXME: gothic2 map02 has a 304 vertex poly!!!!
#define MAXPLANEVERTICES 256
static FOutVector planeVerts[MAXPLANEVERTICES];

// -----------------+
// HWR_RenderPlane  : Render a floor or ceiling convex polygon
// -----------------+
// Called from HWR_Subsector, HWR_Render3DWater
void HWR_RenderPlane(extrasubsector_t * xsub, fixed_t fixedheight,
		     FBITFIELD PolyFlags, int lightlevel, int lumpnum)    // SoM: 3D floors ect.
{
    polyvertex_t *pv;
    float height;               //constant y for all points on the convex flat polygon
    FOutVector *v3d;
    int nrPlaneVerts;           //verts original define of convex flat polygon
    int i;
    float flatxref, flatyref;
    double flatsize;
    int flatflag;
    int size;

    FSurfaceInfo Surf;

    // no convex poly were generated for this subsector
    if (!xsub->planepoly)
        return;

    height = FIXED_TO_FLOAT( fixedheight );

    pv = xsub->planepoly->pts;
    nrPlaneVerts = xsub->planepoly->numpts;

    if (nrPlaneVerts < 3)       //not even a triangle ?
        return;

    if (nrPlaneVerts > MAXPLANEVERTICES)        // FIXME: exceeds plVerts size
    {
        CONS_Printf("polygon size of %d exceeds max value of %d vertices\n", nrPlaneVerts, MAXPLANEVERTICES);
        return;
    }

    size = W_LumpLength(lumpnum);

    switch (size)
    {
        case 4194304:  // 2048x2048 lump
            flatsize = 2048.0f;
            flatflag = 2047;
            break;
        case 1048576:  // 1024x1024 lump
            flatsize = 1024.0f;
            flatflag = 1023;
            break;
        case 262144:   // 512x512 lump
            flatsize = 512.0f;
            flatflag = 511;
            break;
        case 65536:    // 256x256 lump
            flatsize = 256.0f;
            flatflag = 255;
            break;
        case 16384:    // 128x128 lump
            flatsize = 128.0f;
            flatflag = 127;
            break;
        case 1024:     // 32x32 lump
            flatsize = 32.0f;
            flatflag = 31;
            break;
        default:       // 64x64 lump
            flatsize = 64.0f;
            flatflag = 63;
            break;
    }

    //reference point for flat texture coord for each vertex around the polygon
    flatxref = ((fixed_t) pv->x & (~flatflag)) / flatsize;
    flatyref = ((fixed_t) pv->y & (~flatflag)) / flatsize;

    // transform
    v3d = planeVerts;
    for (i = 0; i < nrPlaneVerts; i++, v3d++, pv++)
    {
        // Hurdler: add scrolling texture on floor/ceiling
        float scrollx = 0.0f, scrolly = 0.0f;
        if (gr_frontsector)
        {
            if (fixedheight < dup_viewz)        // it's a floor
            {
                scrollx = gr_frontsector->floor_xoffs * (FIXED_TO_FLOAT_MULT / flatsize);
                scrolly = gr_frontsector->floor_yoffs * (FIXED_TO_FLOAT_MULT / flatsize);
            }
            else
            {
                scrollx = gr_frontsector->ceiling_xoffs * (FIXED_TO_FLOAT_MULT / flatsize);
                scrolly = gr_frontsector->ceiling_yoffs * (FIXED_TO_FLOAT_MULT / flatsize);
            }
        }
        v3d->sow = (pv->x / flatsize) - flatxref + scrollx;
        v3d->tow = flatyref - (pv->y / flatsize) + scrolly;
        v3d->x = pv->x;
        v3d->y = height;
        v3d->z = pv->y;
    }

    // only useful for flat coloured triangles
    //Surf.FlatColor = 0xff804020;

    //  use different light tables
    //  for horizontal / vertical / diagonal
    //  note: try to get the same visual feel as the original
    Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue =
       LightLevelToLum(lightlevel);        // SoM: Don't take from the frontsector

    //Hurdler: colormap test
    if (gr_frontsector && !fixedcolormap)
    {
        sector_t *sector = gr_frontsector;

        if (gr_frontsector->ffloors)
        {
            ffloor_t *caster;

            caster = gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, fixedheight, false)].caster;
            sector = caster ? &sectors[caster->secnum] : gr_frontsector;
        }
        if (sector && sector->extra_colormap)
        {
            RGBA_t temp;
            int light;
            int alpha;

            light = LightLevelToLum(lightlevel);
            temp.rgba = sector->extra_colormap->rgba;
            alpha = temp.s.alpha;
            Surf.FlatColor.s.red = ((26 - alpha) * light + alpha * temp.s.red) / 26;
            Surf.FlatColor.s.blue = ((26 - alpha) * light + alpha * temp.s.blue) / 26;
            Surf.FlatColor.s.green = ((26 - alpha) * light + alpha * temp.s.green) / 26;
            Surf.FlatColor.s.alpha = 0xff;
        }
    }

    if ((PolyFlags & PF_Translucent) && !fixedcolormap)
    {
        Surf.FlatColor.s.alpha = PolyFlags >> 24;
        HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts, PF_Translucent | PF_Modulated | PF_Occlude | PF_Clip);
    }
    else
    {
        Surf.FlatColor.s.alpha = 0xff;
        HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts, PolyFlags | PF_Masked | PF_Modulated | PF_Clip);
    }

    //12/11/99: Hurdler: add here code for dynamic lighting on planes
    HWR_PlaneLighting(planeVerts, nrPlaneVerts);

    //experimental code: shadow of the player: should be done only on floor
    //HWR_DynamicShadowing(planeVerts, nrPlaneVerts, plVerts, plyr);
}

#ifdef POLYSKY
// this don't draw anything it only update the z-buffer so there isn't problem with
// wall/things upper that sky (map12)
// Called from HWR_Subsector
static void HWR_RenderSkyPlane(extrasubsector_t * xsub, fixed_t fixedheight)
//                              FBITFIELD         PolyFlags )
{
    polyvertex_t *pv;
    float height;               //constant y for all points on the convex flat polygon
    FOutVector *v3d;
    int nrPlaneVerts;           //verts original define of convex flat polygon
    int i;
    float flatxref, flatyref;

    // no convex poly were generated for this subsector
    if (!xsub->planepoly)
        return;

    height = FIXED_TO_FLOAT( fixedheight );

    pv = xsub->planepoly->pts;
    nrPlaneVerts = xsub->planepoly->numpts;

    if (nrPlaneVerts < 3)       //not even a triangle ?
        return;

    //HWR_GetTexture (skytexture);

    //reference point for flat texture coord for each vertex around the polygon
    flatxref = ((fixed_t) pv->x & (~63)) / 64.0f;
    flatyref = ((fixed_t) pv->y & (~63)) / 64.0f;

    // transform
    v3d = planeVerts;
    for (i = 0; i < nrPlaneVerts; i++, v3d++, pv++)
    {
        v3d->sow = must be transformed and projected !;
        v3d->tow = must be transformed and projected !;
        v3d->x = pv->x;
        v3d->y = height;
        v3d->z = pv->y;
    }

    HWD.pfnDrawPolygon(NULL, planeVerts, nrPlaneVerts, PF_Invisible | PF_Occlude | PF_Masked | PF_Clip);
}
#endif //polysky

#endif //doplanes

/*
   wallVerts order is :
          3--2
          | /|
          |/ |
          0--1
*/
#ifdef WALLSPLATS
// Called from HWR_ProjectWall, HWR_RenderWall
void HWR_DrawSegsSplats(FSurfaceInfo * pSurf)
{
    FOutVector trVerts[4], *wv;
    wallVert3D wallVerts[4];
    wallVert3D *pwallVerts;
    wallsplat_t *splat;
    GlidePatch_t *gpatch;
    int i;
    FSurfaceInfo pSurf2;
    // seg bbox
    fixed_t segbbox[4];

    M_ClearBox(segbbox);
    M_AddToBox(segbbox, ((polyvertex_t *) gr_curline->v1)->x * FIXED_TO_FLOAT_MULT,
	       ((polyvertex_t *) gr_curline->v1)->y / FIXED_TO_FLOAT_MULT);
    M_AddToBox(segbbox, ((polyvertex_t *) gr_curline->v2)->x / FIXED_TO_FLOAT_MULT,
	       ((polyvertex_t *) gr_curline->v2)->y / FIXED_TO_FLOAT_MULT);

    // splat are drawn by line but this func is called for eatch segs of a line
    /*
       BP: DONT WORK BECAUSE Z-buffer !!!!
       FIXME : the splat must be stored by segs !
       if( gr_curline->linedef->splatdrawn == validcount )
       return;
       gr_curline->linedef->splatdrawn = validcount;
     */

    splat = (wallsplat_t *) gr_curline->linedef->splats;
    for (; splat; splat = splat->next)
    {
        // For each splat
        //BP: don't draw splat extern to this seg
        //    this is quick fix best is explain in logboris.txt at 12-4-2000
        if (!M_PointInBox(segbbox, splat->v1.x, splat->v1.y)
	    && !M_PointInBox(segbbox, splat->v2.x, splat->v2.y))
            continue;

        gpatch = W_CachePatchNum(splat->patch, PU_CACHE);
        HWR_GetPatch(gpatch);

        // FIXME: can do without temp wallVerts,
        // they are only copied into trVerts.
	// Consider unrolling the loop and merge with the first assigns.
        wallVerts[0].x = wallVerts[3].x = FIXED_TO_FLOAT( splat->v1.x );
        wallVerts[0].z = wallVerts[3].z = FIXED_TO_FLOAT( splat->v1.y );
        wallVerts[2].x = wallVerts[1].x = FIXED_TO_FLOAT( splat->v2.x );
        wallVerts[2].z = wallVerts[1].z = FIXED_TO_FLOAT( splat->v2.y );

        // i is temp y
        i = splat->top;
        if (splat->yoffset)
            i += *splat->yoffset;

        wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( i ) + (gpatch->height >> 1);
        wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( i ) - (gpatch->height >> 1);

        wallVerts[3].s = wallVerts[3].t = wallVerts[2].s = wallVerts[0].t = 0.0f;
        wallVerts[1].s = wallVerts[1].t = wallVerts[2].t = wallVerts[0].s = 1.0f;

        // transform
        wv = trVerts;
        pwallVerts = wallVerts;
        for (i = 0; i < 4; i++, wv++, pwallVerts++)
        {
            wv->x = pwallVerts->x;
            wv->z = pwallVerts->z;
            wv->y = pwallVerts->y;

            wv->sow = pwallVerts->s;
            wv->tow = pwallVerts->t;
        }
        memcpy(&pSurf2, pSurf, sizeof(FSurfaceInfo));
        switch (splat->flags & SPLATDRAWMODE_MASK)
        {
            case SPLATDRAWMODE_OPAQUE:
                pSurf2.FlatColor.s.alpha = 0xff;
                i = PF_Translucent;
                break;
            case SPLATDRAWMODE_TRANS:
                pSurf2.FlatColor.s.alpha = 128;
                i = PF_Translucent;
                break;
            case SPLATDRAWMODE_SHADE:
                pSurf2.FlatColor.s.alpha = 0xff;
                i = PF_Substractive;
                break;
        }

        HWD.pfnDrawPolygon(&pSurf2, trVerts, 4, i | PF_Modulated | PF_Clip | PF_Decal);
    }
}
#endif

// ==========================================================================
//                                        WALL GENERATION FROM SUBSECTOR SEGS
// ==========================================================================
// Called from HWR_StoreWallRange, HWR_DrawSprite
int HWR_TranstableToAlpha(int transtablenum, FSurfaceInfo * pSurf)
{
    switch (transtablenum)
    {
        case tr_transmed:
            pSurf->FlatColor.s.alpha = 0x80;
            return PF_Translucent;
        case tr_transmor:
            pSurf->FlatColor.s.alpha = 0x40;
            return PF_Translucent;
        case tr_transhi:
            pSurf->FlatColor.s.alpha = 0x30;
            return PF_Translucent;
        case tr_transfir:
            pSurf->FlatColor.s.alpha = 0x80;
            return PF_Additive;
        case tr_transfx1:
            pSurf->FlatColor.s.alpha = 0xff;
            return PF_Translucent;
    }
    return PF_Translucent;
}

// v1,v2 : the start & end vertices along the original wall segment, that may have been
//         clipped so that only a visible portion of the wall seg is drawn.
// floorheight, ceilingheight : depend on wall upper/lower/middle, comes from the sectors.

void HWR_AddTransparentWall(wallVert3D * wallVerts, FSurfaceInfo * pSurf, int texnum, int blend);

// -----------------+
// HWR_ProjectWall  :
// -----------------+
/*
   wallVerts order is :
          3--2
          | /|
          |/ |
          0--1
*/
// Called from HWR_SplitWall, HWR_StoreWallRange
void HWR_ProjectWall(wallVert3D * wallVerts, FSurfaceInfo * pSurf, int blendmode)
{
    FOutVector trVerts[4];
    int i;
    FOutVector *wv;

    // transform
    wv = trVerts;
    // it sounds really stupid to do this conversion with the new T&L code
    // we should directly put the right information in the right structure
    // wallVerts3D seems ok, doesn't need FOutVector
    // also remove the light copy
    for (i = 0; i < 4; i++, wv++, wallVerts++)
    {
        wv->sow = wallVerts->s;
        wv->tow = wallVerts->t;
        wv->x = wallVerts->x;
        wv->y = wallVerts->y;
        wv->z = wallVerts->z;
    }

    HWD.pfnDrawPolygon(pSurf, trVerts, 4, blendmode | PF_Modulated | PF_Occlude | PF_Clip);

    if (gr_curline->linedef->splats && cv_splats.value)
        HWR_DrawSegsSplats(pSurf);

    //Hurdler: TDOD: do static lighting using gr_curline->lm
    HWR_WallLighting(trVerts);

    //Hurdler: for better dynamic light in dark area, we should draw the light first
    //         and then the wall all that with the right blending func
    //HWD.pfnDrawPolygon( pSurf, trVerts, 4, PF_Additive|PF_Modulated|PF_Occlude|PF_Clip);
}

// ==========================================================================
//                                                          BSP , CULL, ETC..
// ==========================================================================

// return the frac from the interception of the clipping line
// (in fact a clipping plane that has a constant, so can clip with simple 2d)
// with the wall segment
//
static float HWR_ClipViewSegment(int x, polyvertex_t * v1, polyvertex_t * v2)
{
    float num;
    float den;
    float v1x;
    float v1y;
    float v1dx;
    float v1dy;
    float v2dx;
    float v2dy;

    angle_t clipangle = gr_xtoviewangle[x];

    // a segment of a polygon
    v1x = v1->x;
    v1y = v1->y;
    v1dx = (v2->x - v1->x);
    v1dy = (v2->y - v1->y);

    // the clipping line
    clipangle = clipangle + dup_viewangle;      //back to normal angle (non-relative)
    v2dx = FIXED_TO_FLOAT( finecosine[clipangle >> ANGLETOFINESHIFT] );
    v2dy = FIXED_TO_FLOAT( finesine[clipangle >> ANGLETOFINESHIFT] );

    den = v2dy * v1dx - v2dx * v1dy;
    if (den == 0)
        return -1;      // parallel

    // calc the frac along the polygon segment,
    //num = (v2x - v1x)*v2dy + (v1y - v2y)*v2dx;
    //num = -v1x * v2dy + v1y * v2dx;
    num = (gr_viewx - v1x) * v2dy + (v1y - gr_viewy) * v2dx;

    return num / den;
}

//
// HWR_SplitWall
//
static void HWR_SplitWall(sector_t * sector, wallVert3D * wallVerts, int texnum, FSurfaceInfo * Surf, int cutflag)
{
    /*
       SoM: split up and light walls according to the
       lightlist. This may also include leaving out parts
       of the wall that can't be seen
     */
    GlideTexture_t *glTex;
    float realtop, realbot, top, bot;
    float pegt, pegb, pegmul;
    float height, bheight = 0;
    int solid, i;
    lightlist_t *list = sector->lightlist;

    realtop = top = wallVerts[2].y;
    realbot = bot = wallVerts[0].y;
    pegt = wallVerts[2].t;
    pegb = wallVerts[0].t;
    pegmul = (pegb - pegt) / (top - bot);

    for (i = 1; i < sector->numlights; i++)
    {
        if (top < realbot)
            return;

        //Hurdler: fix a crashing bug, but is it correct?
        //if (!list[i].caster)
        //    continue;

        if (list[i].caster)
            solid = list[i].caster->flags & cutflag;
        else
            solid = false;

        height = FIXED_TO_FLOAT( list[i].height );
        if (solid)
            bheight = FIXED_TO_FLOAT( *list[i].caster->bottomheight );

        if (height >= top)
        {
            if (solid && top > bheight)
                top = bheight;
            continue;
        }

        //Found a break;
        bot = height;

        if (bot < realbot)
            bot = realbot;

        if (!fixedcolormap)
        {
            FUINT lightnum;
            sector_t *sector;

            lightnum = LightLevelToLum(*list[i - 1].lightlevel);
            // store Surface->FlatColor to modulate wall texture
            Surf->FlatColor.s.red = Surf->FlatColor.s.green = Surf->FlatColor.s.blue = lightnum;

            //Hurdler: colormap test
            sector = list[i - 1].caster ? &sectors[list[i - 1].caster->secnum] : gr_frontsector;
            if (sector->extra_colormap)
            {
                RGBA_t temp;
                int alpha;

                temp.rgba = sector->extra_colormap->rgba;
                alpha = temp.s.alpha;
                Surf->FlatColor.s.red = ((26 - alpha) * lightnum + alpha * temp.s.red) / 26;
                Surf->FlatColor.s.blue = ((26 - alpha) * lightnum + alpha * temp.s.blue) / 26;
                Surf->FlatColor.s.green = ((26 - alpha) * lightnum + alpha * temp.s.green) / 26;
                Surf->FlatColor.s.alpha = 0xff;
            }
        }

        wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
        wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);

        // set top/bottom coords
        wallVerts[2].y = wallVerts[3].y = top;
        wallVerts[0].y = wallVerts[1].y = bot;

        glTex = HWR_GetTexture(texnum);
        if (glTex->mipmap.flags & TF_TRANSPARENT)
            HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Environment);
        else
            HWR_ProjectWall(wallVerts, Surf, PF_Masked);

        if (solid)
            top = bheight;
        else
            top = height;
    }

    bot = realbot;
    if (top <= realbot)
        return;

    if (!fixedcolormap)
    {
        FUINT lightnum;
        sector_t *sector;

        lightnum = LightLevelToLum(*list[i - 1].lightlevel);
        // store Surface->FlatColor to modulate wall texture
        Surf->FlatColor.s.red = Surf->FlatColor.s.green = Surf->FlatColor.s.blue = lightnum;

        sector = list[i - 1].caster ? &sectors[list[i - 1].caster->secnum] : gr_frontsector;
        if (sector->extra_colormap)
        {
            RGBA_t temp;
            int alpha;

            temp.rgba = sector->extra_colormap->rgba;
            alpha = temp.s.alpha;
            Surf->FlatColor.s.red = ((26 - alpha) * lightnum + alpha * temp.s.red) / 26;
            Surf->FlatColor.s.blue = ((26 - alpha) * lightnum + alpha * temp.s.blue) / 26;
            Surf->FlatColor.s.green = ((26 - alpha) * lightnum + alpha * temp.s.green) / 26;
            Surf->FlatColor.s.alpha = 0xff;
        }
    }

    wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
    wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);

    // set top/bottom coords
    wallVerts[2].y = wallVerts[3].y = top;
    wallVerts[0].y = wallVerts[1].y = bot;

    glTex = HWR_GetTexture(texnum);
    if (glTex->mipmap.flags & TF_TRANSPARENT)
        HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Environment);
    else
        HWR_ProjectWall(wallVerts, Surf, PF_Masked);
}

//
// HWR_StoreWallRange
// A portion or all of a wall segment will be drawn, from startfrac to endfrac,
//  where 0 is the start of the segment, 1 the end of the segment
// Anything between means the wall segment has been clipped with solidsegs,
//  reducing wall overdraw to a minimum
//
static void HWR_StoreWallRange(int startfrac, int endfrac)
{
    wallVert3D wallVerts[4];
    v2d_t vs, ve;               // start, end vertices of 2d line (view from above)

    fixed_t worldtop;		// front sector
    fixed_t worldbottom;
    fixed_t worldbacktop = 0;	// back sector, only used on two sided lines
    fixed_t worldbackbottom = 0;

    GlideTexture_t *grTex;
    float cliplow, cliphigh;
    int midtexnum;
    fixed_t h, l;               // 3D sides and 2s middle textures

    FUINT lightnum = 0;         // shut up compiler
    FSurfaceInfo Surf;

    if (startfrac > endfrac)
        return;

    gr_sidedef = gr_curline->sidedef;
    gr_linedef = gr_curline->linedef;

    // mark the segment as visible for auto map
    gr_linedef->flags |= ML_MAPPED;

    worldtop = gr_frontsector->ceilingheight;
    worldbottom = gr_frontsector->floorheight;

    vs.x = ((polyvertex_t *) gr_curline->v1)->x;
    vs.y = ((polyvertex_t *) gr_curline->v1)->y;
    ve.x = ((polyvertex_t *) gr_curline->v2)->x;
    ve.y = ((polyvertex_t *) gr_curline->v2)->y;

    //
    // clip the wall segment to solidsegs
    //

/*  BP : removed since there is no more clipwalls !
    // clip start of segment
    if (startfrac > 0){
        if (startfrac>1)
        {
#ifdef PARANOIA
            CONS_Printf ("startfrac %f\n", startfrac );
#endif
            startfrac = 1;
        }
            vs.x = vs.x + (ve.x - vs.x) * startfrac;
            vs.y = vs.y + (ve.y - vs.y) * startfrac;
        }

    // clip end of segment
    if (endfrac < 1){
        if (endfrac<0)
        {
#ifdef PARANOIA
            CONS_Printf ("  endfrac %f\n", endfrac );
#endif
            endfrac=0;
        }
            ve.x = vs.x + (ve.x - vs.x) * endfrac;
            ve.y = vs.y + (ve.y - vs.y) * endfrac;
        }
*/
    // remember vertices ordering
    //  3--2
    //  | /|
    //  |/ |
    //  0--1
    // make a wall polygon (with 2 triangles), using the floor/ceiling heights,
    // and the 2d map coords of start/end vertices
    wallVerts[0].x = wallVerts[3].x = vs.x;
    wallVerts[0].z = wallVerts[3].z = vs.y;
    wallVerts[2].x = wallVerts[1].x = ve.x;
    wallVerts[2].z = wallVerts[1].z = ve.y;
    wallVerts[0].w = wallVerts[1].w = wallVerts[2].w = wallVerts[3].w = 1.0f;

    if (drawtextured)
    {
        // x offset the texture
        fixed_t texturehpeg = gr_sidedef->textureoffset + gr_curline->offset;

        // clip texture s start/end coords with solidsegs
        if (startfrac > 0 && startfrac < 1)
            cliplow = texturehpeg + gr_curline->length * startfrac;
        else
            cliplow = texturehpeg;

        if (endfrac > 0 && endfrac < 1)
            cliphigh = texturehpeg + gr_curline->length * endfrac;
        else
            cliphigh = texturehpeg + gr_curline->length;
    }

    //  use different light tables
    //  for horizontal / vertical / diagonal
    //  note: try to get the same visual feel as the original
    Surf.FlatColor.s.alpha = 0xff;
    if (fixedcolormap)
    {
        // TODO: better handling of fixedcolormap
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
    }
    else
    {
        //FUINT   lightnum;

        lightnum = LightLevelToLum(gr_frontsector->lightlevel);

        if (((polyvertex_t *) gr_curline->v1)->y == ((polyvertex_t *) gr_curline->v2)->y && lightnum >= (255 / LIGHTLEVELS))
            lightnum -= (255 / LIGHTLEVELS);
        else if (((polyvertex_t *) gr_curline->v1)->x == ((polyvertex_t *) gr_curline->v2)->x && lightnum < 255 - (255 / LIGHTLEVELS))
            lightnum += (255 / LIGHTLEVELS);

        // store Surface->FlatColor to modulate wall texture
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = lightnum;

        //Hurdler: it seems to be better here :)
        if (gr_frontsector)
        {
            sector_t *sector = gr_frontsector;

            //Hurdler: colormap test
            if (sector->ffloors)
            {
                ffloor_t *caster;

                caster = sector->lightlist[R_GetPlaneLight(sector, sector->floorheight, false)].caster;
                sector = caster ? &sectors[caster->secnum] : sector;
            }
            if (sector->extra_colormap)
            {
                RGBA_t temp;
                int alpha;

                temp.rgba = sector->extra_colormap->rgba;
                alpha = temp.s.alpha;
                Surf.FlatColor.s.red = ((26 - alpha) * lightnum + alpha * temp.s.red) / 26;
                Surf.FlatColor.s.blue = ((26 - alpha) * lightnum + alpha * temp.s.blue) / 26;
                Surf.FlatColor.s.green = ((26 - alpha) * lightnum + alpha * temp.s.green) / 26;
                Surf.FlatColor.s.alpha = 0xff;
            }
        }
    }

    if (gr_backsector)
    {
        // two sided line
        worldbacktop = gr_backsector->ceilingheight;
        worldbackbottom = gr_backsector->floorheight;

        // hack to allow height changes in outdoor areas
        if (gr_frontsector->ceilingpic == skyflatnum && gr_backsector->ceilingpic == skyflatnum)
        {
            worldtop = worldbacktop;
        }

        // check TOP TEXTURE
        // texture num are either 0=no-texture, or valid
        int toptexnum = texturetranslation[gr_sidedef->toptexture];
        if (worldbacktop < worldtop && toptexnum)
        {
            if (drawtextured)
            {
                fixed_t texturevpegtop; //top

                grTex = HWR_GetTexture(toptexnum);

                // PEGGING
                if (gr_linedef->flags & ML_DONTPEGTOP)
                    texturevpegtop = 0;
                else
	        {
		    // [WDJ] sometimes maybe textureheight[gr_sidedef->toptexture] != textureheight[toptexnum]
//                    texturevpegtop = worldbacktop + textureheight[gr_sidedef->toptexture] - worldtop;
                    texturevpegtop = worldbacktop + textureheight[toptexnum] - worldtop;
		}

                texturevpegtop += gr_sidedef->rowoffset;

                wallVerts[3].t = wallVerts[2].t = texturevpegtop * grTex->scaleY;
                wallVerts[0].t = wallVerts[1].t = (texturevpegtop + worldtop - worldbacktop) * grTex->scaleY;
                wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
            }

            // set top/bottom coords
            wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( worldtop );
            wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( worldbacktop );

            if (gr_frontsector->numlights)
                HWR_SplitWall(gr_frontsector, wallVerts, toptexnum, &Surf, FF_CUTSOLIDS);
            else if (grTex->mipmap.flags & TF_TRANSPARENT)
                HWR_AddTransparentWall(wallVerts, &Surf, toptexnum, PF_Environment);
            else
                HWR_ProjectWall(wallVerts, &Surf, PF_Masked);
        }

        // check BOTTOM TEXTURE
        // texture num are either 0=no-texture, or valid
        int bottomtexnum = texturetranslation[gr_sidedef->bottomtexture];
        if (worldbackbottom > worldbottom && bottomtexnum)    //only if VISIBLE!!!
        {
            if (drawtextured)
            {
                fixed_t texturevpegbottom = 0;  //bottom

                grTex = HWR_GetTexture(bottomtexnum);

                // PEGGING
                if (gr_linedef->flags & ML_DONTPEGBOTTOM)
                    texturevpegbottom = worldtop - worldbackbottom;
                else
                    texturevpegbottom = 0;

                texturevpegbottom += gr_sidedef->rowoffset;

                wallVerts[3].t = wallVerts[2].t = texturevpegbottom * grTex->scaleY;
                wallVerts[0].t = wallVerts[1].t = (texturevpegbottom + worldbackbottom - worldbottom) * grTex->scaleY;
                wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
            }

            // set top/bottom coords
            wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( worldbackbottom );
            wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( worldbottom );

            if (gr_frontsector->numlights)
                HWR_SplitWall(gr_frontsector, wallVerts, bottomtexnum, &Surf, FF_CUTSOLIDS);
            else if (grTex->mipmap.flags & TF_TRANSPARENT)
                HWR_AddTransparentWall(wallVerts, &Surf, bottomtexnum, PF_Environment);
            else
                HWR_ProjectWall(wallVerts, &Surf, PF_Masked);
        }
        // texture num are either 0=no-texture, or valid
        midtexnum = texturetranslation[gr_sidedef->midtexture];
        if (midtexnum)
        {
            int blendmode;
            fixed_t opentop, openbottom, polytop, polybottom;

            // SoM: a little note: This code re-arranging will
            // fix the bug in Nimrod map02. opentop and openbottom
            // record the limits the texture can be displayed in.
            // polytop and polybottom, are the ideal (i.e. unclipped)
            // heights of the polygon, and h & l, are the final (clipped)
            // poly coords.

            opentop = (worldtop < worldbacktop) ? worldtop : worldbacktop;
            openbottom = (worldbottom > worldbackbottom) ? worldbottom : worldbackbottom;

            if (gr_linedef->flags & ML_DONTPEGBOTTOM)
            {
                polybottom = openbottom + gr_sidedef->rowoffset;
                polytop = polybottom + textureheight[midtexnum];
            }
            else
            {
                polytop = opentop + gr_sidedef->rowoffset;
                polybottom = polytop - textureheight[midtexnum];
            }
            if ((gr_frontsector->ceilingheight == gr_backsector->ceilingheight) || (gr_linedef->flags & ML_DONTDRAW))
                h = polytop;
            else
                h = polytop < opentop ? polytop : opentop;

            if ((gr_frontsector->floorheight == gr_backsector->floorheight) || (gr_linedef->flags & ML_DONTDRAW))
                l = polybottom;
            else
                l = polybottom > openbottom ? polybottom : openbottom;

            if (drawtextured)
            {
                fixed_t texturevpeg;
                // PEGGING
                if (gr_linedef->flags & ML_DONTPEGBOTTOM)
                    texturevpeg = l + textureheight[gr_sidedef->midtexture] - h + polybottom - l;
                else
                    texturevpeg = polytop - h;

                grTex = HWR_GetTexture(midtexnum);

                wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
                wallVerts[0].t = wallVerts[1].t = (h - l + texturevpeg) * grTex->scaleY;
                wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
            }
            // set top/bottom coords
            wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( h );
            wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( l );

            // set alpha for transparent walls (new boom and legacy linedef types)
            // ooops ! this do not work at all because render order we should render it in backtofront order
            switch (gr_linedef->special)
            {
                case 260:  // Boom make translucent
	                   // Legacy translucent  284 to 288
                case 284:  // Legacy translucent, brighten (greenish)
                    blendmode = HWR_TranstableToAlpha(tr_transmed, &Surf);
                    break;
                case 285:  // Legacy translucent, brighten (less greenish)
                    blendmode = HWR_TranstableToAlpha(tr_transmor, &Surf);
                    break;
                case 286:  // Legacy translucent, darkens
                    blendmode = HWR_TranstableToAlpha(tr_transhi, &Surf);
                    break;
                case 287:  // Legacy translucent, brightens
                    blendmode = HWR_TranstableToAlpha(tr_transfir, &Surf);
                    break;
                case 288:  // Legacy selective translucent, on selected colors
	            //FIXME: not work like this must be loaded with firetranslucent to true !
                    blendmode = HWR_TranstableToAlpha(tr_transfx1, &Surf);
                    break;
                case 283:  // Legacy fog sheet
                    blendmode = PF_Substractive;
                    break;
                default:
                    blendmode = PF_Masked;
                    break;
            }
            if (grTex->mipmap.flags & TF_TRANSPARENT)
                blendmode = PF_Environment;

            if (blendmode != PF_Masked)
                HWR_AddTransparentWall(wallVerts, &Surf, midtexnum, blendmode);
            else
                HWR_ProjectWall(wallVerts, &Surf, blendmode);
        }
    }
    else
    {
        // Single sided line... Deal only with the middletexture (if one exists)
        midtexnum = texturetranslation[gr_sidedef->midtexture];
        if (midtexnum)
        {
            if (drawtextured)
            {
                fixed_t texturevpeg;
                // PEGGING
                if ((unsigned short)gr_linedef->flags & ML_DONTPEGBOTTOM)
                    texturevpeg = worldbottom + textureheight[gr_sidedef->midtexture] - worldtop + gr_sidedef->rowoffset;
                else
                    // top of texture at top
                    texturevpeg = gr_sidedef->rowoffset;

                grTex = HWR_GetTexture(midtexnum);

                wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
                wallVerts[0].t = wallVerts[1].t = (texturevpeg + worldtop - worldbottom) * grTex->scaleY;
                wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
            }
            // set top/bottom coords
            wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( worldtop );
            wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( worldbottom );

            // I don't think that solid walls can use translucent linedef types...
            if (gr_frontsector->numlights)
                HWR_SplitWall(gr_frontsector, wallVerts, midtexnum, &Surf, FF_CUTSOLIDS);
            else
            {
                if (grTex->mipmap.flags & TF_TRANSPARENT)
                    HWR_AddTransparentWall(wallVerts, &Surf, midtexnum, PF_Environment);
                else
                    HWR_ProjectWall(wallVerts, &Surf, PF_Masked);
            }
        }
    }

    //Hurdler: 3d-floors test
#ifdef R_FAKEFLOORS
    if (gr_frontsector && gr_backsector && gr_frontsector->tag != gr_backsector->tag && (gr_backsector->ffloors || gr_frontsector->ffloors))
    {
        ffloor_t *rover;
        fixed_t highcut, lowcut;

        highcut = gr_frontsector->ceilingheight < gr_backsector->ceilingheight ? gr_frontsector->ceilingheight : gr_backsector->ceilingheight;
        lowcut = gr_frontsector->floorheight > gr_backsector->floorheight ? gr_frontsector->floorheight : gr_backsector->floorheight;

        if (gr_backsector->ffloors)
        {
            for (rover = gr_backsector->ffloors; rover; rover = rover->next)
            {
                if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || (rover->flags & FF_INVERTSIDES))
                    continue;
                if (*rover->topheight < lowcut || *rover->bottomheight > highcut)
                    continue;

                h = *rover->topheight;
                l = *rover->bottomheight;
                if (h > highcut)
                    h = highcut;
                if (l < lowcut)
                    l = lowcut;
                //Hurdler: HW code starts here
                //FIXME: check if peging is correct
                // set top/bottom coords
                wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( h );
                wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( l );

                if (drawtextured)
                {
                    grTex = HWR_GetTexture(texturetranslation[sides[rover->master->sidenum[0]].midtexture]);

                    wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h) * grTex->scaleY;
                    wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h)) * grTex->scaleY;
                    wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                    wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
                }
                if (rover->master->special != 302) // NOT Legacy 3D fog in tagged sector
                {
                    int blendmode = PF_Masked;

                    if (rover->flags & FF_TRANSLUCENT)
                    {
                        blendmode = PF_Translucent;
                        if (cv_grtranswall.value)
                            Surf.FlatColor.s.alpha = rover->alpha;
                    }
                    else if (grTex->mipmap.flags & TF_TRANSPARENT)
                    {
                        blendmode = PF_Environment;
                    }

                    if (gr_frontsector->numlights)
                        HWR_SplitWall(gr_frontsector, wallVerts, texturetranslation[sides[rover->master->sidenum[0]].midtexture], &Surf, rover->flags & FF_EXTRA ? FF_CUTEXTRA : FF_CUTSOLIDS);
                    else
                    {
                        if (blendmode != PF_Masked)
                            HWR_AddTransparentWall(wallVerts, &Surf, texturetranslation[sides[rover->master->sidenum[0]].midtexture], blendmode);
                        else
                            HWR_ProjectWall(wallVerts, &Surf, PF_Masked);
                    }
                }
            }
        }
        else if (gr_frontsector->ffloors)
        {
            for (rover = gr_frontsector->ffloors; rover; rover = rover->next)
            {
                if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || !(rover->flags & FF_ALLSIDES))
                    continue;
                if (*rover->topheight < lowcut || *rover->bottomheight > highcut)
                    continue;

                h = *rover->topheight;
                l = *rover->bottomheight;
                if (h > highcut)
                    h = highcut;
                if (l < lowcut)
                    l = lowcut;
                //Hurdler: HW code starts here
                //FIXME: check if peging is correct
                // set top/bottom coords
                wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT( h );
                wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT( l );

                if (drawtextured)
                {
                    grTex = HWR_GetTexture(texturetranslation[sides[rover->master->sidenum[0]].midtexture]);

                    wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h) * grTex->scaleY;
                    wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h)) * grTex->scaleY;
                    wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
                    wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
                }
                if (rover->master->special != 302) // NOT Legacy 3D fog in tagged sector
                {
                    int blendmode = PF_Masked;

                    if (rover->flags & FF_TRANSLUCENT)
                    {
                        blendmode = PF_Translucent;
                        if (cv_grtranswall.value)
                            Surf.FlatColor.s.alpha = rover->alpha;
                    }
                    else if (grTex->mipmap.flags & TF_TRANSPARENT)
                    {
                        blendmode = PF_Environment;
                    }

                    if (gr_backsector->numlights)
                        HWR_SplitWall(gr_backsector, wallVerts, texturetranslation[sides[rover->master->sidenum[0]].midtexture], &Surf, rover->flags & FF_EXTRA ? FF_CUTEXTRA : FF_CUTSOLIDS);
                    else
                    {
                        if (blendmode != PF_Masked)
                            HWR_AddTransparentWall(wallVerts, &Surf, texturetranslation[sides[rover->master->sidenum[0]].midtexture], blendmode);
                        else
                            HWR_ProjectWall(wallVerts, &Surf, PF_Masked);
                    }
                }
            }
        }
    }
#endif
//Hurdler: end of 3d-floors test
}

//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct
{
    int first;
    int last;

} cliprange_t;

//Hurdler: just like in r_bsp.c
#define MAXSEGS         MAXVIDWIDTH/2+1

// newend is one past the last valid seg
cliprange_t *newend;
cliprange_t gr_solidsegs[MAXSEGS];

void printsolidsegs(void)
{
    cliprange_t *start;
    if (!newend || cv_grbeta.value != 2)
        return;
    for (start = gr_solidsegs; start != newend; start++)
        CONS_Printf("%d-%d|", start->first, start->last);
    CONS_Printf("\n\n");
}

//
//
//
static void HWR_ClipSolidWallSegment(int first, int last)
{
    cliprange_t *next;
    cliprange_t *start;
    float lowfrac, highfrac;
    boolean clipwalls_fragment = false;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first - 1)
        start++;

    if (first < start->first)
    {
        if (last < start->first - 1)
        {
            // Post is entirely visible (above start),
            //  so insert a new clippost.
            HWR_StoreWallRange(first, last);

            next = newend;
            newend++;

            while (next != start)
            {
                *next = *(next - 1);
                next--;
            }
            next->first = first;
            next->last = last;
            printsolidsegs();
            return;
        }

        // There is a fragment above *start.
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(first, last);
            clipwalls_fragment = true;
        }
        else
        {
            highfrac = HWR_ClipViewSegment(start->first + 1, (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(0, highfrac);
        }
        // Now adjust the clip size.
        start->first = first;
    }

    // Bottom contained in start?
    if (last <= start->last)
    {
        printsolidsegs();
        return;
    }
    next = start;
    while (last >= (next + 1)->first - 1)
    {
        // There is a fragment between two posts.
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(first, last);
            clipwalls_fragment = true;
        }
        else
        {
            lowfrac = HWR_ClipViewSegment(next->last - 1, (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            highfrac = HWR_ClipViewSegment((next + 1)->first + 1, (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(lowfrac, highfrac);
        }
        next++;

        if (last <= next->last)
        {
            // Bottom is contained in next.
            // Adjust the clip size.
            start->last = next->last;
            goto crunch;
        }
    }

    if (first == next->first + 1)       // 1 line texture
    {
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(first, last);
            clipwalls_fragment = true;
        }
        else
            HWR_StoreWallRange(0, 1);
    }
    else
    {
        // There is a fragment after *next.
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(first, last);
            clipwalls_fragment = true;
        }
        else
        {
            lowfrac = HWR_ClipViewSegment(next->last - 1, (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(lowfrac, 1);
        }
    }

    // Adjust the clip size.
    start->last = last;

    // Remove start+1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
        printsolidsegs();
        // Post just extended past the bottom of one post.
        return;
    }

    while (next++ != newend)
    {
        // Remove a post.
        *++start = *next;
    }

    newend = start;
    printsolidsegs();
}

//
//  handle LineDefs with upper and lower texture (windows)
//
static void HWR_ClipPassWallSegment(int first, int last)
{
    cliprange_t *start;
    float lowfrac, highfrac;
    //to allow noclipwalls but still solidseg reject of non-visible walls
    boolean clipwalls_fragment = false;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first - 1)
        start++;

    if (first < start->first)
    {
        if (last < start->first - 1)
        {
            // Post is entirely visible (above start).
            HWR_StoreWallRange(0, 1);
            return;
        }

        // There is a fragment above *start.
        if (!cv_grclipwalls.value)
        {
            //20/08/99: Changed by Hurdler (taken from faB's code)
            if (!clipwalls_fragment)
                HWR_StoreWallRange(0, 1);
            clipwalls_fragment = true;
        }
        else
        {
            highfrac = HWR_ClipViewSegment(min(start->first + 1, start->last), (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(0, highfrac);
        }
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    while (last >= (start + 1)->first - 1)
    {
        // There is a fragment between two posts.
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(0, 1);
            clipwalls_fragment = true;
        }
        else
        {
            lowfrac = HWR_ClipViewSegment(max(start->last - 1, start->first), (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            highfrac = HWR_ClipViewSegment(min((start + 1)->first + 1, (start + 1)->last), (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(lowfrac, highfrac);
        }
        start++;

        if (last <= start->last)
            return;
    }

    if (first == start->first + 1)      // 1 line texture
    {
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(0, 1);
            clipwalls_fragment = true;
        }
        else
            HWR_StoreWallRange(0, 1);
    }
    else
    {
        // There is a fragment after *next.
        if (!cv_grclipwalls.value)
        {
            if (!clipwalls_fragment)
                HWR_StoreWallRange(0, 1);
            clipwalls_fragment = true;
        }
        else
        {
            lowfrac = HWR_ClipViewSegment(max(start->last - 1, start->first), (polyvertex_t *) gr_curline->v1, (polyvertex_t *) gr_curline->v2);
            HWR_StoreWallRange(lowfrac, 1);
        }
    }
}

// --------------------------------------------------------------------------
//  HWR_ClipToSolidSegs check if it is hide by wall (solidsegs)
// --------------------------------------------------------------------------
static boolean HWR_ClipToSolidSegs(int first, int last)
{
    cliprange_t *start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first - 1)
        start++;

    if (first < start->first)
        return true;

    // Bottom contained in start?
    if (last <= start->last)
        return false;

    return true;
}

//
// HWR_ClearClipSegs
//
static void HWR_ClearClipSegs(void)
{
    gr_solidsegs[0].first = -0x7fffffff;
    gr_solidsegs[0].last = -1;
    gr_solidsegs[1].first = vid.width;  //viewwidth;
    gr_solidsegs[1].last = 0x7fffffff;
    newend = gr_solidsegs + 2;
}

// -----------------+
// HWR_AddLine      : Clips the given segment and adds any visible pieces to the line list.
// Notes            : gr_cursectorlight is set to the current subsector -> sector -> light value
//                  : ( it may be mixed with the wall's own flat colour in the future ... )
// -----------------+
static void HWR_AddLine(seg_t * line)
{
    int x1;
    int x2;
    angle_t angle1;
    angle_t angle2;
    angle_t span;
    angle_t tspan;

    // SoM: Backsector needs to be run through R_FakeFlat
    sector_t tempsec;

    gr_curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle(((polyvertex_t *) line->v1)->x * FRACUNIT, ((polyvertex_t *) line->v1)->y * FRACUNIT);
    angle2 = R_PointToAngle(((polyvertex_t *) line->v2)->x * FRACUNIT, ((polyvertex_t *) line->v2)->y * FRACUNIT);

    // Clip to view edges.
    span = angle1 - angle2;

    // backface culling : span is < ANG180 if ang1 > ang2 : the seg is facing
    if (span >= ANG180)
        return;

    // Global angle needed by segcalc.
    //rw_angle1 = angle1;
    angle1 -= dup_viewangle;
    angle2 -= dup_viewangle;

    tspan = angle1 + gr_clipangle;
    if (tspan > 2 * gr_clipangle)
    {
        tspan -= 2 * gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;

        angle1 = gr_clipangle;
    }
    tspan = gr_clipangle - angle2;
    if (tspan > 2 * gr_clipangle)
    {
        tspan -= 2 * gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = -gr_clipangle;
    }

#if 0
    {
        float fx1, fx2, fy1, fy2;
        //BP: test with a better projection than viewangletox[R_PointToAngle(angle)]
        // do not enable this at release 4 mul and 2 div
        fx1 = ((polyvertex_t *) (line->v1))->x - gr_viewx;
        fy1 = ((polyvertex_t *) (line->v1))->y - gr_viewy;
        fy2 = (fx1 * gr_viewcos + fy1 * gr_viewsin);
        if (fy2 < 0)
            // the point is back
            fx1 = 0;
        else
            fx1 = gr_windowcenterx + (fx1 * gr_viewsin - fy1 * gr_viewcos) * gr_centerx / fy2;

        fx2 = ((polyvertex_t *) (line->v2))->x - gr_viewx;
        fy2 = ((polyvertex_t *) (line->v2))->y - gr_viewy;
        fy1 = (fx2 * gr_viewcos + fy2 * gr_viewsin);
        if (fy1 < 0)
            // the point is back
            fx2 = vid.width;
        else
            fx2 = gr_windowcenterx + (fx2 * gr_viewsin - fy2 * gr_viewcos) * gr_centerx / fy1;

        x1 = fx1 + 0.5;
        x2 = fx2 + 0.5;
    }
#else
    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;

    x1 = gr_viewangletox[angle1];
    x2 = gr_viewangletox[angle2];
#endif
    // Does not cross a pixel?
//    if (x1 == x2)
/*    {
        // BP: HERE IS THE MAIN PROBLEM !
        //CONS_Printf("tineline\n");
        return;
    }
*/
    gr_backsector = line->backsector;

    // Single sided line?
    if (!gr_backsector)
        goto clipsolid;

    gr_backsector = R_FakeFlat(gr_backsector, &tempsec, NULL, NULL, true);
   
#ifdef DOORCLOSED_FIX
    // [WDJ] Improvement on door closed detection from r_bsp
    doorclosed = 0; //SoM: 3/25/2000
#endif

    // Closed door.
    if (gr_backsector->ceilingheight <= gr_frontsector->floorheight
	|| gr_backsector->floorheight >= gr_frontsector->ceilingheight)
        goto clipsolid;

#ifdef DOORCLOSED_FIX
    //SoM: 3/25/2000: Check for automap fix. Store in doorclosed for r_segs.c
    if ((doorclosed = R_DoorClosed()))
      goto clipsolid;
#endif

    // Window.
    if (gr_backsector->ceilingheight != gr_frontsector->ceilingheight
	|| gr_backsector->floorheight != gr_frontsector->floorheight)
        goto clippass;

#if 1
    // [WDJ] 4/20/2010 From software renderer, to get improvements
    // Reject empty lines used for triggers and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides, and no middle texture.
    if (gr_backsector->ceilingpic == gr_frontsector->ceilingpic
        && gr_backsector->floorpic == gr_frontsector->floorpic
        && gr_backsector->lightlevel == gr_frontsector->lightlevel
        && gr_curline->sidedef->midtexture == 0

        //SoM: 3/22/2000: Check offsets too!
        && gr_backsector->floor_xoffs == gr_frontsector->floor_xoffs
        && gr_backsector->floor_yoffs == gr_frontsector->floor_yoffs
        && gr_backsector->ceiling_xoffs == gr_frontsector->ceiling_xoffs
        && gr_backsector->ceiling_yoffs == gr_frontsector->ceiling_yoffs

        //SoM: 3/17/2000: consider altered lighting
        && gr_backsector->floorlightsec == gr_frontsector->floorlightsec
        && gr_backsector->ceilinglightsec == gr_frontsector->ceilinglightsec
        //SoM: 4/3/2000: Consider colormaps
        && gr_backsector->extra_colormap == gr_frontsector->extra_colormap
        && ((!gr_frontsector->ffloors && !gr_backsector->ffloors) ||
           (gr_frontsector->tag == gr_backsector->tag)))
    {
        return;
    }
#else
    // Reject empty lines used for triggers and special events.
    // Identical floor and ceiling on both sides,
    //  identical light levels on both sides,
    //  and no middle texture.
    if (gr_backsector->ceilingpic == gr_frontsector->ceilingpic
	&& gr_backsector->floorpic == gr_frontsector->floorpic
	&& gr_backsector->lightlevel == gr_frontsector->lightlevel
        && gr_curline->sidedef->midtexture == 0
	&& !gr_backsector->ffloors
	&& !gr_frontsector->ffloors)
        // SoM: For 3D sides... Boris, would you like to take a
        // crack at rendering 3D sides? You would need to add the
        // above check and add code to HWR_StoreWallRange...
    {
        return;
    }
#endif

  clippass:
    if (x1 == x2)
    {
        x2++;
        x1 -= 2;
    }
    HWR_ClipPassWallSegment(x1, x2 - 1);
    return;

  clipsolid:
    if (x1 == x2)
        goto clippass;
    HWR_ClipSolidWallSegment(x1, x2 - 1);
}

// HWR_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
// modified to use local variables

extern int checkcoord[12][4];   //r_bsp.c

static boolean HWR_CheckBBox(fixed_t * bspcoord)
{
    int boxpos;

    fixed_t x1;
    fixed_t y1;
    fixed_t x2;
    fixed_t y2;

    angle_t angle1;
    angle_t angle2;
    angle_t span;
    angle_t tspan;

    int sx1;
    int sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (dup_viewx <= bspcoord[BOXLEFT])
        boxpos = 0;
    else if (dup_viewx < bspcoord[BOXRIGHT])
        boxpos = 1;
    else
        boxpos = 2;

    if (dup_viewy >= bspcoord[BOXTOP])
        boxpos |= 0;
    else if (dup_viewy > bspcoord[BOXBOTTOM])
        boxpos |= 1 << 2;
    else
        boxpos |= 2 << 2;

    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    // check clip list for an open space
    angle1 = R_PointToAngle(x1, y1) - dup_viewangle;
    angle2 = R_PointToAngle(x2, y2) - dup_viewangle;

    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
        return true;

    tspan = angle1 + gr_clipangle;

    if (tspan > 2 * gr_clipangle)
    {
        tspan -= 2 * gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle1 = gr_clipangle;
    }
    tspan = gr_clipangle - angle2;
    if (tspan > 2 * gr_clipangle)
    {
        tspan -= 2 * gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle2 = -gr_clipangle;
    }

    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
    sx1 = gr_viewangletox[angle1];
    sx2 = gr_viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;

    return HWR_ClipToSolidSegs(sx1, sx2 - 1);
}

sector_t *R_FakeFlat(sector_t *, sector_t *, int *, int *, boolean);

// -----------------+
// HWR_Subsector    : Determine floor/ceiling planes.
//                  : Add sprites of things in sector.
//                  : Draw one or more line segments.
// Notes            : Sets gr_cursectorlight to the light of the parent sector, to modulate wall textures
// -----------------+
static int doomwaterflat;       //set by R_InitFlats hack
// Called from HWR_RenderBSPNode
static void HWR_Subsector(int num)
{
    int count;
    seg_t *line;
    subsector_t *sub;
    sector_t tempsec;           //SoM: 4/7/2000
    int floorlightlevel;
    int ceilinglightlevel;
    int locFloorHeight, locCeilingHeight;
    int light;
    fixed_t wh;

//no risk while developing, enough debugging nights!
#ifdef PARANOIA
    if (num >= addsubsector)
        I_Error("HWR_Subsector: ss %i with numss = %i, addss = %d", num, numsubsectors, addsubsector);

    /*
       if (num>=numsubsectors)
       I_Error ("HWR_Subsector: ss %i with numss = %i",
       num,
       numsubsectors );
     */
#endif

    if (num < numsubsectors)
    {
        sscount++;
        // subsector
        sub = &subsectors[num];
        // sector
        gr_frontsector = sub->sector;
        // how many linedefs
        count = sub->numlines;
        // first line seg
        line = &segs[sub->firstline];
    }
    else
    {
        // there are no segs but only planes
        sub = &subsectors[0];
        gr_frontsector = sub->sector;
        count = 0;
        line = NULL;
    }

    //SoM: 4/7/2000: Test to make Boom water work in Hardware mode.
    gr_frontsector = R_FakeFlat(gr_frontsector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);
    //FIXME: Use floorlightlevel and ceilinglightlevel insted of lightlevel.

    // ------------------------------------------------------------------------
    // sector lighting, DISABLED because it's done in HWR_StoreWallRange
    // ------------------------------------------------------------------------
    // TODO : store a RGBA instead of just intensity, allow coloured sector lighting
    //light = (FUBYTE)(sub->sector->lightlevel & 0xFF) / 255.0f;
    //gr_cursectorlight.red   = light;
    //gr_cursectorlight.green = light;
    //gr_cursectorlight.blue  = light;
    //gr_cursectorlight.alpha = light;

// ----- for special tricks with HW renderer -----
    if (gr_frontsector->pseudoSector)
    {
        locFloorHeight = gr_frontsector->virtualFloorheight;
        locCeilingHeight = gr_frontsector->virtualCeilingheight;
    }
    else if (gr_frontsector->virtualFloor)
    {
        locFloorHeight = gr_frontsector->virtualFloorheight;
        if (gr_frontsector->virtualCeiling)
            locCeilingHeight = gr_frontsector->virtualCeilingheight;
        else
            locCeilingHeight = gr_frontsector->ceilingheight;
    }
    else if (gr_frontsector->virtualCeiling)
    {
        locCeilingHeight = gr_frontsector->virtualCeilingheight;
        locFloorHeight = gr_frontsector->floorheight;
    }
    else
    {
        locFloorHeight = gr_frontsector->floorheight;
        locCeilingHeight = gr_frontsector->ceilingheight;
    }
// ----- end special tricks -----

    if (gr_frontsector->ffloors)
    {
        if (gr_frontsector->moved)
        {
            gr_frontsector->numlights = sub->sector->numlights = 0;
            R_Prep3DFloors(gr_frontsector);
            sub->sector->lightlist = gr_frontsector->lightlist;
            sub->sector->numlights = gr_frontsector->numlights;
            sub->sector->moved = gr_frontsector->moved = false;
        }

#if 1
        // [WDJ] from r_bsp.c 4/22/2010
	// adapted to local vars, and may still need a little tuning
//        light = R_GetPlaneLight(gr_frontsector, gr_frontsector->floorheight, false);
        light = R_GetPlaneLight(gr_frontsector, locFloorHeight, false);
        if(gr_frontsector->floorlightsec == -1)
        {
	  floorlightlevel = *gr_frontsector->lightlist[light].lightlevel;
//        floorcolormap = gr_frontsector->lightlist[light].extra_colormap;
        }
//        light = R_GetPlaneLight(gr_frontsector, gr_frontsector->ceilingheight);
        light = R_GetPlaneLight(gr_frontsector, locCeilingHeight);
        if(gr_frontsector->ceilinglightsec == -1)
        {
	  ceilinglightlevel = *gr_frontsector->lightlist[light].lightlevel;
//        ceilingcolormap = gr_frontsector->lightlist[light].extra_colormap;
	}
#else
        floorlightlevel = *gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, locFloorHeight, false)].lightlevel;
        ceilinglightlevel = *gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, locCeilingHeight, false)].lightlevel;
#endif
    }

    // render floor ?
#ifdef DOPLANES
    // yeah, easy backface cull! :)
    if (locFloorHeight < dup_viewz)
    {
        if (gr_frontsector->floorpic != skyflatnum)
        {
            if (sub->validcount != validcount)
            {
                HWR_GetFlat(levelflats[gr_frontsector->floorpic].lumpnum);
                HWR_RenderPlane(&extrasubsectors[num], locFloorHeight, PF_Occlude,
				floorlightlevel, levelflats[gr_frontsector->floorpic].lumpnum);
            }
        }
        else
        {
#ifdef POLYSKY
            HWR_RenderSkyPlane(&extrasubsectors[num], locFloorHeight);
#endif
            cv_grsky.value = true;
        }
    }

    if (locCeilingHeight > dup_viewz)
    {
        if (gr_frontsector->ceilingpic != skyflatnum)
        {
            if (sub->validcount != validcount)
            {
                HWR_GetFlat(levelflats[gr_frontsector->ceilingpic].lumpnum);
                HWR_RenderPlane(&extrasubsectors[num], locCeilingHeight, PF_Occlude,
				ceilinglightlevel, levelflats[gr_frontsector->ceilingpic].lumpnum);
            }
        }
        else
        {
#ifdef POLYSKY
            HWR_RenderSkyPlane(&extrasubsectors[num], locCeilingHeight);
#endif
            cv_grsky.value = true;
        }
    }

#ifdef R_FAKEFLOORS
    if (gr_frontsector->ffloors)
    {
        // TODO:fix light, xoffs, yoffs, extracolormap ?
        ffloor_t *rover;

        R_Prep3DFloors(gr_frontsector);
        for (rover = gr_frontsector->ffloors; rover; rover = rover->next)
        {

            if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
                continue;
            if (sub->validcount == validcount)
                continue;

            if (*rover->bottomheight <= gr_frontsector->ceilingheight
		&& *rover->bottomheight >= gr_frontsector->floorheight
//                && ((dup_viewz < *rover->bottomheight && !(rover->flags & FF_INVERTPLANES))
                && ((dup_viewz <= *rover->bottomheight && !(rover->flags & FF_INVERTPLANES))
		    || (dup_viewz > *rover->bottomheight && (rover->flags & FF_BOTHPLANES))))
            {
                if (rover->flags & (FF_TRANSLUCENT | FF_FOG))   // SoM: Flags are more efficient
                {
                    light = R_GetPlaneLight(gr_frontsector, *rover->bottomheight, dup_viewz < *rover->bottomheight ? true : false);
                    HWR_Add3DWater(levelflats[*rover->bottompic].lumpnum, &extrasubsectors[num],
				   *rover->bottomheight, *gr_frontsector->lightlist[light].lightlevel, rover->alpha);
                }
                else
                {
                    HWR_GetFlat(levelflats[*rover->bottompic].lumpnum);
                    light = R_GetPlaneLight(gr_frontsector, *rover->bottomheight, dup_viewz < *rover->bottomheight ? true : false);
                    HWR_RenderPlane(&extrasubsectors[num], *rover->bottomheight, PF_Occlude,
				    *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->bottompic].lumpnum);
                }
            }
            if (*rover->topheight >= gr_frontsector->floorheight
		&& *rover->topheight <= gr_frontsector->ceilingheight
//                && ((dup_viewz > *rover->topheight && !(rover->flags & FF_INVERTPLANES))
                && ((dup_viewz >= *rover->topheight && !(rover->flags & FF_INVERTPLANES))
		    || (dup_viewz < *rover->topheight && (rover->flags & FF_BOTHPLANES))))
            {
                if (rover->flags & (FF_TRANSLUCENT | FF_FOG))
                {
                    light = R_GetPlaneLight(gr_frontsector, *rover->topheight, dup_viewz < *rover->topheight ? true : false);
                    HWR_Add3DWater(levelflats[*rover->toppic].lumpnum, &extrasubsectors[num],
				   *rover->topheight, *gr_frontsector->lightlist[light].lightlevel, rover->alpha);
                }
                else
                {
                    HWR_GetFlat(levelflats[*rover->toppic].lumpnum);
                    light = R_GetPlaneLight(gr_frontsector, *rover->topheight, dup_viewz < *rover->topheight ? true : false);
                    HWR_RenderPlane(&extrasubsectors[num], *rover->topheight, PF_Occlude,
				    *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->toppic].lumpnum);
                }
            }

        }
    }
#endif
#endif //doplanes

// Hurder ici se passe les choses intéressantes!
// on vient de tracer le sol et le plafond
// on trace à présent d'abord les sprites et ensuite les murs
// hurdler: faux: on ajoute seulement les sprites, le murs sont tracés d'abord
    if (line != NULL)
    {
        // draw sprites first , coz they are clipped to the solidsegs of
        // subsectors more 'in front'
#ifdef SPRITE_LIGHTLEVEL
	HWR_AddSprites(gr_frontsector, tempsec.lightlevel);  // ???
//	HWR_AddSprites(gr_frontsector, gr_frontsector->lightlevel);  // ???
#else
        HWR_AddSprites(gr_frontsector);
#endif

        //Hurdler: at this point validcount must be the same, but is not because
        //         gr_frontsector doesn't point anymore to sub->sector due to
        //         the call gr_frontsector = R_FakeFlat(...)
        //         if it's not done, the sprite is drawn more than once,
        //         what looks really bad with translucency or dynamic light,
        //         without talking about the overdraw of course.
        sub->sector->validcount = validcount;   //TODO: fix that in a better way

        while (count--)
        {
            HWR_AddLine(line);
            line++;
        }
    }

//20/08/99: Changed by Hurdler (taken from faB's code)
#ifdef DOPLANES
    // -------------------- WATER IN DEV. TEST ------------------------
    //dck hack : use abs(tag) for waterheight
    if (gr_frontsector->tag < 0)
    {
        wh = ((-gr_frontsector->tag) << 16) + (FRACUNIT / 2);
        if (wh > gr_frontsector->floorheight && wh < gr_frontsector->ceilingheight)
        {
            HWR_GetFlat(doomwaterflat);
            HWR_RenderPlane(&extrasubsectors[num], wh, PF_Translucent, gr_frontsector->lightlevel, doomwaterflat);
        }
    }
    // -------------------- WATER IN DEV. TEST ------------------------
#endif
    sub->validcount = validcount;
}

//
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.

#ifdef coolhack
//t;b;l;r
static fixed_t hackbbox[4];
//BOXTOP,
//BOXBOTTOM,
//BOXLEFT,
//BOXRIGHT
static boolean HWR_CheckHackBBox(fixed_t * bb)
{
    if (bb[BOXTOP] < hackbbox[BOXBOTTOM])       //y up
        return false;
    if (bb[BOXBOTTOM] > hackbbox[BOXTOP])
        return false;
    if (bb[BOXLEFT] > hackbbox[BOXRIGHT])
        return false;
    if (bb[BOXRIGHT] < hackbbox[BOXLEFT])
        return false;
    return true;
}
#endif

// BP: big hack for a test in lighning ref:1249753487AB
int *bbox;

static void HWR_RenderBSPNode(int bspnum)
{
    node_t *bsp;
    int side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
        {
            //*(gr_drawsubsector_p++) = 0;
            HWR_Subsector(0);
        }
        else
        {
            //*(gr_drawsubsector_p++) = bspnum&(~NF_SUBSECTOR);
            HWR_Subsector(bspnum & (~NF_SUBSECTOR));
        }
        return;
    }

    // not a subsector, a nodes
    bsp = &nodes[bspnum];

    // Decide which side the view point is on.
    side = R_PointOnSide(dup_viewx, dup_viewy, bsp);

    // BP: big hack for a test in lighning ref:1249753487AB
    bbox = bsp->bbox[side];

    // Recursively divide front space.
    HWR_RenderBSPNode(bsp->children[side]);

    // Possibly divide back space.
    if (HWR_CheckBBox(bsp->bbox[side ^ 1]))
    {
        // BP: big hack for a test in lighning ref:1249753487AB
        bbox = bsp->bbox[side ^ 1];
        HWR_RenderBSPNode(bsp->children[side ^ 1]);
    }
}

/*
//
// Clear 'stack' of subsectors to draw
//
static void HWR_ClearDrawSubsectors (void)
{
    gr_drawsubsector_p = gr_drawsubsectors;
}

//
// Draw subsectors pushed on the drawsubsectors 'stack', back to front
//
static void HWR_RenderSubsectors (void)
{
    while (gr_drawsubsector_p > gr_drawsubsectors)
    {
        HWR_RenderBSPNode (
        lastsubsec->nextsubsec = bspnum & (~NF_SUBSECTOR);
    }
}
*/

// ==========================================================================
//                                                              FROM R_MAIN.C
// ==========================================================================

#ifdef NO_MLOOK_EXTENDS_FOV
angle_t fineanglefov = FIELDOFVIEW;
#endif

//BP : exactely the same as R_InitTextureMapping
// Called from r_main:R_ExecuteSetViewSize
void HWR_InitTextureMapping(void)
{
    int i;
    int x;
    int t;
    fixed_t focallength;

    fixed_t grcenterx;
    fixed_t grcenterxfrac;
    int grviewwidth;
#ifdef NO_MLOOK_EXTENDS_FOV
    angle_t clipanglefov;
    static angle_t oldclipanglefov = 0;

    clipanglefov = fineanglefov + 2 * abs((int) aimingangle);
    if (clipanglefov == oldclipanglefov)
        return;
    oldclipanglefov = clipanglefov;
    clipanglefov >>= ANGLETOFINESHIFT;
    if (clipanglefov >= ((angle_t) ANG180 - (angle_t) ANGLE_1) >> ANGLETOFINESHIFT)
        clipanglefov = (ANG180 - ANGLE_1) >> ANGLETOFINESHIFT;

    CONS_Printf("HW_InitTextureMapping() %d %d %d\n", clipanglefov, aimingangle >> ANGLETOFINESHIFT, fineanglefov >> ANGLETOFINESHIFT);
#else
#define clipanglefov (FIELDOFVIEW>>ANGLETOFINESHIFT)
#endif
    grviewwidth = vid.width;
    grcenterx = grviewwidth / 2;
    grcenterxfrac = grcenterx << FRACBITS;

    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    focallength = FixedDiv(grcenterxfrac, finetangent[FINEANGLES / 4 + clipanglefov / 2]);

    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (finetangent[i] > FRACUNIT * 2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT * 2)
            t = grviewwidth + 1;
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (grcenterxfrac - t + FRACUNIT - 1) >> FRACBITS;

            if (t < -1)
                t = -1;
            else if (t > grviewwidth + 1)
                t = grviewwidth + 1;
        }
        gr_viewangletox[i] = t;
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.
    for (x = 0; x <= grviewwidth; x++)
    {
        i = 0;
        while (gr_viewangletox[i] > x)
            i++;
        gr_xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
    }

    // Take out the fencepost cases from viewangletox.
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (gr_viewangletox[i] == -1)
            gr_viewangletox[i] = 0;
        else if (gr_viewangletox[i] == grviewwidth + 1)
            gr_viewangletox[i] = grviewwidth;
    }

    gr_clipangle = gr_xtoviewangle[0];
}

// ==========================================================================
// gr_things.c
// ==========================================================================

// sprites are drawn after all wall and planes are rendered, so that
// sprite translucency effects apply on the rendered view (instead of the background sky!!)

gr_vissprite_t gr_vissprites[MAXVISSPRITES];
gr_vissprite_t *gr_vissprite_p;

// --------------------------------------------------------------------------
// HWR_ClearSprites
// Called at frame start.
// --------------------------------------------------------------------------
static void HWR_ClearSprites(void)
{
    gr_vissprite_p = gr_vissprites;
}

// --------------------------------------------------------------------------
// HWR_NewVisSprite
// --------------------------------------------------------------------------
gr_vissprite_t gr_overflowsprite;

gr_vissprite_t *HWR_NewVisSprite(void)
{
    if (gr_vissprite_p == &gr_vissprites[MAXVISSPRITES])
        return &gr_overflowsprite;

    gr_vissprite_p++;
    return gr_vissprite_p - 1;
}

// -----------------+
// HWR_DrawSprite   : Draw flat sprites
//                  : (monsters, bonuses, weapons, lights, ...)
// Returns          :
// -----------------+
static void HWR_DrawSprite(gr_vissprite_t * spr)
{
#ifndef TERM_TRANSFORM
    int i;
    float tr_x;
    float tr_y;
    FOutVector *wv;
#endif   
    FOutVector wallVerts[4];
    GlidePatch_t *gpatch;       //sprite patch converted to hardware
    FSurfaceInfo Surf;

    // cache sprite graphics
    //12/12/99: Hurdler:
    //          OK, I don't change anything for MD2 support because I want to be
    //          sure to do it the right way. So actually, we keep normal sprite
    //          in memory and we add the md2 model if it exists for that sprite

    // convert sprite differently when fxtranslucent is detected
    if ((spr->mobj->frame & FF_TRANSMASK) == tr_transfx1 << FF_TRANSSHIFT)
    {
        firetranslucent = true;
        gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);
        firetranslucent = false;
    }
    else
        gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);

    HWR_DL_AddLight(spr, gpatch);

    // create the sprite billboard
    //
    //  3--2
    //  | /|
    //  |/ |
    //  0--1

#ifdef TERM_TRANSFORM
#if 1
    // fastest, use transform terms in optimized shared code
    // Combined transforms for look up/down and scaling
    float topty = spr->ty - gpatch->height;
    wallVerts[0].x = wallVerts[3].x = (spr->x1 * sprite_trans_x_to_x);
    wallVerts[1].x = wallVerts[2].x = (spr->x2 * sprite_trans_x_to_x);
    float tranzy = spr->tz * sprite_trans_z_to_y;
    wallVerts[0].y = wallVerts[1].y = (topty * sprite_trans_y_to_y) + tranzy;
    wallVerts[2].y = wallVerts[3].y = (spr->ty * sprite_trans_y_to_y) + tranzy;
    float tranzz = spr->tz * sprite_trans_z_to_z;
    wallVerts[0].z = wallVerts[1].z = (topty * sprite_trans_y_to_z) + tranzz;
    wallVerts[2].z = wallVerts[3].z = (spr->ty * sprite_trans_y_to_z) + tranzz;
#else
    // Use standard transform calls, many redundant calculations
    // Combined transforms for look up/down and scaling
    float topty = spr->ty - gpatch->height;
    HWR_transform_sprite_FOut(spr->x1, topty, spr->tz, &wallVerts[0]);
    HWR_transform_sprite_FOut(spr->x2, topty, spr->tz, &wallVerts[1]);
    HWR_transform_sprite_FOut(spr->x2, spr->ty, spr->tz, &wallVerts[2]);
    HWR_transform_sprite_FOut(spr->x1, spr->ty, spr->tz, &wallVerts[3]);
#endif
#else
    // old serial transform code
    wallVerts[0].x = wallVerts[3].x = spr->x1;
    wallVerts[2].x = wallVerts[1].x = spr->x2;
    wallVerts[2].y = wallVerts[3].y = spr->ty;
    wallVerts[0].y = wallVerts[1].y = spr->ty - gpatch->height;

    // make a wall polygon (with 2 triangles), using the floor/ceiling heights,
    // and the 2d map coords of start/end vertices
    wallVerts[0].z = wallVerts[1].z = wallVerts[2].z = wallVerts[3].z = spr->tz;

    // transform
    wv = wallVerts;

    for (i = 0; i < 4; i++, wv++)
    {
        //look up/down
        tr_x = wv->z;
        tr_y = wv->y;
        wv->y = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin);
        wv->z = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos);

        //scale y before frustum so that frustum can be scaled to screen height
        wv->y *= ORIGINAL_ASPECT * gr_fovlud;
        wv->x *= gr_fovlud;
    }
#endif   

    if (spr->flip)
    {
        wallVerts[0].sow = wallVerts[3].sow = gpatch->max_s;
        wallVerts[2].sow = wallVerts[1].sow = 0;
    }
    else
    {
        wallVerts[0].sow = wallVerts[3].sow = 0;
        wallVerts[2].sow = wallVerts[1].sow = gpatch->max_s;
    }
    wallVerts[3].tow = wallVerts[2].tow = 0;
    wallVerts[0].tow = wallVerts[1].tow = gpatch->max_t;

    // cache the patch in the graphics card memory
    //12/12/99: Hurdler: same comment as above (for md2)
    //Hurdler: 25/04/2000: now support colormap in hardware mode
    HWR_GetMappedPatch(gpatch, spr->colormap);

    // sprite (TODO: coloured-) lighting by modulating the RGB components
    // [WDJ] coloured seems to be done below
    Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = spr->sectorlight;

    //Hurdler: colormap test
    if (!fixedcolormap)
    {
        sector_t *sector = spr->mobj->subsector->sector;

        if (sector->ffloors)
        {
            ffloor_t *caster;

            caster = sector->lightlist[R_GetPlaneLight(sector, spr->mobj->z, false)].caster;
            sector = caster ? &sectors[caster->secnum] : sector;
        }
        if (sector->extra_colormap)
        {
            RGBA_t temp;
            int alpha;

            temp.rgba = sector->extra_colormap->rgba;
            alpha = temp.s.alpha;
            Surf.FlatColor.s.red = ((26 - alpha) * spr->sectorlight + alpha * temp.s.red) / 26;
            Surf.FlatColor.s.blue = ((26 - alpha) * spr->sectorlight + alpha * temp.s.blue) / 26;
            Surf.FlatColor.s.green = ((26 - alpha) * spr->sectorlight + alpha * temp.s.green) / 26;
            Surf.FlatColor.s.alpha = 0xff;
        }
    }

    //TODO: do the test earlier
    if (!cv_grmd2.value || (md2_models[spr->mobj->sprite].scale < 0.0f))
    {
        int blend = 0;
        if (spr->mobj->frame & FF_TRANSMASK)
            blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK) >> FF_TRANSSHIFT, &Surf);
        else if (spr->mobj->frame & FF_SMOKESHADE)
        {
            Surf.FlatColor.s.alpha = 0x80;
            blend = PF_Translucent;
        }
        else if (spr->mobj->flags & MF_SHADOW)
        {
            Surf.FlatColor.s.alpha = 0x40;
            blend = PF_Translucent;
        }
        else
        {
            // BP: i agree that is little better in environement but it don't
            //     work properly under glide nor with fogcolor to ffffff :(
            // Hurdler: PF_Environement would be cool, but we need to fix
            //          the issue with the fog before
            Surf.FlatColor.s.alpha = 0xFF;
            blend = PF_Translucent | PF_Occlude;
        }

        HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend | PF_Modulated | PF_Clip);
    }

    // draw a corona if this sprite contain light(s)
#ifndef NEWCORONAS
    HWR_DoCoronasLighting(wallVerts, spr);
#endif
}

// --------------------------------------------------------------------------
// Sort vissprites by distance
// --------------------------------------------------------------------------
static gr_vissprite_t gr_vsprsortedhead;

static void HWR_SortVisSprites(void)
{
    int i;
    int count;
    gr_vissprite_t *ds;
    gr_vissprite_t *best = NULL;        //shut up compiler
    gr_vissprite_t unsorted;
    float bestdist;

    count = gr_vissprite_p - gr_vissprites;

    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
        return;

    for (ds = gr_vissprites; ds < gr_vissprite_p; ds++)
    {
        ds->next = ds + 1;
        ds->prev = ds - 1;
    }

    gr_vissprites[0].prev = &unsorted;
    unsorted.next = &gr_vissprites[0];
    (gr_vissprite_p - 1)->next = &unsorted;
    unsorted.prev = gr_vissprite_p - 1;

    // pull the vissprites out by scale
    gr_vsprsortedhead.next = gr_vsprsortedhead.prev = &gr_vsprsortedhead;
    for (i = 0; i < count; i++)
    {
        bestdist = ZCLIP_PLANE - 1;
        for (ds = unsorted.next; ds != &unsorted; ds = ds->next)
        {
            if (ds->tz > bestdist)
            {
                bestdist = ds->tz;
                best = ds;
            }
        }
        best->next->prev = best->prev;
        best->prev->next = best->next;
        best->next = &gr_vsprsortedhead;
        best->prev = gr_vsprsortedhead.prev;
        gr_vsprsortedhead.prev->next = best;
        gr_vsprsortedhead.prev = best;
    }
}

// --------------------------------------------------------------------------
//  Draw all vissprites
// --------------------------------------------------------------------------
static void HWR_DrawSprites(void)
{
    if (gr_vissprite_p > gr_vissprites)
    {
        gr_vissprite_t *spr;

        // draw all vissprites back to front
        for (spr = gr_vsprsortedhead.next; spr != &gr_vsprsortedhead; spr = spr->next)
        {
            HWR_DrawSprite(spr);
        }
    }
}

// --------------------------------------------------------------------------
//  Draw all MD2
// --------------------------------------------------------------------------
static void HWR_DrawMD2S(void)
{
    if (gr_vissprite_p > gr_vissprites)
    {
        gr_vissprite_t *spr;

        // draw all MD2 back to front
        for (spr = gr_vsprsortedhead.next; spr != &gr_vsprsortedhead; spr = spr->next)
        {
            HWR_DrawMD2(spr);
        }
    }
}

// --------------------------------------------------------------------------
// HWR_AddSprites
// During BSP traversal, this adds sprites by sector.
// --------------------------------------------------------------------------
static unsigned char sectorlight;
// Does not need separate lightlevel as long as it is called with frontsector.
#ifdef SPRITE_LIGHTLEVEL
static void HWR_AddSprites(sector_t * sec, int sprite_lightlevel)
#else
static void HWR_AddSprites(sector_t * sec)
#endif
{
    mobj_t *thing;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

#if 1
    // sprite lighting
    if(!sec->numlights) // when numlights then light in DrawSprite
    {
#ifdef SPRITE_LIGHTLEVEL
      int lightlevel = sprite_lightlevel;
      if(sec->model < SM_fluid)   lightlevel = sec->lightlevel;
#else      
      // from frontsector is correct for all models
      int lightlevel = sec->lightlevel;
#endif       
      sectorlight = LightLevelToLum(lightlevel); // add extra light
    }
#else
    // old sprite lighting
    // [WDJ] The "& 0xff" ought to cause lighting errors
    sectorlight = LightLevelToLum(sec->lightlevel & 0xff);
#endif

    // Handle all things in sector.
    for (thing = sec->thinglist; thing; thing = thing->snext)
        if ((thing->flags2 & MF2_DONTDRAW) == 0)
            HWR_ProjectSprite(thing);
}

// --------------------------------------------------------------------------
// HWR_ProjectSprite
//  Generates a vissprite for a thing if it might be visible.
// --------------------------------------------------------------------------
// BP why not use xtoviewangle/viewangletox like in bsp ?....
static void HWR_ProjectSprite(mobj_t * thing)
{
    gr_vissprite_t *vis;

    float tr_x;
    float tr_y;

    float tx;
    float tz;

    float x1;
    float x2;

    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int lump;
    unsigned rot;
    boolean flip;
    angle_t ang;

    // transform the origin point
    tr_x = FIXED_TO_FLOAT( thing->x ) - gr_viewx;
    tr_y = FIXED_TO_FLOAT( thing->y ) - gr_viewy;

    // rotation around vertical axis
    tz = (tr_x * gr_viewcos) + (tr_y * gr_viewsin);

    // thing is behind view plane?
    if (tz < ZCLIP_PLANE)
        return;

    tx = (tr_x * gr_viewsin) - (tr_y * gr_viewcos);

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned) thing->sprite >= numsprites)
        I_Error("HWR_ProjectSprite: invalid sprite number %i ", thing->sprite);
#endif

    //Fab:02-08-98: 'skin' override spritedef currently used for skin
    if (thing->skin)
        sprdef = &((skin_t *) thing->skin)->spritedef;
    else
        sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
    if ((thing->frame & FF_FRAMEMASK) >= sprdef->numframes)
        I_Error("HWR_ProjectSprite: invalid sprite frame %i : %i for %s", thing->sprite, thing->frame, sprnames[thing->sprite]);
#endif
    sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle(thing->x, thing->y);       // uses viewx,viewy
        rot = (ang - thing->angle + (unsigned) (ANG45 / 2) * 9) >> 29;
        //Fab: lumpid is the index for spritewidth,spriteoffset... tables
        lump = sprframe->lumpid[rot];
        flip = (boolean) sprframe->flip[rot];
    }
    else
    {
        // use single rotation for all views
        rot = 0;        //Fab: for vis->patch below
        lump = sprframe->lumpid[0];     //Fab: see note above
        flip = (boolean) sprframe->flip[0];
    }

    // calculate edges of the shape
    tx -= FIXED_TO_FLOAT( spriteoffset[lump] );

    // project x
    x1 = gr_windowcenterx + (tx * gr_centerx / tz);

    // BP: FOV des sprites, c'est ici que sa ce passe
    // left edge off the right side?
#ifdef NO_MLOOK_EXTENDS_FOV
    if (x1 > gr_viewwidth)
#else
    if ((x1 > gr_viewwidth) && !cv_grmlook_extends_fov.value)
        //if ((x1 > gr_viewwidth) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
        return;

    //faB:tr_x doesnt matter
    // hurdler: it's used in cliptosolidsegs
    tr_x = x1;

    x1 = tx;

    tx += FIXED_TO_FLOAT( spritewidth[lump] );
    x2 = gr_windowcenterx + (tx * gr_centerx / tz);

    // BP: FOV des sprites, ici aussi
    // right edge off the left side
#ifdef NO_MLOOK_EXTENDS_FOV
    if (x2 < 0)
#else
    if ((x2 < 0) && !cv_grmlook_extends_fov.value)
        //if ((x2 < 0) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
        return;

    // sprite completely hidden ?
#ifdef NO_MLOOK_EXTENDS_FOV
    if (!HWR_ClipToSolidSegs((int) tr_x, (int) x2))
#else
    if ((!HWR_ClipToSolidSegs((int) tr_x, (int) x2)) && !cv_grmlook_extends_fov.value)
        //if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
        return;

#if 1
   {
   // [WDJ] from r_things.c
   // This fixes the thing lighting in special sectors
    sector_t*		thingsector;	 // [WDJ] 11/14/2009
    int                 thingmodelsec;
    boolean	        thing_has_model;  // has a model, such as water
    int light;
    fixed_t  gz_top = thing->z + spritetopoffset[lump];
    thingsector = thing->subsector->sector;	 // [WDJ] 11/14/2009
    if(thingsector->numlights)
    {
      int lightlevel;
      light = R_GetPlaneLight(thingsector, gz_top, false);
      lightlevel = *thingsector->lightlist[light].lightlevel;
      if(thingsector->lightlist[light].caster && thingsector->lightlist[light].caster->flags & FF_FOG)
	 sectorlight = LightLevelToLum(lightlevel); // add extralight
      else
         sectorlight = LightLevelToLum_extra(lightlevel, 0);  // extralight=0
    }

    thingmodelsec = thingsector->modelsec;
    thing_has_model = thingsector->model > SM_fluid; // water

    if (thing_has_model)   // only clip things which are in special sectors
    {
      sector_t * thingmodsecp = & sectors[thingmodelsec];
      int phs_modelsec = viewplayer->mo->subsector->sector->modelsec;
      // [WDJ] modelsec is used for more than water, do proper test
      boolean phs_has_mod = viewplayer->mo->subsector->sector->model > SM_fluid;
      // [WDJ] 4/20/2010  Added some structure and ()
      if (phs_has_mod)
      {
	  if( (viewz < sectors[phs_modelsec].floorheight) ?
	      (thing->z >= thingmodsecp->floorheight)
	      : (gz_top < thingmodsecp->floorheight)
	      )
	      return;
	  if( (viewz > sectors[phs_modelsec].ceilingheight) ?
	      ((gz_top < thingmodsecp->ceilingheight) && (viewz >= thingmodsecp->ceilingheight))
	      : (thing->z >= thingmodsecp->ceilingheight)
	      )
	      return;
      }
    }
   }
#if 0
    // gr vis does not have a heightsec (yet ??)
    // [WDJ] Only pass water models, not colormap model sectors
    vis->heightsec = thing_has_model ? thingmodelsec : -1 ; //SoM: 3/17/2000
#endif
#endif
   
    //
    // store information in a vissprite
    //
    vis = HWR_NewVisSprite();
    vis->x1 = x1;
    vis->x2 = tx;
    vis->tz = tz;
    vis->patchlumpnum = sprframe->lumppat[rot];
    vis->flip = flip;
    vis->mobj = thing;

    //Hurdler: 25/04/2000: now support colormap in hardware mode
    if (thing->flags & MF_TRANSLATION)
        vis->colormap = (byte *) translationtables - 256 + ((thing->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    else
        vis->colormap = colormaps;

    // set top/bottom coords
    vis->ty = FIXED_TO_FLOAT( thing->z + spritetopoffset[lump] ) - gr_viewz;

    //CONS_Printf("------------------\nH: sprite  : %d\nH: frame   : %x\nH: type    : %d\nH: sname   : %s\n\n",
    //            thing->sprite, thing->frame, thing->type, sprnames[thing->sprite]);

    if (thing->state->frame & FF_FULLBRIGHT)
        // TODO: disable also the fog
        vis->sectorlight = 0xff;
    else
        vis->sectorlight = sectorlight;
}

#define BASEYCENTER           (BASEVIDHEIGHT/2)
// -----------------+
// HWR_DrawPSprite  : Draw 'player sprites' : weapons, etc.
//                  : fSectorLight ranges 0...1
// -----------------+
void HWR_DrawPSprite(pspdef_t * psp, int lightlevel)
{
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int lump;
    boolean flip;

    wallVert3D wallVerts[4];
    FOutVector projVerts[4];
    int i;
    wallVert3D *wv;
    float tx;
    float ty;
    float x1;
    float x2;

    GlidePatch_t *gpatch;       //sprite patch converted to hardware

    FSurfaceInfo Surf;

    // decide which patch to use
#ifdef RANGECHECK
    if ((unsigned) psp->state->sprite >= numsprites)
        I_Error("HWR_ProjectSprite: invalid sprite number %i ", psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ((psp->state->frame & FF_FRAMEMASK) >= sprdef->numframes)
        I_Error("HWR_ProjectSprite: invalid sprite frame %i : %i ", psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

    //Fab:debug
#ifdef PARANOIA
    if (sprframe == NULL)
        I_Error("sprframes NULL for state %d\n", psp->state - states);
#endif

    lump = sprframe->lumpid[0];
    flip = (boolean) sprframe->flip[0];

    // calculate edges of the shape

    tx = FIXED_TO_FLOAT( (psp->sx - ((BASEVIDWIDTH / 2) << FRACBITS)) );
    tx -= FIXED_TO_FLOAT( spriteoffset[lump] );
    x1 = gr_windowcenterx + (tx * gr_pspritexscale);

    wallVerts[3].x = wallVerts[0].x = tx;

    tx += FIXED_TO_FLOAT( spritewidth[lump] );
    x2 = gr_windowcenterx + (tx * gr_pspritexscale) - 1;

    wallVerts[2].x = wallVerts[1].x = tx;

    //  3--2
    //  | /|
    //  |/ |
    //  0--1
    wallVerts[0].z = wallVerts[1].z = wallVerts[2].z = wallVerts[3].z = 1;
    wallVerts[0].w = wallVerts[1].w = wallVerts[2].w = wallVerts[3].w = 1;

    // cache sprite graphics
    gpatch = W_CachePatchNum(sprframe->lumppat[0], PU_CACHE);
    HWR_GetPatch(gpatch);

    // set top/bottom coords
    ty = FIXED_TO_FLOAT( psp->sy - spritetopoffset[lump] );
    if (cv_splitscreen.value && (cv_grfov.value == 90))
        ty -= 20;       //Hurdler: so it's a bit higher
    if (raven) {
        if (rdraw_viewheight == vid.height || (!cv_scalestatusbar.value && vid.dupy > 1))
            ty += FIXED_TO_FLOAT( PSpriteSY[viewplayer->readyweapon] );
    }

    wallVerts[3].y = wallVerts[2].y = (float) BASEYCENTER - ty;

    ty += gpatch->height;
    wallVerts[0].y = wallVerts[1].y = (float) BASEYCENTER - ty;

    if (flip)
    {
        wallVerts[0].s = wallVerts[3].s = gpatch->max_s;
        wallVerts[2].s = wallVerts[1].s = 0.0f;
    }
    else
    {
        wallVerts[0].s = wallVerts[3].s = 0.0f;
        wallVerts[2].s = wallVerts[1].s = gpatch->max_s;
    }
    wallVerts[3].t = wallVerts[2].t = 0.0f;
    wallVerts[0].t = wallVerts[1].t = gpatch->max_t;

    // project clipped vertices
    wv = wallVerts;
    for (i = 0; i < 4; i++, wv++)
    {
        //Hurdler: sorry, I had to multiply all by 4 for correct splitscreen mode
        projVerts[i].x = wv->x / 40.0f;
        projVerts[i].y = wv->y / 25.0f;
        projVerts[i].z = 4.0f;
        projVerts[i].sow = wv->s;
        projVerts[i].tow = wv->t;
    }

    // clip 2d polygon to view window
    //wClipVerts = ClipToView (projVerts, outVerts, 4 );

    // set transparency and light level

    if (viewplayer->mo->flags & MF_SHADOW)
    {
        if (viewplayer->powers[pw_invisibility] > 4 * TICRATE || viewplayer->powers[pw_invisibility] & 8)
            Surf.FlatColor.s.alpha = 0xff / 3;
        else
            Surf.FlatColor.s.alpha = 2 * 0xff / 3;
    }
    else
        Surf.FlatColor.s.alpha = 0xff;

    if (psp->state->frame & FF_FULLBRIGHT)
    {
        // TODO: remove fog for this sprite !
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
    }
    else
    {
        sector_t *sector = viewplayer->mo->subsector->sector;

        // default opaque mode using alpha 0 for holes
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = lightlevel;
        //Hurdler: colormap test

        if (!fixedcolormap)
        {
            if (sector->ffloors)
            {
                ffloor_t *caster;

                caster = sector->lightlist[R_GetPlaneLight(sector, dup_viewz, false)].caster;
                sector = caster ? &sectors[caster->secnum] : sector;
            }
            if (sector->extra_colormap)
            {
                RGBA_t temp;
                int alpha;

                temp.rgba = sector->extra_colormap->rgba;
                alpha = temp.s.alpha;
                Surf.FlatColor.s.red = ((26 - alpha) * lightlevel + alpha * temp.s.red) / 26;
                Surf.FlatColor.s.blue = ((26 - alpha) * lightlevel + alpha * temp.s.blue) / 26;
                Surf.FlatColor.s.green = ((26 - alpha) * lightlevel + alpha * temp.s.green) / 26;
                Surf.FlatColor.s.alpha = 0xff;
            }
        }
    }

    // invis player doesnt look good with PF_Environment so use PF_Translucent instead
    if (viewplayer->powers[pw_invisibility])
        HWD.pfnDrawPolygon(&Surf, projVerts, 4, PF_Modulated | PF_Translucent | PF_NoDepthTest);
    else
        HWD.pfnDrawPolygon(&Surf, projVerts, 4, PF_Modulated | PF_Environment | PF_NoDepthTest | PF_Occlude);
}

// --------------------------------------------------------------------------
// HWR_DrawPlayerSprites
// --------------------------------------------------------------------------
static void HWR_DrawPlayerSprites(void)
{
    int i;
    pspdef_t *psp;
    int lightlevel, light;

    if (viewplayer->mo->subsector->sector->numlights)
    {
        light = R_GetPlaneLight(viewplayer->mo->subsector->sector, viewplayer->mo->z + viewplayer->mo->info->height, false);
        lightlevel = LightLevelToLum(*viewplayer->mo->subsector->sector->lightlist[light].lightlevel);
    }
    else
        // get light level
        lightlevel = LightLevelToLum(viewplayer->mo->subsector->sector->lightlevel);

    // add all active psprites
    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
    {
        if (psp->state)
            HWR_DrawPSprite(psp, lightlevel);
    }
}

// ==========================================================================
//
// ==========================================================================
void HWR_DrawSkyBackground(player_t * player)
{
    FOutVector v[4];
    angle_t angle;
    float f;

//  3--2
//  | /|
//  |/ |
//  0--1
    HWR_GetTexture(skytexture);

    //Hurdler: the sky is the only texture who need 4.0f instead of 1.0
    //         because it's called just after clearing the screen
    //         and thus, the near clipping plane is set to 3.99
    v[0].x = v[3].x = -4.0f;
    v[1].x = v[2].x = 4.0f;
    v[0].y = v[1].y = -4.0f;
    v[2].y = v[3].y = 4.0f;

    v[0].z = v[1].z = v[2].z = v[3].z = 4.0f;

#define WRAPANGLE (ANGLE_MAX/4)
    angle = ((dup_viewangle + gr_xtoviewangle[0]) % WRAPANGLE);

    v[0].sow = v[3].sow = 1.0 + ((float) angle) / (WRAPANGLE - 1);
    v[2].sow = v[1].sow = ((float) angle) / (WRAPANGLE - 1);

    f = 40 + 200 * FIXED_TO_FLOAT( finetangent[(2048 - ((int) aimingangle >> (ANGLETOFINESHIFT + 1))) & FINEMASK] );
    if (f < 0)
        f = 0;
    if (f > 240 - 127)
        f = 240 - 127;
    v[3].tow = v[2].tow = f / 127.0f;
    v[0].tow = v[1].tow = (f + 127) / 127.0f;   //suppose 256x128 sky...

    HWD.pfnDrawPolygon(NULL, v, 4, 0);
}

// -----------------+
// HWR_ClearView : clear the viewwindow, with maximum z value
// -----------------+
void HWR_ClearView(void)
{
    //  3--2
    //  | /|
    //  |/ |
    //  0--1

    //FIXTHIS faB - enable depth mask, disable color mask

    HWD.pfnGClipRect((int) gr_viewwindowx, (int) gr_viewwindowy, (int) (gr_viewwindowx + gr_viewwidth), (int) (gr_viewwindowy + gr_viewheight), 3.99f);
    HWD.pfnClearBuffer(false, true, 0);

    //disable clip window - set to full size
    // rem by Hurdler
    // HWD.pfnGClipRect (0,0,vid.width,vid.height );
}

// -----------------+
// HWR_SetViewSize  : set projection and scaling values depending on the
//                  : view window size
// -----------------+
void HWR_SetViewSize(int blocks)
{
    // setup view size

    // clamping viewsize is normally not needed coz it's done in R_ExecuteSetViewSize()
    // BEFORE calling here
    if (blocks < 3 || blocks > 12)
        blocks = 10;
    if (blocks > 10 || (blocks == 10 && (!cv_scalestatusbar.value || cv_grtranslucenthud.value < 255)))
    {
        gr_viewwidth = (float) vid.width;
        gr_viewheight = (float) vid.height;
    }
    else
    {
        gr_viewwidth = (float) ((blocks * vid.width / 10) & ~7);
        gr_viewheight = (float) ((blocks * (vid.height - stbarheight / 2) / 10) & ~1);
    }

    if (cv_splitscreen.value)
        gr_viewheight /= 2;

    gr_centerx = gr_viewwidth / 2;
    gr_basecentery = gr_viewheight / 2; //note: this is (gr_centerx * gr_viewheight / gr_viewwidth)

    gr_viewwindowx = (vid.width - gr_viewwidth) / 2;
    gr_windowcenterx = (float) (vid.width / 2);
    if (gr_viewwidth == vid.width)
    {
        gr_baseviewwindowy = 0;
        gr_basewindowcentery = gr_viewheight / 2;       // window top left corner at 0,0
    }
    else
    {
        gr_baseviewwindowy = (vid.height - stbarheight - gr_viewheight) / 2;
        gr_basewindowcentery = (float) ((vid.height - stbarheight) / 2);
    }

    gr_pspritexscale = gr_viewwidth / BASEVIDWIDTH;
    gr_pspriteyscale = ((vid.height * gr_pspritexscale * BASEVIDWIDTH) / BASEVIDHEIGHT) / vid.width;
}

//25/08/99: added by Hurdler for splitscreen correct palette changes and overlay
extern player_t *plyr;
void ST_doPaletteStuff(void);
void ST_overlayDrawer(int playernum);

//Hurdler: 3D water sutffs
static int numplanes = 0;
static int numwalls = 0;

// ==========================================================================
//
// ==========================================================================
void HWR_RenderPlayerView(int viewnumber, player_t * player)
{
    //static float    distance = BASEVIDWIDTH;

    //31/08/99: added by Hurdler for splitscreen correct palette changes
    //                             & splitscreen dynamic lighting
    {
        // do we really need to save player (is it not the same)?
        player_t *saved_player = plyr;
        plyr = player;
        ST_doPaletteStuff();
        plyr = saved_player;
        HWR_SetLights(viewnumber);
    }

    // note: sets viewangle, viewx, viewy, viewz
    R_SetupFrame(player);

    // copy view cam position for local use
    dup_viewx = viewx;
    dup_viewy = viewy;
    dup_viewz = viewz;
    dup_viewangle = viewangle;

    // set window position
    gr_centery = gr_basecentery;
    gr_viewwindowy = gr_baseviewwindowy;
    gr_windowcentery = gr_basewindowcentery;
    if (cv_splitscreen.value && viewnumber == 1)
    {
        //gr_centery += (vid.height/2 );
        gr_viewwindowy += (vid.height / 2);
        gr_windowcentery += (vid.height / 2);
    }

    // hmm solidsegs probably useless here
    //R_ClearDrawSegs ( );
    // useless
    //R_ClearPlanes (player );
    //HWR_ClearSprites ( );

    // check for new console commands.
    NetUpdate();

    gr_viewx = FIXED_TO_FLOAT( dup_viewx );
    gr_viewy = FIXED_TO_FLOAT( dup_viewy );
    gr_viewz = FIXED_TO_FLOAT( dup_viewz );
    gr_viewsin = FIXED_TO_FLOAT( viewsin );
    gr_viewcos = FIXED_TO_FLOAT( viewcos );

    // viewludsin( aimingangle ) instead of viewsin( viewangle )
    gr_viewludsin = FIXED_TO_FLOAT(finecosine[aimingangle >> ANGLETOFINESHIFT]);
    gr_viewludcos = FIXED_TO_FLOAT(-finesine[aimingangle >> ANGLETOFINESHIFT]);

    //04/01/2000: Hurdler: added for T&L
    //                     It should replace all other gr_viewxxx when finished
    atransform.anglex = (float) (aimingangle >> ANGLETOFINESHIFT) * (360.0f / (float) FINEANGLES);
    atransform.angley = (float) (viewangle >> ANGLETOFINESHIFT) * (360.0f / (float) FINEANGLES);
    atransform.x = gr_viewx;    // FIXED_TO_FLOAT( viewx )
    atransform.y = gr_viewy;    // FIXED_TO_FLOAT( viewy )
    atransform.z = gr_viewz;    // FIXED_TO_FLOAT( viewz )
    atransform.scalex = 1;
    atransform.scaley = ORIGINAL_ASPECT;
    atransform.scalez = 1;
    atransform.fovxangle = cv_grfov.value;
    atransform.fovyangle = cv_grfov.value;
    atransform.splitscreen = cv_splitscreen.value;
    gr_fovlud = 1 / tan(cv_grfov.value * PI / 360);

#ifdef NO_MLOOK_EXTENDS_FOV
    // enlage fOV when looking up/down
    HWR_InitTextureMapping();
#endif

    //------------------------------------------------------------------------
    HWR_ClearView();

    if (cv_grfog.value)
        HWR_FoggingOn();

    if (cv_grsky.value)
        HWR_DrawSkyBackground(player);

    //Hurdler: it doesn't work in splitscreen mode
    //cv_grsky.value = false;
    cv_grsky.value = cv_splitscreen.value;

    // added by Hurdler for FOV 120
//    if (cv_grfov.value != 90)
//        HWD.pfnSetSpecialState(HWD_SET_FOV, cv_grfov.value);

    //14/11/99: Hurdler: we will add lights while processing sprites
    //it doesn't work with all subsectors (if we use AddSprites to do that).
    //TOO bad, that's why I removed this line (until this is fixed).
//    HWR_ResetLights();

    HWR_ClearSprites();

    HWR_ClearClipSegs();

    //04/01/2000: Hurdler: added for T&L
    //                     Actually it only works on Walls and Planes
    HWD.pfnSetTransform(&atransform);
#ifdef TERM_TRANSFORM
    // [WDJ] transform upgrade
    HWR_set_view_transform();
#endif

    validcount++;
    HWR_RenderBSPNode(numnodes - 1);

#ifndef NO_MLOOK_EXTENDS_FOV
    if (cv_grmlook_extends_fov.value && (aimingangle || cv_grfov.value > 90))
    {
        dup_viewangle += ANG90;
        HWR_ClearClipSegs();
        HWR_RenderBSPNode(numnodes - 1);        //lefT

        dup_viewangle += ANG90;
        if (cv_grmlook_extends_fov.value == 2 && ((int) aimingangle > ANG45 || (int) aimingangle < -ANG45))
        {
            HWR_ClearClipSegs();
            HWR_RenderBSPNode(numnodes - 1);    //back
        }

        dup_viewangle += ANG90;
        HWR_ClearClipSegs();
        HWR_RenderBSPNode(numnodes - 1);        //right

        dup_viewangle += ANG90;
    }
#endif
    if (numwalls && !cv_grtranswall.value)      //Hurdler: render transparent walls after everything
    {
        HWR_RenderTransparentWalls();
    }

    // Check for new console commands.
    NetUpdate();

    //14/11/99: Hurdler: moved here because it doesn't work with
    // subsector, see other comments;
    HWR_ResetLights();

    // Draw MD2 and sprites
    HWR_SortVisSprites();
    HWR_DrawMD2S();
    // Draw the sprites like it was done previously without T&L
    HWD.pfnSetTransform(NULL);
    HWR_DrawSprites();

#ifdef NEWCORONAS
    //Hurdler: they must be drawn before translucent planes, what about gl fog?
    HWR_DrawCoronas();
#endif

    if (numplanes || numwalls)  //Hurdler: render 3D water and transparent walls after everything
    {
        HWD.pfnSetTransform(&atransform);
        if (numplanes)
            HWR_Render3DWater();
        if (numwalls && cv_grtranswall.value)
            HWR_RenderTransparentWalls();
        HWD.pfnSetTransform(NULL);
    }

    // Check for new console commands.
    NetUpdate();

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset && !camera.chase && cv_psprites.value && script_camera_on == false)
        HWR_DrawPlayerSprites();

    //------------------------------------------------------------------------
    // put it off for menus etc
    if (cv_grfog.value)
        HWD.pfnSetSpecialState(HWD_SET_FOG_MODE, 0);

    // added by Hurdler for correct splitscreen
    // moved here by hurdler so it works with the new near clipping plane
    HWD.pfnGClipRect(0, 0, vid.width, vid.height, 0.9f);
}

// ==========================================================================
//                                                                        FOG
// ==========================================================================

//FIXTHIS faB

static unsigned int atohex(char *s)
{
    int iCol;
    char *sCol;
    char cCol;
    int i;

    if (lstrlen(s) < 6)
        return 0;

    iCol = 0;
    sCol = s;
    for (i = 0; i < 6; i++, sCol++)
    {
        iCol <<= 4;
        cCol = *sCol;
        if (cCol >= '0' && cCol <= '9')
            iCol |= cCol - '0';
        else
        {
            if (cCol >= 'F')
                cCol -= ('a' - 'A');
            if (cCol >= 'A' && cCol <= 'F')
                iCol = iCol | (cCol - 'A' + 10);
        }
    }
    //CONS_Printf ("col %x\n", iCol);
    return iCol;
}

void HWR_FoggingOn(void)
{
    HWD.pfnSetSpecialState(HWD_SET_FOG_COLOR, atohex(cv_grfogcolor.string));
    HWD.pfnSetSpecialState(HWD_SET_FOG_DENSITY, cv_grfogdensity.value);
    HWD.pfnSetSpecialState(HWD_SET_FOG_MODE, 1);
}

// ==========================================================================
//                                                         3D ENGINE COMMANDS
// ==========================================================================

static void CV_grFov_OnChange(void)
{
    // autoset mlook when FOV > 90
    if ((!cv_grmlook_extends_fov.value) && (cv_grfov.value > 90))
        CV_SetValue(&cv_grmlook_extends_fov, 1);
}

static void CV_grPolygonSmooth_OnChange(void)
{
    HWD.pfnSetSpecialState(HWD_SET_POLYGON_SMOOTH, cv_grpolygonsmooth.value);
}

/*
static void CV_grFogColor_OnChange (void)
{
    //HWD.pfnSetSpecialState (HWD_SET_FOG_COLOR, atohex(cv_grfogcolor.string));
}
*/
static void Command_GrStats_f(void)
{
    //debug
    Z_CheckHeap(9875);

    CONS_Printf("Patch info headers : %7d kb\n", Z_TagUsage(PU_HWRPATCHINFO) >> 10);
    CONS_Printf("3D Texture cache   : %7d kb\n", Z_TagUsage(PU_HWRCACHE) >> 10);
    CONS_Printf("Plane polygone     : %7d kb\n", Z_TagUsage(PU_HWRPLANE) >> 10);
}

// **************************************************************************
//                                                            3D ENGINE SETUP
// **************************************************************************

// --------------------------------------------------------------------------
// Add hardware engine commands & consvars
// --------------------------------------------------------------------------
//added by Hurdler: console varibale that are saved
void HWR_AddCommands(void)
{
    CV_RegisterVar(&cv_grgammablue);
    CV_RegisterVar(&cv_grgammagreen);
    CV_RegisterVar(&cv_grgammared);
    //CV_RegisterVar (&cv_grcontrast);
    //CV_RegisterVar (&cv_grpolygonsmooth); // moved below
    CV_RegisterVar(&cv_grmd2);
    CV_RegisterVar(&cv_grtranswall);
    CV_RegisterVar(&cv_grmblighting);
    CV_RegisterVar(&cv_grstaticlighting);
    CV_RegisterVar(&cv_grdynamiclighting);
    CV_RegisterVar(&cv_grcoronas);
    CV_RegisterVar(&cv_grcoronasize);
    CV_RegisterVar(&cv_grfov);
    CV_RegisterVar(&cv_grfogdensity);
    CV_RegisterVar(&cv_grfogcolor);
    CV_RegisterVar(&cv_grfog);
    CV_RegisterVar(&cv_grmlook_extends_fov);
    CV_RegisterVar(&cv_grfiltermode);
    CV_RegisterVar(&cv_grcorrecttricks);
    CV_RegisterVar(&cv_grsolvetjoin);
}

void HWR_AddEngineCommands(void)
{
    CV_RegisterVar(&cv_grpolygonsmooth);

    // engine state variables
    //CV_RegisterVar (&cv_grsky);
    //CV_RegisterVar (&cv_grzbuffer);
    //CV_RegisterVar (&cv_grclipwalls);
    CV_RegisterVar(&cv_grrounddown);

    // engine development mode variables
    // - usage may vary from version to version..
    CV_RegisterVar(&cv_gralpha);
    CV_RegisterVar(&cv_grbeta);
    CV_RegisterVar(&cv_grgamma);

    // engine commands
    COM_AddCommand("gr_stats", Command_GrStats_f);
}

// --------------------------------------------------------------------------
// Setup the hardware renderer
// --------------------------------------------------------------------------
void HWR_Startup(void)
{
    static int startupdone = 0;

    CONS_Printf("HWR_Startup()\n");

    // setup GlidePatch_t scaling
    gr_patch_scalex = 1.0 / (float) vid.width;
    gr_patch_scaley = 1.0 / (float) vid.height;

    // initalze light lut translation
    InitLumLut();

    // do this once
    if (!startupdone)
    {
        HWR_InitPolyPool();
        // add console cmds & vars
        HWR_AddEngineCommands();
        HWR_InitTextureCache();

        // for test water translucent surface
        doomwaterflat = W_CheckNumForName("FWATER1");
        if (doomwaterflat == -1)        // if FWATER1 not found (in doom shareware)
            doomwaterflat = W_GetNumForName("WATER0");

        HWR_InitMD2();
    }

    HWR_InitLight();

    if (rendermode == render_opengl)
        textureformat = patchformat = GR_RGBA;

    startupdone = 1;
}

// --------------------------------------------------------------------------
// Free resources allocated by the hardware renderer
// --------------------------------------------------------------------------
void HWR_Shutdown(void)
{
    CONS_Printf("HWR_Shutdown()\n");
    HWR_FreeExtraSubsectors();
    HWR_FreePolyPool();
    HWR_FreeTextureCache();
}


#ifdef TERM_TRANSFORM
// temporary, to supply old call
void transform(float *cx, float *cy, float *cz)
{
    // translation
    // Combined transforms for position, direction, look up/down, and scaling
    float tr_x = *cx - gr_viewx;  // wx is passed in *cx
    float tr_y = *cz - gr_viewy;  // wy is passed in *cz
    float tr_z = *cy - gr_viewz;  // wz is passed in *cy
    *cx = (tr_x * world_trans_x_to_x)
       + (tr_y * world_trans_y_to_x);
    *cy = (tr_x * world_trans_x_to_y )
       + (tr_y * world_trans_y_to_y )
       + (tr_z * world_trans_z_to_y );
    *cz = (tr_x * world_trans_x_to_z )
       + (tr_y * world_trans_y_to_z )
       + (tr_z * world_trans_z_to_z );
}

#else
void transform(float *cx, float *cy, float *cz)
{
    float tr_x, tr_y;
    // translation
    tr_x = *cx - gr_viewx;
    tr_y = *cz - gr_viewy;
//   *cy = *cy;

    // rotation around vertical y axis
    *cx = (tr_x * gr_viewsin) - (tr_y * gr_viewcos);
    tr_x = (tr_x * gr_viewcos) + (tr_y * gr_viewsin);

    //look up/down
    tr_y = *cy - gr_viewz;

    *cy = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin);
    *cz = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos);

    //scale y before frustum so that frustum can be scaled to screen height
    *cy *= ORIGINAL_ASPECT * gr_fovlud;
    *cx *= gr_fovlud;
}
#endif

//Hurdler: 3D Water stuff
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX_3DWATER 512
static planeinfo_t *planeinfo = NULL;

void HWR_Add3DWater(int lumpnum, extrasubsector_t * xsub, fixed_t fixedheight, int lightlevel, int alpha)
{
    if (!(numplanes % MAX_3DWATER))
    {
        planeinfo = (planeinfo_t *) realloc(planeinfo, (numplanes + MAX_3DWATER) * sizeof(planeinfo_t));
    }
    planeinfo[numplanes].fixedheight = fixedheight;
    planeinfo[numplanes].lightlevel = lightlevel;
    planeinfo[numplanes].lumpnum = lumpnum;
    planeinfo[numplanes].xsub = xsub;
    planeinfo[numplanes].alpha = alpha;
    numplanes++;
}

#define DIST_PLANE(i) ABS(planeinfo[(i)].fixedheight-dup_viewz)

//FIXME: this doesn't work yet
void HWR_QuickSortPlane(int low, int high)
{
    if (low < high)
    {
        int lo = low;
        int hi = high + 1;
        int pivot = DIST_PLANE(low);

        for (;;)
        {
            while (DIST_PLANE(++lo) < pivot);
            while (DIST_PLANE(--hi) > pivot);
            if (lo < hi)
            {   //swap (ia, low, hi);
                planeinfo_t temp;
                memcpy(&temp, &planeinfo[low], sizeof(planeinfo_t));
                memcpy(&planeinfo[low], &planeinfo[hi], sizeof(planeinfo_t));
                memcpy(&planeinfo[hi], &temp, sizeof(planeinfo_t));
            }
            else
                break;
        }
        {       //swap (ia, low, hi);
            planeinfo_t temp;
            memcpy(&temp, &planeinfo[low], sizeof(planeinfo_t));
            memcpy(&planeinfo[low], &planeinfo[hi], sizeof(planeinfo_t));
            memcpy(&planeinfo[hi], &temp, sizeof(planeinfo_t));
        }
        HWR_QuickSortPlane(low, hi - 1);
        HWR_QuickSortPlane(hi + 1, high);
    }
}

void HWR_Render3DWater()
{
    int i;

    //bubble sort 3D Water for correct alpha blending
    //FIXME: do a quick sort since there can be lots of plane to sort
    {
        int permut = 1;
        while (permut)
        {
            int j;
	    permut = 0;
            for (j = 0; j < numplanes - 1; j++)
            {
                if (ABS(planeinfo[j].fixedheight - dup_viewz) < ABS(planeinfo[j + 1].fixedheight - dup_viewz))
                {
                    planeinfo_t temp;
                    memcpy(&temp, &planeinfo[j + 1], sizeof(planeinfo_t));
                    memcpy(&planeinfo[j + 1], &planeinfo[j], sizeof(planeinfo_t));
                    memcpy(&planeinfo[j], &temp, sizeof(planeinfo_t));
                    permut = 1;
                }
            }
        }
    }
    //HWR_QuickSortPlane(0, numplanes-1);

    gr_frontsector = NULL;      //Hurdler: gr_fronsector is no longer valid
    for (i = 0; i < numplanes; i++)
    {
        FBITFIELD PolyFlags = PF_Translucent | (planeinfo[i].alpha << 24);

        HWR_GetFlat(planeinfo[i].lumpnum);
        HWR_RenderPlane(planeinfo[i].xsub, planeinfo[i].fixedheight, PolyFlags, planeinfo[i].lightlevel, planeinfo[i].lumpnum);
    }
    numplanes = 0;
}

//Hurdler: manage transparent texture a little better
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX_TRANSPARENTWALL 256

typedef struct
{
    wallVert3D wallVerts[4];
    FSurfaceInfo Surf;
    int texnum;
    int blend;
} wallinfo_t;

static wallinfo_t *wallinfo = NULL;

// Called from HWR_SplitWall, HWR_StoreWallRange
void HWR_AddTransparentWall(wallVert3D * wallVerts, FSurfaceInfo * pSurf, int texnum, int blend)
{
    if (!(numwalls % MAX_TRANSPARENTWALL))
    {
        wallinfo = (wallinfo_t *) realloc(wallinfo, (numwalls + MAX_TRANSPARENTWALL) * sizeof(wallinfo_t));
    }
    memcpy(wallinfo[numwalls].wallVerts, wallVerts, sizeof(wallinfo[numwalls].wallVerts));
    memcpy(&wallinfo[numwalls].Surf, pSurf, sizeof(FSurfaceInfo));
    wallinfo[numwalls].texnum = texnum;
    wallinfo[numwalls].blend = blend;
    numwalls++;
}

void HWR_RenderWall(wallVert3D * wallVerts, FSurfaceInfo * pSurf, int blend);

void HWR_RenderTransparentWalls()
{
    int i;

    /*
       { // sorting is disbale for now, do it!
       int permut = 1;
       while (permut)
       {
       int j;
       for (j=0, permut=0; j<numwalls-1; j++)
       {
       if (ABS(wallinfo[j].fixedheight-dup_viewz) < ABS(wallinfo[j+1].fixedheight-dup_viewz))
       {
       wallinfo_t temp;
       memcpy(&temp, &wallinfo[j+1], sizeof(wallinfo_t));
       memcpy(&wallinfo[j+1], &wallinfo[j], sizeof(wallinfo_t));
       memcpy(&wallinfo[j], &temp, sizeof(wallinfo_t));
       permut = 1;
       }
       }
       }
       }
     */

    for (i = 0; i < numwalls; i++)
    {
        HWR_GetTexture(wallinfo[i].texnum);
        HWR_RenderWall(wallinfo[i].wallVerts, &wallinfo[i].Surf, wallinfo[i].blend);
    }
    numwalls = 0;
}

void HWR_RenderWall(wallVert3D * wallVerts, FSurfaceInfo * pSurf, int blend)
{
    FOutVector trVerts[4];
    int i;
    FOutVector *wv;

    // transform
    wv = trVerts;
    // it sounds really stupid to do this conversion with the new T&L code
    // we should directly put the right information in the right structure
    // wallVerts3D seems ok, doesn't need FOutVector
    // also remove the light copy
    for (i = 0; i < 4; i++, wv++, wallVerts++)
    {
        wv->sow = wallVerts->s;
        wv->tow = wallVerts->t;
        wv->x = wallVerts->x;
        wv->y = wallVerts->y;
        wv->z = wallVerts->z;
    }

    HWD.pfnDrawPolygon(pSurf, trVerts, 4, blend | PF_Modulated | PF_Occlude | PF_Clip);

    if (gr_curline->linedef->splats && cv_splats.value)
        HWR_DrawSegsSplats(pSurf);

    //Hurdler: TODO: do static lighting using gr_curline->lm
    HWR_WallLighting(trVerts);
}
