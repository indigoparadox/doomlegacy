// $Id$
//
// chasecam (and maybe other cameras) class implementation

#include "doomdef.h"
#include "p_camera.h"
#include "g_map.h"
#include "command.h"
#include "tables.h"
#include "r_main.h"

Camera camera;

consvar_t cv_cam_dist   = {"cam_dist"  ,"128"  ,CV_FLOAT,NULL};
consvar_t cv_cam_height = {"cam_height", "20"   ,CV_FLOAT,NULL};
consvar_t cv_cam_speed  = {"cam_speed" ,  "0.25",CV_FLOAT,NULL};

short G_ClipAimingPitch(int *aiming);

void Camera::ClearCamera()
{
  if (mp)
    Remove();
}

void Camera::MoveChaseCamera(Actor *p)
{
  fixed_t tx, ty, tz;

  if (p == NULL)
    I_Error("MoveChaseCamera: no target\n");

  if (mp == NULL)
    ResetCamera(p);

  angle_t ang = p->angle;

  // sets ideal cam pos
  fixed_t dist = cv_cam_dist.value;
  tx = p->x - FixedMul( finecosine[(ang>>ANGLETOFINESHIFT) & FINEMASK], dist);
  ty = p->y - FixedMul(   finesine[(ang>>ANGLETOFINESHIFT) & FINEMASK], dist);
  tz = p->z + (cv_viewheight.value << FRACBITS) + cv_cam_height.value;

  // P_PathTraverse ( p->x, p->y, x, y, PT_ADDLINES, PTR_UseTraverse );

  // move camera down to move under lower ceilings
  subsector_t *newsubsec = mp->R_IsPointInSubsector((p->x + tx) >> 1, (p->y + ty) >> 1);
              
  if (!newsubsec)
    {
      // use player sector 
      if (p->subsector->sector->ceilingheight - height < tz)
	tz = p->subsector->sector->ceilingheight - height - 11*FRACUNIT;
      // don't be blocked by a opened door
    }
  else if (newsubsec->sector->ceilingheight - height < tz)
    // no fit
    tz = newsubsec->sector->ceilingheight - height-11*FRACUNIT;

  // is the camera fit is there own sector
  newsubsec = mp->R_PointInSubsector(tx, ty);
  if (newsubsec->sector->ceilingheight - height < tz)
    tz = newsubsec->sector->ceilingheight - height - 11*FRACUNIT;


  // point viewed by the camera
  // this point is just 64 unit forward the player
  dist = 64 << FRACBITS;
  fixed_t viewpointx, viewpointy;

  viewpointx = p->x + FixedMul( finecosine[(ang>>ANGLETOFINESHIFT) & FINEMASK], dist);
  viewpointy = p->y + FixedMul( finesine[(ang>>ANGLETOFINESHIFT) & FINEMASK], dist);

  angle = R_PointToAngle2(tx, ty, viewpointx, viewpointy);

  // follow the player
  px = FixedMul(tx - x, cv_cam_speed.value);
  py = FixedMul(ty - y, cv_cam_speed.value);
  pz = FixedMul(tz - z, cv_cam_speed.value);

  // compute aiming to look the viewed point
  float f1, f2;
  f1 = FIXED_TO_FLOAT(viewpointx - x);
  f2 = FIXED_TO_FLOAT(viewpointy - y);
  dist = sqrt(f1*f1+f2*f2)*FRACUNIT;
  angle = R_PointToAngle2(0, z, dist, p->z + (p->height>>1)
			  + finesine[(p->aiming>>ANGLETOFINESHIFT) & FINEMASK] * 64);

  G_ClipAimingPitch((int *)&angle);
  dist = aiming - angle;
  aiming -= (dist >> 3);
}


//
// was P_MoveCamera :
// make sure the camera is not outside the world
// and looks at the thing it is supposed to
//

void Camera::ResetCamera(Actor *p)
{
  chase = true;

  flags = MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY|MF_FLOAT| MF_NOTRIGGER | MF_NOCLIPTHING;
  flags2 = MF2_SLIDE;

  health = 1000;
  mass = 10*FRACUNIT;
  radius = 20*FRACUNIT;
  height = 16*FRACUNIT;

  x = p->x;
  y = p->y;
  z = p->z + (cv_viewheight.value << FRACBITS);

  if (mp == NULL)
    {
      // add cam to the map
      p->mp->AddThinker(this);
      //cam = p->mp->SpawnActor(x,y,z, MT_CHASECAM);
    }

  angle = p->angle;
  aiming = 0;

  // hey we should make sure that the sounds are heard from the camera
  // instead of the marine's head : TODO
}


/*
bool PTR_FindCameraPoint (intercept_t* in)
{
  
  static fixed_t cameraz;
  int         side;
	fixed_t             slope;
	fixed_t             dist;
	line_t*             li;

	li = in->d.line;

	if ( !(li->flags & ML_TWOSIDED) )
        return false;

    // crosses a two sided line
    //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
    //                lowfloor is the height of the lowest floor
    //                         (be it front or back)
    P_LineOpening (li);

    dist = FixedMul (attackrange, in->frac);

    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
    //added:18-02-98: comments :
    // find the slope aiming on the border between the two floors
    slope = FixedDiv (openbottom - cameraz , dist);
    if (slope > aimslope)
    return false;
    }

    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
    slope = FixedDiv (opentop - shootz , dist);
    if (slope < aimslope)
    goto hitline;
    }

    return true;

    // hit line
    hitline:
  // stop the search
  return false;
}
*/
