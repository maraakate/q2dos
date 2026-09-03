# Microsoft Developer Studio Project File - Name="coopgamex86" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=coopgamex86 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coopgamex86.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coopgamex86.mak" CFG="coopgamex86 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coopgamex86 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "coopgamex86 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coopgamex86 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\coop\Release"
# PROP Intermediate_Dir ".\coop\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D id386=1 /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\coop\Release/gamex86.dll"

!ELSEIF  "$(CFG)" == "coopgamex86 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\coop\Debug"
# PROP Intermediate_Dir ".\coop\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D id386=1 /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\coop\Debug/gamex86.dll"

!ENDIF 

# Begin Target

# Name "coopgamex86 - Win32 Release"
# Name "coopgamex86 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\coop\b_cam.c
# End Source File
# Begin Source File

SOURCE=..\coop\dm_ball.c
# End Source File
# Begin Source File

SOURCE=..\coop\dm_tag.c
# End Source File
# Begin Source File

SOURCE=..\coop\flashlight.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_ai.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_chase.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_cmds.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_combat.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_coop.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_func.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_items.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_main.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_misc.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_monster.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newai.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newdm.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newfnc.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newtarg.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newtrig.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_newweap.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_phys.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_save.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_spawn.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_sphere.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_target.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_trigger.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_turret.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_utils.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_vote.c
# End Source File
# Begin Source File

SOURCE=..\coop\g_weapon.c
# End Source File
# Begin Source File

SOURCE=..\coop\game.def
# End Source File
# Begin Source File

SOURCE=..\coop\m_actor.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_berserk.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss2.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss3.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss31.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss32.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss5.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_brain.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_carrier.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_chick.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_fixbot.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_flash.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_flipper.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_float.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_flyer.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_gekk.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_gladb.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_gunner.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_hover.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_infantry.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_insane.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_medic.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_move.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_mutant.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_parasite.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_soldier.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_stalker.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_supertank.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_tank.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_turret.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_widow.c
# End Source File
# Begin Source File

SOURCE=..\coop\m_widow2.c
# End Source File
# Begin Source File

SOURCE=..\coop\menu.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_client.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_hook.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_hud.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_trail.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_view.c
# End Source File
# Begin Source File

SOURCE=..\coop\p_weapon.c
# End Source File
# Begin Source File

SOURCE=..\coop\q_shared.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_acannon.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_ai.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_anim.h
# End Source File
# Begin Source File

SOURCE=..\coop\z_boss.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_boss.h
# End Source File
# Begin Source File

SOURCE=..\coop\z_camera.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_handler.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_handler.h
# End Source File
# Begin Source File

SOURCE=..\coop\z_hound.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_hound.h
# End Source File
# Begin Source File

SOURCE=..\coop\z_item.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_sentien.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_sentien.h
# End Source File
# Begin Source File

SOURCE=..\coop\z_spawn.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_trigger.c
# End Source File
# Begin Source File

SOURCE=..\coop\z_weapon.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\coop\b_cam.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\clientfields.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\fields.h
# End Source File
# Begin Source File

SOURCE=..\coop\flashlight.h
# End Source File
# Begin Source File

SOURCE=..\coop\g_local.h
# End Source File
# Begin Source File

SOURCE=..\coop\game.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\gamefunc_decs.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\gamefunc_list.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\gamemmove_decs.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\gamemmove_list.h
# End Source File
# Begin Source File

SOURCE=..\coop\tables\levelfields.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_actor.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_berserk.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss2.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss31.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_boss32.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_brain.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_carrier.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_chick.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_fixbot.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_flipper.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_float.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_flyer.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_gekk.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_gunner.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_hover.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_infantry.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_insane.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_medic.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_mutant.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_parasite.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_player.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_rider.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_soldier.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_stalker.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_supertank.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_tank.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_turret.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_widow.h
# End Source File
# Begin Source File

SOURCE=..\coop\m_widow2.h
# End Source File
# Begin Source File

SOURCE=..\coop\menu.h
# End Source File
# Begin Source File

SOURCE=..\coop\q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
