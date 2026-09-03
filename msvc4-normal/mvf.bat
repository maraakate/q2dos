@echo off
set Q2DOSBINDIR=Release

if not defined Q2DOSDIR ( goto failed )

:startcopy
echo Copying %Q2DOSBINDIR% Build.

move /Y %Q2DOSBINDIR%\q2.exe "%Q2DOSDIR%"
move /y Dedicated_%Q2DOSBINDIR%\q2ded.exe "%Q2DOSDIR%"
move /Y %Q2DOSBINDIR%\ref_soft.dll "%Q2DOSDIR%"
move /Y %Q2DOSBINDIR%\ref_gl.dll "%Q2DOSDIR%"
move /Y %Q2DOSBINDIR%\gamespy.dll "%Q2DOSDIR%"
move /Y %Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\baseq2"
move /Y %Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\baseq2"
move /Y 3zb2\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\3zb2"
move /Y 3zb2\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\3zb2"
move /Y ace\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\ace"
move /Y ace\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\ace"
move /Y action\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\action"
move /Y action\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\action"
move /Y chaos\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\chaos"
move /Y chaos\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\chaos"
move /Y coop\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\coop"
move /Y coop\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\coop"
move /Y ctf\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\ctf"
move /Y ctf\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\ctf"
move /Y dday\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\dday"
move /Y dday\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\dday"
move /Y rogue\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\rogue"
move /Y rogue\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\rogue"
move /Y xatrix\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\xatrix"
move /Y xatrix\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\xatrix"
move /Y Zaero\%Q2DOSBINDIR%\gamex86.dll "%Q2DOSDIR%\zaero"
move /Y Zaero\%Q2DOSBINDIR%\gamex64.dll "%Q2DOSDIR%\zaero"

goto end

:failed
echo Q2DOSDIR not defined!
pause
goto end

:end
set Q2DOSBINDIR=
