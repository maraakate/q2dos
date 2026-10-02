@echo off
call clean.bat
call setenv.bat

make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
objcopy --only-keep-debug q2.exe q2.dbg
objcopy --strip-debug --strip-unneeded q2.exe
REM upx -9 q2.exe

cd ref_soft
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd ref_gl
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd Goa
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd game
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd ctf
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd 3zb2
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd acebot
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd action
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd chaos
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd dday
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd rsrc
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd xsrc
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
cd ..

cd zaero
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj clean
make %1 %2 %3 %4 %5 %6 %7 %8 %9 -f Makefile.dj
