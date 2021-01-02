Title: Compiling Doom Legacy 1.4x
Author: Wesley Johnson
Date: 2020-12-20

In order to compile Doom Legacy 1.4x SDL you'll need to have the following
libraries installed on your system.
Make sure you install the developer packages of the libraries,
which include both the runtime libraries (DLLs) and
the stuff required during compilation (header files and import libraries).

NOTE: Most Linux distributions offer these libraries in the form of
precompiled packages.

SDL:
  Simple DirectMedia Layer. A multiplatform multimedia library.
  Version 1.2.10+
  "http://www.libsdl.org/download-1.2.php"
  Ubuntu: libsdl1.2-dev "http://packages.ubuntu.com/libsdl1.2-dev"

SDL Mixer:
  A multichannel mixer library based for SDL.
  Version 1.2.7+
  "http://www.libsdl.org/projects/SDL_mixer/"
  Ubuntu: libsdl-mixer1.2-dev  "http://packages.ubuntu.com/libsdl-mixer1.2-dev"
  
OpenGL
  The standard cross-platform graphics library, usually comes with the OS.
  There may be specific versions for specific video cards.
  OpenGL 1.3+

libzip
  Optional, for zip archive reading.  Linux Only.
  Set HAVE_LIBZIP in make_options file.
  Set HAVE_LIBZIP=12 if you have libzip 1.2 or later, which will
  use the libzip zip_seek and will disable the local zip_seek function.
  Enable zip archive reading by setting ZIPWAD in doomdef.h.

dlopen
  Optional, for ziplib detection.  Linux Only.
  Set HAVE_DLOPEN in make_options file.
  Enable libzip detection by setting ZIPWAD_OPTIONAL in doomdef.h.


You will require the following programs during the build process:

Compiler:
  GCC 3.3+, the Gnu Compiler Collection which, among other things, is
  a free C/C++ compiler. "http://gcc.gnu.org/"
  Linux systems most likely already have it installed.
  
  Has been compiled on Linux with Gnu 5.5.0.
  Has been compiled on Linux with Clang 3.8.0
  
  Windows users can install MinGW a GCC port.
  Windows users can install MSYS, which provides unix commands, and POSIX utilities for Win32.
  MinGW: "http://www.mingw.org/", "http://www.mingw.org/node/18"

  Has been compiled on Windows XP with Mingw-32 5.0.2, and MSYS 1.0.11.
    Use the command "mingw32-make".
    Using the default "make" command with MSYS invokes something else that does not work.


Download the DoomLegacy source.
  "http://sourceforge.net/projects/doomlegacy/"

You can either get the source package or, for the latest code snapshot.
Checkout the legacy_one/trunk/ directory from the Subversion repository at SourceForge:
Subversion: "http://subversion.apache.org/">Subversion
SourceForge: svn co https://doomlegacy.svn.sourceforge.net/svnroot/doomlegacy/legacy_one/trunk some_local_dir


From now on, your local copy of this directory will be referred to as
TRUNK, although you can name it anything you want.

You can have multiple versions of the 'src' directory, such as d01,
d02, d03, etc..

To compile these, cd to the src directory you wish to compile, and run
'make' from there.  The src Makefile will find the BUILD directory.


Chapter: Make Options

The make_options file controls the make process.

Edit it to select various compiling options.  Spelling of options must
be exactly one of the specified choices.


* The "make_options" file must be created by you.  It is customized to your
preferences.

* Select the make_options_xx for your operating system, and copy it
to make_options.
Linux Example:
>> cp  make_options_nix  make_options

The make_options_xx will contain appropriate selections for your
operating system.  Copying options from a make_options file for a different
operating system probably will not work.

* Lines that start with # are comments.  To turn off an option, put
a # at the beginning of that line. Lines without the # are active
selections.

* MAKE_OPTIONS_PRESENT: this informs 'make' that the make_options file has
been read. Leave it alone.

* OS: the operating system, such as LINUX, FREEBSD.

* SMIF: the port draw library, such as SDL, or X11.

* ARCH: the cpu architecture.
You can build a binary customized to your processor.

The examples work for most processors.
It does not need to be exact, as processors will run binaries compiled for any
older CPU versions of their line.

See your compiler docs for the -march, -mcpu, and -mtune command line
switches specifics. Without this line the compiler will select its
default build target, such a generic32, or i386.

For a modern x86 processor, selecting "ARCH=-march=i686" will compile
a binary for an x686, which will be smaller and faster than an x386
binary.  This binary will run on any x686 compatible processor.

If you have a specific processor, like an Athlon, then specifying
that will have the compiler use specific optimizations for the Athlon.
The resultant binary will run faster on an Athlon.  It may run on
other processors too, but in some cases this is not guaranteed.

* CD_MUSIC: if you don't have the music libraries, then turn off
music by setting CD_MUSIC=0.  The build will not have music.

* USEASM: use assembly code for certain operations.
This used to be slightly faster code, but modern compilers will now beat this.
The assembly is not updated often enough, so it may not be current.
Avoid this unless you like pain.

* There are several install options for DoomLegacy.  Each can be
customized.  See the next section.


Chapter: Install Options

The Makefile has options for installing the program.  This is
traditional for Linux makefiles.  It is espcially useful when
compiling your own binary.

You can use any other kind of installation too, such as copying the
bin files to a "run" directory.  Do not try to run DoomLegacy from the
compile environment, as the run setup is quite different.

You can have several doomlegacy versions in your run directory.
They may have any name you choose.  I suggest making a name that
identifies the variation, such as "doomlegacy_i686_athlon_v3".

You must have system permissions for system install.
An ordinary user can only install_user.

>> make install
  Provides install instructions.

The following are supported in "make_options":

>> make install_sys
  Install doomlegacy to the system.
  INSTALL_SYS_DIR: the system directory where the binary will be installed.
  INSTALL_SHARE_DIR: the system directory where the support files will be installed.
  
>> make install_games
  Install doomlegacy to the system games.
  INSTALL_GAMES_DIR: the system directory where the binary will be installed.
  INSTALL_SHARE_DIR: the system directory where the support files will be installed.

>> make install_user
  Install doomlegacy to a user directory.
  INSTALL_USER_DIR: the user directory where the binary and suport files will be installed.
  INSTALL_LEGACYWAD_DIR: the directory where the support files will be installed.


Chapter: Compiling Legacy

1. Open a shell window, like console. Go to the TRUNK directory.

2. If you want a separate BUILD directory, then create it.
Create bin, dep, objs directories there.
The BUILD directory will need its own make_options file (see step 4).
This allows you to compile SDL in one directory, and X11 in another.
>> make BUILD=x11 dirs

3. The top level TRUNK is the default BUILD directory.
The make_options file there will control the build, when BUILD is not
specified.

4. Select and configure your make_options file.
Copy one of the make_options_xx files to make_options of your BUILD
directory.  Edit your make_options to set your configure options.
>> cp  make_options_nix  x11/make_options

5. Edit compile options in doomdef.h.
These are options within DoomLegacy.
Some features can be disabled to make a smaller binary.
There are also some experimental code options, that are kept disabled.

6. To clean the build:
When you have been copying files, or have modified make_options,
you need to start from a clean build.  This removes old object files.
>> make clean

With a BUILD directory:
>> make BUILD=x11 clean

7. To build the executable:
>> make

To build with bin, obj, dep, in a separate directory (example x11):
>> make BUILD=x11

8. To build a debug version:
This puts the debugging symbols for the debugger in the binary.
The debug version also forces DoomLegacy video to an 800x600 window,
so the user can switch between the debugger window and the DoomLegacy window.
>> make DEBUG=1

With a BUILD directory:
>> make BUILD=x11 DEBUG=1


10. Example: X11 DEBUG
>> make BUILD=x11d clean
>> make BUILD=x11d DEBUG=1 MIXER=1

Debugging with X11 can be difficult.
Keep your game window and debugging window separated.
Be prepared to switch to another console and kill the game.
Print information to a log file rather than try to look at it
with a debugger.

11. Example: MinGW
Must use mingw32-make, not the make command from MSYS.
>> mingw32-make DEBUG=1 clean

12. Read the Makefile for the help at the top of the file.
This will document the latest commands.


See docs/source.html for more details.
