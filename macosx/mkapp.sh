#!/bin/sh

# generate directory structure for an app bundle
mkdir -p Quake2.app/Contents/MacOS
mkdir -p Quake2.app/Contents/Frameworks
mkdir -p Quake2.app/Contents/Resources

# package info
cp -p PkgInfo Quake2.app/Contents
cp -p Info.plist Quake2.app/Contents
# icon resources
cp -p quake2.icns Quake2.app/Contents/Resources
# needed frameworks
cp -pr SDL.framework Quake2.app/Contents/Frameworks
# the binary
cp -p ../quake2 Quake2.app/Contents/MacOS
# ogg/vorbis decoder
cp -p lib/libogg.dylib Quake2.app/Contents/MacOS
cp -p lib/libvorbisfile.dylib Quake2.app/Contents/MacOS
cp -p lib/libvorbis.dylib Quake2.app/Contents/MacOS
