README for Doom Legacy [VERSION], svn[SVNREV]
======================================
Released [DATE]

What you will need
------------------

1. Doom Legacy executable (e.g. doomlegacy_[VERSION]_windows_32_sdl.zip)
2. The shared files in doomlegacy_[VERSION]_common.zip (contains legacy.wad [WADVERSION] and the docs)
3. Run-time libraries (SDL, SDL_mixer), unless you already have them installed
4. One or more IWADs containing the game data (Doom, Final Doom, UltDoom, Plutonia, TNT, Heretic, FreeDoom, etc.)

We provide precompiled binaries for Linux and Win32, built with default options.
You can also compile a binary yourself, the source code package contains the compilation instructions.

Doom Legacy SDL version requires the SDL and SDL_mixer run-time libraries.
Using SDL is recommended, but there are some other compile options (like X11 native).


Play setup
----------

Extract the executable and the common package to the same folder.

When run, Doom Legacy will create a hidden directory in your home directory to keep your config
and game save files.  This directory name varies by operating system, as
set by DEFAULTDIR1 and DEFAULTDIR2.
The alternative home directory DEFHOME is used if a user specific one cannot be made.

Doom Legacy will search for WADs in predefined directories before it looks in the current run directory.
For the legacy.wad in LEGACYWADDIR.
For the game IWAD and other wad files in DEFWADS01 to DEFWADS20.
In the DEFWADS strings, the ~ is the home directory, as
implemented by Doom Legacy (Windows and Dos too).
Doom Legacy will search sub-directories too.
::

  GAME_SEARCH_DEPTH   4
  IWAD_SEARCH_DEPTH   5

  Linux, FreeBSD, and Unix:
  The binary can also be installed in "/usr/local/bin".
  DEFHOME    "legacyhome"
  DEFAULTDIR1 ".doomlegacy"
  DEFAULTDIR2 ".legacy"
  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
  DEFWADS01  "~/games/doomlegacy/wads"
  DEFWADS02  "~/games/doomwads"
  DEFWADS03  "~/games/doom"
  DEFWADS04  "/usr/local/share/games/doomlegacy/wads"
  DEFWADS05  "/usr/local/share/games/doomwads"
  DEFWADS06  "/usr/local/share/games/doom"
  DEFWADS07  "/usr/local/games/doomlegacy/wads"
  DEFWADS08  "/usr/local/games/doomwads"
  DEFWADS09  "/usr/share/games/doom"
  DEFWADS10  "/usr/share/games/doomlegacy/wads"
  DEFWADS11  "/usr/share/games/doomwads"
  DEFWADS12  "/usr/games/doomlegacy/wads"
  DEFWADS13  "/usr/games/doomwads"
  DEFWADS16  "~/games/doomlegacy"
  DEFWADS17  "/usr/local/share/games/doomlegacy"
  DEFWADS18  "/usr/local/games/doomlegacy"
  DEFWADS19  "/usr/share/games/doomlegacy"
  DEFWADS20  "/usr/games/doomlegacy"

  Windows:
  DEFHOME   "\legacyhome"
  DEFAULTDIR1 "doomlegacy"
  DEFAULTDIR2 "legacy"
  DEFWADS01  "~\games\doom"
  DEFWADS02  "~\games\doomwads"
  DEFWADS03  "~\games\doomlegacy\wads"
  DEFWADS04  "\doomwads"
  DEFWADS05  "\games\doomwads"
  DEFWADS06  "\games\doom"
  DEFWADS10  "\Program Files\doomlegacy\wads"

  Dos:
  DEFHOME    "DL_HOME"
  DEFAULTDIR1 "dmlegacy"
  DEFAULTDIR2 "legacy"
  DEFWADS01  "~\games\doom"
  DEFWADS02  "~\games\doomwads"
  DEFWADS03  "~\games\doomlegacy\wads"
  DEFWADS04  "\doomwads"
  DEFWADS05  "\games\doomwads"
  DEFWADS06  "\games\doom"

  Mac:
  DEFHOME   "/usr/local/games/legacyhome"
  DEFAULTDIR1 ".doomlegacy"
  DEFAULTDIR2 ".legacy"
  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
  DEFWADS01  "~/games/doomlegacy/wads"
  DEFWADS02  "~/games/doomwads"
  DEFWADS03  "~/games/doom"
  DEFWADS04  "/usr/local/share/games/doomlegacy/wads"
  DEFWADS05  "/usr/local/share/games/doomwads"
  DEFWADS06  "/usr/local/share/games/doom"
  DEFWADS07  "/usr/local/games/doomlegacy/wads"
  DEFWADS08  "/usr/local/games/doomwads"
  DEFWADS09  "/usr/share/games/doom"
  DEFWADS10  "/usr/share/games/doomlegacy/wads"
  DEFWADS11  "/usr/share/games/doomwads"
  DEFWADS12  "/usr/games/doomlegacy/wads"
  DEFWADS13  "/usr/games/doomwads"
  DEFWADS16  "~/games/doomlegacy"
  DEFWADS17  "/usr/local/share/games/doomlegacy"
  DEFWADS18  "/usr/local/games/doomlegacy"
  DEFWADS19  "/usr/share/games/doomlegacy"
  DEFWADS20  "/usr/games/doomlegacy"

  Others:
  DEFHOME   "/usr/local/games/legacyhome"
  DEFAULTDIR1 "doomlegacy"
  DEFAULTDIR2 "legacy"
  LEGACYWADDIR  "/usr/local/share/games/doomlegacy"
  DEFWADS01  "~/games/doomlegacy/wads"
  DEFWADS02  "~/games/doomwads"
  DEFWADS03  "~/games/doom"
  DEFWADS04  "/usr/local/share/games/doomlegacy/wads"
  DEFWADS05  "/usr/local/share/games/doomwads"
  DEFWADS06  "/usr/local/share/games/doom"
  DEFWADS07  "/usr/local/games/doomlegacy/wads"
  DEFWADS08  "/usr/local/games/doomwads"
  DEFWADS09  "/usr/share/games/doom"
  DEFWADS10  "/usr/share/games/doomlegacy/wads"
  DEFWADS11  "/usr/share/games/doomwads"
  DEFWADS12  "/usr/games/doomlegacy/wads"
  DEFWADS13  "/usr/games/doomwads"
  DEFWADS16  "~/games/doomlegacy"
  DEFWADS17  "/usr/local/share/games/doomlegacy"
  DEFWADS18  "/usr/local/games/doomlegacy"
  DEFWADS19  "/usr/share/games/doomlegacy"
  DEFWADS20  "/usr/games/doomlegacy"




Versions other than SDL
-----------------------

There are some options to compile a version of Doom Legacy for other display systems.

Linux X11-windows native (tested, have binaries)
  - requires X11 (such as X11R6), the usual Linux window system that is included with every
    Linux package (only tiny Linux systems running standalone would be without this).

FreeBSD X11-windows native (tested by at least one user)
  - similar to Linux X11 but has some slight library differences.

Linux GGI (old and not tested lately)
  - requires GGI libraries

Unixware, and Openserver5 versions (untested lately, usability is unknown)
  - has different music servers

Windows Direct-X native (may or may not work depending upon your header files)
  - requires Direct-X 7 (at least).
  - with or without FMOD

Mac SDL (code exists, is not working, needs a tester).

Macos native (old and not tested lately).

Os2 native (old and not tested lately).

DOS native (old and not tested lately).
