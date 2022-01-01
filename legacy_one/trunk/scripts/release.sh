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

binary_spec_name() {
  case $1 in
  "Linux_SDL_32"   ) bsn="linux2.6_32_sdl" ;;
  "Linux_SDL_64"   ) bsn="linux2.6_64_sdl" ;;
  "Windows_SDL_32" ) bsn="windows_32_sdl" ;;
  "Windows_SDL_64" ) bsn="windows_64_sdl" ;;
  "Linux_X11_32"   ) bsn="linux2.6_32_x11" ;;
  * ) bsn="" ;;
  esac
  echo $bsn
}

# binary architecture and platform
get_binary_spec2() {
  bsn=""
  if [ "$2" != "" ]; then
    bsn=$(binary_spec_name $2)
  fi
  if [ "$bsn" == "" ]; then
    select  bspec2 in $valid_binary_spec ; do break; done
    bsn=$(binary_spec_name $bspec2)
  fi

  # Any echo becomes part of return value
  echo $bsn
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


if [ -e "bin" ]; then
bin="bin"
else
bin="$srcdir/bin"
fi
if [ ! -e "$bin" ]; then
  echo "Error: $bin not found"
  exit
fi

if [ -e "spec" ]; then
specfile="spec"
else if [ -e "spec.txt" ]; then
specfile="spec.txt"
fi
fi


wadtool="$bin/wadtool"                      # prebuilt wadtool executable, for updating legacy.wad
exefile="$bin/doomlegacy"                   # prebuilt Doom Legacy executable to package


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
        binary_spec=$( get_binary_spec2 "" "$2" )

	echo "Building a binary package for "$binary_spec

	# prepare the README file
	echo "Copy README"
	sed -e "s/\[DATE\]/$releasedate/" -e "s/\[VERSION\]/$version/" -e "s/\[WADVERSION\]/$wadversion/" -e "s/\[SVNREV\]/$svnrev/" <$srcdir/scripts/$readmefile >$tempdir/$readmefile
        if [ -e "$specfile" ]; then
	    echo "Copy SPECFILE"
            cp -p $specfile  $tempdir
	fi
	cp -a $exefile $tempdir
	echo "Make TAR"
	tar -cjf $releasedir/$prefix"_"$binary_spec".tar.bz2" $tempdir
	echo "Copy to $releasedir"
	#zip -r $releasedir/$prefix"_"$binary_spec".zip" $tempdir
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
        ;;

    *)
        echo $"Usage: $0 {source|legacywad|common|binary|upload|docs|clean}"
        echo $"       $0 binary Linux_SDL_32"
        exit 1
esac
echo "Done."
exit
