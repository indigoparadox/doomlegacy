#! /bin/bash
# Bash script for building Doom Legacy releases and uploading them to SourceForge.net
# Copyright (C) 2014-2020 by Doom Legacy Team.

echo "Doom Legacy Release tool: 1.48"

#========================================================
# Set these variables first.
# Make links to srcdir, bin, legacy.wad.

username=""                                 # sourceforge.net username of the person doing the upload
workdir="."                                 # working directory, where all the packages are built
readmefile="README.rst"                     # name of the README file to be included in every binary release package


# Space separated list of valid binary package names
valid_binary_spec="Linux_SDL_32  Linux_SDL_64  Windows_SDL_32  Windows_SDL_64  Linux_X11_32"

# Look for links to the binary locations.
#  lin32, lin64, win32, win64, x11_32

# sets: packname, portdir, bin, lib, binfiles, libfiles
function package_spec_name() {
  unset libfiles
  unset binfiles
  case $1 in
  "Linux_SDL_32"   )
     packname="linux2.6_32_sdl"
     portdir=lin32
     # prebuilt Doom Legacy executable
     if [ -e lin32/doomlegacy ]; then
         bin=lin32
     else if [ -e lin32/bin ]; then
         bin=lin32/bin
     fi fi
     # prebuilt Doom Legacy executable
     binfiles="$bin/doomlegacy"
     ;;

  "Linux_SDL_64"   )
     packname="linux2.6_64_sdl"
     portdir=lin64
     if [ -e lin64/doomlegacy ]; then
         bin=lin64
     else if [ -e lin64/bin ]; then
         bin=lin64/bin
     fi fi
     # prebuilt Doom Legacy executable
     binfiles="$bin/doomlegacy"
     ;;

  "Windows_SDL_32" )
     packname="windows_32_sdl"
     portdir=win32
     if [ -e win32/doomlegacy.exe ]; then
         bin=win32
     else if [ -e win32/bin ]; then
         bin=win32/bin
     fi fi
     # prebuilt Doom Legacy executable
     binfiles="$bin/doomlegacy.exe"
     if [ -e win32/SDL.dll ]; then
         lib=win32
     else if [ -e win32/lib ]; then
         lib=win32/lib
     fi fi
     libfiles="$lib/SDL.dll $lib/SDL_mixer.dll"
     ;;

  "Windows_SDL_64" )
     packname="windows_64_sdl"
     portdir=win64
     if [ -e win64/doomlegacy.exe ]; then
         bin=win64
     else if [ -e win64/bin ]; then
         bin=win64/bin
     fi fi
     # prebuilt Doom Legacy executable
     binfiles="$bin/doomlegacy.exe"
     if [ -e win64/SDL.dll ]; then
         lib=win64
     else if [ -e win64/lib ]; then
         lib=win64/lib
     fi fi
     libfiles="$lib/SDL.dll $lib/SDL_mixer.dll"
     ;;

  "Linux_X11_32" )
     packname="linux2.6_32_x11"
     portdir=x11_32
     if [ -e x11_32/llxdoom ]; then
         bin=x11_32
     else if [ -e x11_32/bin ]; then
         bin=x11_32/bin
     fi fi
     # prebuilt Doom Legacy executable, libraries
     binfiles="$bin/llxdoom"
     if [ -e $bin/llsndserv ]; then
         binfiles=$binfiles "$bin/llsndserv" 
     fi
     if [ -e $bin/musserver ]; then
         binfiles=$binfiles "$bin/musserver" 
     fi
     libfiles="$bin/r_opengl.so"
     ;;

  * ) packname="" ;;
  esac
}

# binary architecture and platform
# sets: packname, portdir, bin, lib, binfiles, libfiles
function get_binary_spec2() {
  packname=""
  if [ "$2" ]; then
      package_spec_name  $2
  fi

  if [ "$packname" == "" ]; then
    select  bspec2 in $valid_binary_spec ; do break; done
    package_spec_name  $bspec2
  fi

  # Suggest using portdir
  if [ "$portdir" ]; then
      if [ ! -e $portdir ]; then
         echo "Suggest put binary, lib, and spec in $portdir."
	 echo "Or can make $portdir a link to such a directory."
	 mkdir $portdir
      fi
  fi
}



if [ -e "srcdir" ]; then
srcdir="srcdir"				    # indirection to trunk
else
srcdir="/home/doomlegacy/legacy_one/trunk"  # Doom Legacy source tree location
fi
if [ ! -e "$srcdir" ]; then
  echo "Error: $srcdir not found"
  exit
fi



if [ -e "spec" ]; then
specfile="spec"
else if [ -e "spec.txt" ]; then
specfile="spec.txt"
fi
fi


# read the Doom Legacy version number from the source code
ver=$(sed -n -e "s/^const int  VERSION  = \([0-9]\)\([0-9]*\).*$/\1.\2/p" $srcdir/src/d_main.c)
rev=$(sed -n -e "s/^const int  REVISION = \([0-9]*\).*$/\1/p" $srcdir/src/d_main.c)
version=$ver.$rev

# read legacy.wad version from the VERSION lump
wadversion=$(sed -n -e "s/^Doom Legacy WAD V\([0-9].[0-9]*\).*$/\1/p" $srcdir/resources/VERSION.txt)

# SVN revision
svnrev=$(svn info $srcdir | sed -n -e "s/^Revision: \([0-9]*\)/\1/p")

# today's date
releasedate=$(date --rfc-3339='date')

# prefix for the package names
prefix="doomlegacy_"$version

# temporary packaging directory
tempdir=$prefix

# directory where all the packages are collected for upload
releasedir=$version


#========================================================

echo Doom Legacy $version, svn$svnrev

# make the release files directory 
mkdir -p $workdir
cd $workdir
mkdir -p $releasedir
mkdir -p $tempdir

case "$1" in
    source)
	echo "Building the source package from src tree at " $(realpath $srcdir)

	srcname=$prefix"_source"
	# temporary packaging directory
	srcpackdir=$tempdir"_source"
	mkdir -p $srcpackdir
	# copy the source tree, remove build cruft
	cp -ar $srcdir/* $srcpackdir
	# remove any bin objs dep
	rm -f $srcpackdir/bin/* $srcpackdir/objs/* $srcpackdir/dep/*

	# into a tar package
	tar -cjf $releasedir/$srcname".tar.bz2" $srcpackdir
	;;

    legacywad)
        if [ -e "legacy.wad" ]; then
            legacywadfile="legacy.wad"                  # legacy.wad or link to location.
        else
            legacywadfile="$srcdir/bin/legacy.wad"      # prebuilt legacy.wad location.
        fi
        if [ ! -e "$legacywadfile" ]; then
	    read -p "legacy.wad file: " legacywadfile
	fi
        if [ ! -e "$legacywadfile" ]; then
            echo "Error: $legacywadfile not found"
            exit
	fi
        # find prebuilt wadtool executable, for updating legacy.wad
        wadtool="wadtool"
	if [ ! -e $wadtool ]; then
            if [ -e "bin" ]; then
                bin="bin"
                wadtool="$bin/wadtool"
	    fi
        fi
	if [ ! -e $wadtool ]; then
	    echo "Must first build wadtool: $wadtool"
	    exit 1
	fi
	# need absolute paths
	legacywad_src=$(realpath $legacywadfile)
	legacywad_dest=$(realpath "legacy.wad" )
	wadtool=$(realpath $wadtool)
	srcdir=$(realpath $srcdir)
    
	echo "Building the legacy.wad from "$legacywadfile" to "$destdir

	# Break legacy.wad into lumps, update the lumps, then rebuild the WAD
	echo "Building updated legacy.wad, version "$wadversion
	waddir=$tempdir"_wad"
	mkdir -p $waddir
	cp -a  $legacywadfile $waddir
	cd $waddir
	$wadtool -x $legacywad_src
	cp -a $srcdir/resources/* .
# Warning: wadtool can not handle the marker lumps.	
	$wadtool -c $legacywad_dest legacy.wad.inventory
	cd ..
        ;;

    common)
        if [ -e "legacy.wad" ]; then
            legacywadfile="legacy.wad"                  # legacy.wad or link to location.
        else
            legacywadfile="$srcdir/bin/legacy.wad"      # prebuilt legacy.wad location.
        fi
        if [ ! -e "$legacywadfile" ]; then
	    read -p "legacy.wad file: " legacywadfile
	fi
        if [ ! -e "$legacywadfile" ]; then
            echo "Error: $legacywadfile not found"
            exit
	fi
    
	echo "Building the common package, legacy.wad at "$legacywadfile

	# common dir must NOT be absolute, or else your whole absolute directory structure
	# will be recorded in the zip file.
	comdir=$prefix"_common"
	mkdir -p  $comdir
	# Remove any old contents
	rm -f $comdir/docs
	rm -f $comdir/*.wad

        # wad files
        cp -p  $legacywadfile  $comdir
        cp -p  $srcdir/resources/dogs.wad  $comdir

	# add the documentation
	cp -ar $srcdir/docs  $comdir

	# into a zip package
	zipfile=$releasedir/$prefix"_common.zip"
	rm -r $zipfile
	zip -r $zipfile  $comdir
        ;;

    binary)
        get_binary_spec2 "" "$2"

	echo "Building a binary package for "$packname

	# prepare the README file
	echo "Copy README"
	sed -e "s/\[DATE\]/$releasedate/" -e "s/\[VERSION\]/$version/" -e "s/\[WADVERSION\]/$wadversion/" -e "s/\[SVNREV\]/$svnrev/" <$srcdir/scripts/$readmefile >$tempdir/$readmefile
	# Copy binary
	echo "Copy $binfiles"
	cp -p $binfiles $tempdir
	# Copy libraries
	if [ "$libfiles" ]; then
	    # One or more library files.
            echo "Copy $libfiles"
            cp -p $libfiles  $tempdir
	fi
	# Copy spec.txt
        if [ -e $portdir/spec.txt ]; then
            specfile=$portdir"/spec.txt"
        else if [ -e $portdir"_spec.txt" ]; then
            specfile=lin32_spec.txt
        fi fi
	if [ "$specfile" ]; then
            if [ -e "$specfile" ]; then
                echo "Copy $specfile"
                cp -p $specfile  $tempdir
	    fi
	fi
	echo "Make TAR"
	tar -cjf $releasedir/$prefix"_"$packname".tar.bz2" $tempdir
	echo "Copy to $releasedir"
	#zip -r $releasedir/$prefix"_"$packname".zip" $tempdir
	# put a copy of the README file in the release directory
	cp -a $tempdir/$readmefile $releasedir
	;;
         
    upload)
        if [ "$username" == "" ]; then
	  read -p "SourceForge username: " username
	fi
	# Upload all the built release packages to Sourceforge.net file release system.
	# note the trailing slash after the source dir, which means "copy the contents of the directory".
	rsync -aiv -e ssh $releasedir/ $username@frs.sourceforge.net:/home/frs/project/doomlegacy/$version
        ;;

    upload_docs)
        if [ "$username" == "" ]; then
	  read -p "SourceForge username: " username
	fi
	# Upload the latest version of the docs to the website.
	# Note the trailing slash after the source dir, which means "copy the contents of the directory".
	rsync -aiv -e ssh $srcdir/docs/ $username,doomlegacy@web.sourceforge.net:htdocs/docs
        ;;

    clean)
	# Clean up (delete) the auxiliary directories.
	rm -r $tempdir
	rm -r $tempdir"_source"
	rm -r $tempdir"_wad"
	rm -r $tempdir"_common"
        ;;

    *)
        echo $"Usage: $0 {source|legacywad|common|binary|upload|docs|clean}"
        echo $"       $0 binary Linux_SDL_32"
        echo "Make port directories with port specific files (or links)"
	echo "  lin32, lin64, win32, win64, x11_32"
        exit 1
esac
echo "Done."
exit
