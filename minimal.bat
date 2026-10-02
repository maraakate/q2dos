@echo off

REM For building a minimal q2.exe
REM All other game or ref_ dxe can be built normally

call clean.bat
call setenv.bat

make -f Makefile.dj clean
make -f Makefile.dj USE_WATT32=0 USE_GAMESPY=0 USE_CURL=0 USE_OGG=0 USE_SNDPCI=0
objcopy --only-keep-debug q2.exe q2.dbg
objcopy --strip-debug --strip-unneeded q2.exe
