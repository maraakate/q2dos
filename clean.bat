@echo off
del /s /q *.d
del /s /q *.o
del /s /q *.obj
del /s /q *.ncb
del /s /q *.plg
del /s /q *.suo
del /s /q *.user
del /s /q *.aps
del /s /q *.idb
del /s /q *.pdb
del /s /q *.res
del /s /q gam*.dxe
del /s /q gam*.dll
del /s /q ref*.dxe
del /s /q ref*.dll
del /q q2*.exe
del /q q2*.dbg

cd msvc4-normal
rmdir /s /q Release
rmdir /s /q Debug
rmdir /s /q Dedicated_Release
rmdir /s /q Dedicated_Debug

cd 3zb2
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd ace
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd action
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd chaos
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd coop
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd ctf
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd dday
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd Rogue
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd tsmod
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd Xatrix
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd Zaero
rmdir /s /q Release
rmdir /s /q Debug
cd ..

cd ..
