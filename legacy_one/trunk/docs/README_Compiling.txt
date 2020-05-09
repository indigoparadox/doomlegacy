Title: Compiling Doom Legacy 1.4x
Author: Wesley Johnson
Date: 2020-5-8

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


You will require the following programs during the build process:

Compiler:
  GCC 3.3+, the Gnu Compiler Collection which, among other things, is
  a free C/C++ compiler. "http://gcc.gnu.org/"
  Linux systems most likely already have it installed.</li>
  
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

* Select the make_options_xx for your operation system, and copy it
to make_options.

The make_options_xx will contain appropriate selections for your
operating system.  Copying options from a make_options file for a different
operating system probably will not work.

* Lines that start with # are comments.  To turn off an option, put
a # at the beginning of that line. Lines without the # are active
selections.

* MAKE_OPTIONS_PRESENT: indication that the make_options file has
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

For a modern x86 process, selecting "ARCH=-march=i686" will compile
a binary for an x686, which will be smaller and faster than an x386
binary. The binary will run on any x686 compatible processor.

If you have a specific processor, like an Athlon, then specifying
that will have the compiler use specific optimizations for the Athlon.
The resultant binary will run faster on an Athlon.  It may run on
other processors too, but in some cases this is not guaranteed.

* CD_MUSIC: if you don't have the music libraries, then turn off
music by setting CD_MUSIC=0.  The build will not have music.

* USEASM: generate assembly.  This used to be faster code, but
modern compilers will now beat this.  The assembly is not updated as
often enough, so it may not be current.  Avoid this unless you like pain.

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
identifies the variation, such as "dl_i686_athlon_v3".

You must have system permissions for the type of install.
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

2. Select and configure your make_options file.

3. The top level TRUNK is the default BUILD directory.
The make_options file there will control the build.

4. Copy one of the make_options_xx files to make_options, and edit it.

5. If you want a separate BUILD directory, then create it.
Create bin, dep, objs directories there.
Create a make_options file in the directory by copying one of the make_options files,
and editing it for your selected build.
>> make BUILD=x11 dirs

6. Edit compile options in doomdef.c.

7. To clean the build:
>> make clean

8. To build the executable:
>> make

9. To build a debug version:
>> make DEBUG=1

10. To build with bin, obj, dep, in a separate directory (example x11):
>> make BUILD=x11

11. Example:
>> make BUILD=x11d clean

12. Example:
>> make BUILD=x11d DEBUG=1 MIXER=1

13. Example MinGW:
>> mingw32-make DEBUG=1 clean

14. Read the Makefile for the help at the top of the file.
This will document the latest commands.


See docs/source.html for more details.

