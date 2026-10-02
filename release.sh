#!/bin/sh

PKG_NAME=q2dos_`date -u +%Y%m%d`.7z
rm -f $PKG_NAME

./clean.sh
./makeall.sh $*

. ./cross_defs.dj

#echo $TARGET-strip q2.exe
#$TARGET-strip q2.exe || exit 1
objcopy --only-keep-debug q2.exe q2.dbg
objcopy --strip-debug --strip-unneeded q2.exe

mkdir -p _build || exit 1

cp -p q2.exe _build
cp -p q2.dbg _build
cp -p ref_soft/ref_soft.dxe _build
cp -p ref_gl/ref_gl.dxe _build
cp -pr dos/3rdparty/lib_dxe/* _build
cp -p CWSDPMI.EXE _build
cp -p readme.txt _build
cp -p readme.fx _build

mkdir -p _build/baseq2
cp -p game/gamex86.dxe _build/baseq2
mkdir -p _build/ctf
cp -p ctf/gamex86.dxe _build/ctf
mkdir -p _build/xatrix
cp -p xsrc/gamex86.dxe _build/xatrix
mkdir -p _build/rogue
cp -p rsrc/gamex86.dxe _build/rogue
mkdir -p _build/3zb2
cp -p 3zb2/gamex86.dxe _build/3zb2
cp -p 3zb2/3ZB.cfg _build/3zb2
mkdir -p _build/ace
cp -p acebot/gamex86.dxe _build/ace
mkdir -p _build/action
cp -p action/gamex86.dxe _build/action
mkdir -p _build/chaos
cp -p chaos/gamex86.dxe _build/chaos
mkdir -p _build/dday
cp -p dday/gamex86.dxe _build/dday
mkdir -p _build/zaero
cp -p zaero/gamex86.dxe _build/zaero

# gamespy shit
cp -p Goa/gamespy.dxe _build

# copy a default gl.dxe
cp -p dos/3rdparty/lib_dxe/opengl/fxmesa/gl.dxe _build

cd _build || exit 1
unix2dos -k readme.txt
unix2dos -k readme.fx
find * -type d|xargs chmod 755
find * -type f|xargs chmod 644
7z a ../$PKG_NAME *
