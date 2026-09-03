@echo off
set Q2DOSBINDIR=Debug

if not defined Q2DOSDEVBASE ( goto failed )

call "%Q2DOSDEVBASE%\clean.bat"
call vcvars32.bat
call "%VS80COMNTOOLS%vsvars32.bat"

cd /D "%Q2DOSDEVBASE%\msvc4-normal"

:startcopy
devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "q2ded"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "gamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "ref_soft"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "ref_gl"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "quake2"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "gamespysdk"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "3zb2gamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "acegamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "actiongamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "chaosgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "coopgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "ctfgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "ddaygamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "roguegamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "xatrixgamex86"

devenv "%Q2DOSDEVBASE%\msvc4-normal\quake2.sln" /Build "%Q2DOSBINDIR%|x64" /Project "zaerogamex86"

CALL mvd.bat
goto end

:failed
echo Q2DOSDEVBASE not defined!
pause
goto end

:end
set Q2DOSBINDIR=
