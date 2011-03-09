// Emacs style mode select -*- C++ -*-
//----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2000 Simon Howard
// Copyright (C) 2001-2011 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Log: t_vari.c,v $
// Revision 1.2  2004/07/27 08:19:37  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------
//
// Variables.
//
// Variable code: create new variables, look up variables, get value,
// set value
//
// variables are stored inside the individual scripts, to allow for
// 'local' and 'global' variables. This way, individual scripts cannot
// access variables in other scripts. However, 'global' variables can
// be made which can be accessed by all scripts. These are stored inside
// a dedicated script_t which exists only to hold all of these global
// variables.
//
// functions are also stored as variables, these are kept in the global
// script so they can be accessed by all scripts. function variables
// cannot be set or changed inside the scripts themselves.
//
//---------------------------------------------------------------------------

/* includes ************************/

#include <stdio.h>
#include <string.h>
#include "z_zone.h"

#include "t_script.h"
#include "t_parse.h"
#include "t_vari.h"
#include "t_func.h"

// the global script just holds all
// the global variables
script_t global_script;

// the hub script holds all the variables
// shared between levels in a hub
script_t hub_script;


// initialise the global script: clear all the variables

void init_variables( void )
{
  int i;
  
  global_script.parent = NULL;    // globalscript is the root script
  hub_script.parent = &global_script; // hub_script is the next level down
  
  for(i=0; i<VARIABLESLOTS; i++)
    global_script.variables[i] = hub_script.variables[i] = NULL;
  
  // any hardcoded global variables can be added here
}

void T_ClearHubScript( void )
{
  int i;

  for(i=0; i<VARIABLESLOTS; i++)
  {
      while(hub_script.variables[i])
      {
	  fs_variable_t *next = hub_script.variables[i]->next;
	  if(hub_script.variables[i]->type == FSVT_string)
	    Z_Free(hub_script.variables[i]->value.s);
	  Z_Free(hub_script.variables[i]);
	  hub_script.variables[i] = next;
      }
  }
}

// find_variable checks through the current script, level script
// and global script to try to find the variable of the name wanted

fs_variable_t * find_variable(char *name)
{
  fs_variable_t *var;
  script_t *current;
  
  current = fs_current_script;
  
  while(current)
  {
      // check this script
      if((var = variableforname(current, name)))
	return var;
      current = current->parent;    // try the parent of this one
  }

  return NULL;    // no variable
}

// create a new variable in a particular script.
// returns a pointer to the new variable.

fs_variable_t * new_variable(script_t *script, char *name, int vtype)
{
  int n;
  fs_variable_t *newvar;
  int tagtype =
    script==&global_script || script==&hub_script ? PU_STATIC : PU_LEVEL;
  
  // find an empty slot first

  newvar = Z_Malloc(sizeof(fs_variable_t), tagtype, 0);
  newvar->name = (char*)Z_Strdup(name, tagtype, 0);
  newvar->type = vtype;
  
  if(vtype == FSVT_string)
  {
      // 256 bytes for string
      newvar->value.s = Z_Malloc(256, tagtype, 0);
      newvar->value.s[0] = 0;
  }
  else if(vtype == FSVT_array)
  {
     newvar->value.a = NULL;
  }

  else
    newvar->value.i = 0;
  
  // now hook it into the hashchain
  
  n = variable_hash(name);
  newvar->next = script->variables[n];
  script->variables[n] = newvar;
  
  return script->variables[n];
}

// search a particular script for a variable, which
// is returned if it exists

fs_variable_t * variableforname(script_t *script, char *name)
{
  int n;
  fs_variable_t *current;
  
  n = variable_hash(name);
  
  current = script->variables[n];
  
  while(current)
  {
      if(!strcmp(name, current->name))        // found it?
	return current;         
      current = current->next;        // check next in chain
  }
  
  return NULL;
}

// free all the variables in a given script
void clear_variables(script_t *script)
{
  int i;
  fs_variable_t *current, *next;
  
  for(i=0; i<VARIABLESLOTS; i++)
  {
      current = script->variables[i];
      
      // go thru this chain
      while(current)
      {
	  // labels are added before variables, during
	  // preprocessing, so will be at the end of the chain
	  // we can be sure there are no more variables to free
	  if(current->type == FSVT_label)
	    break;
	  
	  next = current->next; // save for after freeing
	  
	  // if a string, free string data
	  if(current->type == FSVT_string)
	    Z_Free(current->value.s);
	  
	  current = next; // go to next in chain
      }
      // start of labels or NULL
      script->variables[i] = current;
  }
}

// returns an fs_value_t holding the current
// value of a particular variable.
fs_value_t getvariablevalue(fs_variable_t *v)
{
  fs_value_t returnvar;
  
  if(!v) return nullvar;
  
  if(v->type == FSVT_pString)
  {
      returnvar.type = FSVT_string;
      returnvar.value.s = *v->value.pS;
  }
  else if(v->type == FSVT_pInt)
  {
      returnvar.type = FSVT_int;
      returnvar.value.i = *v->value.pI;
  }
  else if(v->type == FSVT_pFixed)
  {
      returnvar.type = FSVT_fixed;
      returnvar.value.f = *v->value.pFixed;
  }
  else if(v->type == FSVT_pMobj)
  {
      returnvar.type = FSVT_mobj;
      returnvar.value.mobj = *v->value.pMobj;
  }
  else if(v->type == FSVT_pArray)
  {
      returnvar.type = FSVT_array;
      returnvar.value.a = *v->value.pA;
  }
  else
  {
      returnvar.type = v->type;
      // copy the value
      returnvar.value.i = v->value.i;
  }
  
  return returnvar;
}

// set a variable to a value from an fs_value_t

void setvariablevalue(fs_variable_t *v, fs_value_t newvalue)
{
  if(fs_killscript) return;  // protect the variables when killing script
  
  if(!v) return;
  
  if(v->type == FSVT_const)
  {
      // const adapts to the value it is set to
      v->type = newvalue.type;

      // alloc memory for string
      if(v->type == FSVT_string)   // static incase a global_script var
	v->value.s = Z_Malloc(256, PU_STATIC, 0);
  }
  
  if(v->type == FSVT_int)
      v->value.i = intvalue(newvalue);

  if(v->type == FSVT_string)
    strcpy(v->value.s, stringvalue(newvalue));

  if(v->type == FSVT_fixed)
    v->value.fixed = fixedvalue(newvalue);

  if(v->type == FSVT_mobj)
      v->value.mobj = MobjForSvalue(newvalue);


  if(v->type == FSVT_array)
  {
     if(newvalue.type != FSVT_array)
     {
	script_error("cannot coerce value to array type\n");
	return;
     }
     v->value.a = newvalue.value.a;
  }


  if(v->type == FSVT_pInt)
      *v->value.pI = intvalue(newvalue);

  if(v->type == FSVT_pString)
  {
      // free old value
      free(*v->value.pS);
      
      // dup new string
      *v->value.pS = strdup(stringvalue(newvalue));
  }

  if(v->type == FSVT_pFixed)
    *v->value.pFixed = fixedvalue(newvalue);

  if(v->type == FSVT_pMobj)
      *v->value.pMobj = MobjForSvalue(newvalue);
  
  if(v->type == FSVT_pArray)
  {
     if(newvalue.type != FSVT_array)
     {
	script_error("cannot coerce value to array type\n");
	return;
     }
     *v->value.pA = newvalue.value.a;
  }

  if(v->type == FSVT_function)
    script_error("attempt to set function to a value\n");

}



fs_variable_t * add_game_int(char *name, int *var)
{
  fs_variable_t* newvar;
  newvar = new_variable(&global_script, name, FSVT_pInt);
  newvar->value.pI = var;

  return newvar;
}


fs_variable_t * add_game_fixed(char *name, fixed_t *fixed)
{
  fs_variable_t *newvar;
  newvar = new_variable(&global_script, name, FSVT_pFixed);
  newvar->value.pFixed = fixed;

  return newvar;
}

fs_variable_t * add_game_string(char *name, char **var)
{
  fs_variable_t* newvar;
  newvar = new_variable(&global_script, name, FSVT_pString);
  newvar->value.pS = var;

  return newvar;
}



fs_variable_t * add_game_mobj(char *name, mobj_t **mo)
{
  fs_variable_t* newvar;
  newvar = new_variable(&global_script, name, FSVT_pMobj);
  newvar->value.pMobj = mo;

  return newvar;
}


/********************************
                     FUNCTIONS
 ********************************/
// functions are really just variables
// of type FSVT_function. there are two
// functions to control functions (heh)

// new_function: just creates a new variable
//      of type FSVT_function. give it the
//      handler function to be called, and it
//      will be stored as a pointer appropriately.

// evaluate_function: once parse.c is pretty
//      sure it has a function to run it calls
//      this. evaluate_function makes sure that
//      it is a function call first, then evaluates all
//      the arguments given to the function.
//      these are built into an argc/argv-style
//      list. the function 'handler' is then called.

// the basic handler functions are in func.c

int t_argc;                     // number of arguments
fs_value_t *t_argv;               // arguments
fs_value_t t_return;              // returned value

fs_value_t evaluate_function(int start, int stop)
{
  fs_variable_t *func = NULL;
  int startpoint, endpoint;

  // the arguments need to be built locally in case of
  // function returns as function arguments eg
  // print("here is a random number: ", rnd() );
  
  int argc;
  fs_value_t argv[MAXARGS];

  if(tokentype[start] != TT_function || tokentype[stop] != TT_operator
     || tokens[stop][0] != ')' )
    script_error("misplaced closing bracket\n");

  // all the functions are stored in the global script
  else if( !(func = variableforname(&global_script, tokens[start]))  )
    script_error("no such function: '%s'\n",tokens[start]);

  else if(func->type != FSVT_function)
    script_error("'%s' not a function\n", tokens[start]);

  if(fs_killscript) return nullvar; // one of the above errors occurred

  // build the argument list
  // use a C command-line style system rather than
  // a system using a fixed length list

  argc = 0;
  endpoint = start + 2;   // ignore the function name and first bracket
  
  while(endpoint < stop)
  {
      startpoint = endpoint;
      endpoint = find_operator(startpoint, stop-1, ",");
      
      // check for -1: no more ','s 
      if(endpoint == -1)
      {               // evaluate the last expression
	  endpoint = stop;
      }
      if(endpoint-1 < startpoint)
	break;
      
      argv[argc] = evaluate_expression(startpoint, endpoint-1);
      endpoint++;    // skip the ','
      argc++;
  }

  // store the arguments in the global arglist
  t_argc = argc;
  t_argv = argv;

  if(fs_killscript) return nullvar;
  
  // now run the function
  func->value.handler();
  
  // return the returned value
  return t_return;
}

// structure dot (.) operator
// there are not really any structs in FraggleScript, it's
// just a different way of calling a function that looks
// nicer. ie
//      a.b()  = a.b   =  b(a)
//      a.b(c) = b(a,c)

// this function is just based on the one above

fs_value_t OPstructure(int start, int n, int stop)
{
  fs_variable_t *func = NULL;
  
  // the arguments need to be built locally in case of
  // function returns as function arguments eg
  // print("here is a random number: ", rnd() );
  
  int argc;
  fs_value_t argv[MAXARGS];

  // all the functions are stored in the global script
  if( !(func = variableforname(&global_script, tokens[n+1]))  )
    script_error("no such function: '%s'\n",tokens[n+1]);
  
  else if(func->type != FSVT_function)
    script_error("'%s' not a function\n", tokens[n+1]);

  if(fs_killscript) return nullvar; // one of the above errors occurred
  
  // build the argument list

  // add the left part as first arg

  argv[0] = evaluate_expression(start, n-1);
  argc = 1; // start on second argv

  if(stop != n+1)         // can be a.b not a.b()
  {
      int startpoint, endpoint;

      // ignore the function name and first bracket
      endpoint = n + 3;
      
      while(endpoint < stop)
      {
	  startpoint = endpoint;
	  endpoint = find_operator(startpoint, stop-1, ",");
	  
	  // check for -1: no more ','s 
	  if(endpoint == -1)
	  {               // evaluate the last expression
	      endpoint = stop;
	  }
	  if(endpoint-1 < startpoint)
	    break;
	  
	  argv[argc] = evaluate_expression(startpoint, endpoint-1);
	  endpoint++;    // skip the ','
	  argc++;
      }
  }

  // store the arguments in the global arglist
  t_argc = argc;
  t_argv = argv;
  
  if(fs_killscript) return nullvar;
  
  // now run the function
  func->value.handler();
  
  // return the returned value
  return t_return;
}


// create a new function. returns the function number

fs_variable_t * new_function(char *name, void (*handler)() )
{
  fs_variable_t *newvar;

  // create the new variable for the function
  // add to the global script
  
  newvar = new_variable(&global_script, name, FSVT_function);
  
  // add neccesary info
  
  newvar->value.handler = handler;

  return newvar;
}

