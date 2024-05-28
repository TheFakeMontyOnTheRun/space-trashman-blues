# Microsoft Developer Studio Project File - Name="Sub Mare Imperium Derelict" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Sub Mare Imperium Derelict - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sub Mare Imperium Derelict.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sub Mare Imperium Derelict.mak" CFG="Sub Mare Imperium Derelict - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sub Mare Imperium Derelict - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sub Mare Imperium Derelict - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /O2 /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x816 /d "NDEBUG"
# ADD RSC /l 0x816 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "c:\space-trashman-blues\common\include" /I "c:\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\core\include" /I "c:\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /D "CLI_BUILD" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x816 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386

!ENDIF 

# Begin Target

# Name "Sub Mare Imperium Derelict - Win32 Release"
# Name "Sub Mare Imperium Derelict - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\base3d\src\Win32Version\CGDIRenderer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Common.c
# End Source File
# Begin Source File

SOURCE=..\..\..\core\src\Core.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Crawler.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\CreditsScreen.c
# End Source File
# Begin Source File

SOURCE=..\..\base3d\src\CRenderer.c
# End Source File
# Begin Source File

SOURCE=..\..\base3d\src\CRenderer_Rasterization.c
# End Source File
# Begin Source File

SOURCE=..\..\base3d\src\CRenderer_Tesselation.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\CTile3DProperties.c
# End Source File
# Begin Source File

SOURCE=..\..\..\core\src\Derelict.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Dungeon.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\EDirection_Utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Engine.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Events.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\FixP.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\GameMenu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Globals.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\HackingScreen.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\HelpScreen.c
# End Source File
# Begin Source File

SOURCE=..\..\base3d\src\LoadBitmap.c
# End Source File
# Begin Source File

SOURCE=..\..\menu\src\Main.c

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

# ADD CPP /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\MainMenu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\MapWithCharKey.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\PackedFileReader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\core\src\Parser.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\UI.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\Vec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\VisibilityStrategy.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\small.ico
# End Source File
# Begin Source File

SOURCE=".\Sub Mare Imperium.rc"
# End Source File
# Begin Source File

SOURCE=".\The Mistral Report 95.ICO"
# End Source File
# End Group
# Begin Source File

SOURCE=.\bong.wav
# End Source File
# Begin Source File

SOURCE=.\dead.wav
# End Source File
# Begin Source File

SOURCE=.\detected.wav
# End Source File
# Begin Source File

SOURCE=.\detected2.wav
# End Source File
# Begin Source File

SOURCE=.\enemydead.wav
# End Source File
# Begin Source File

SOURCE=.\enemyfire.wav
# End Source File
# Begin Source File

SOURCE=.\fire.wav
# End Source File
# Begin Source File

SOURCE=.\gotclue.wav
# End Source File
# Begin Source File

SOURCE=.\hurt.wav
# End Source File
# Begin Source File

SOURCE=.\menu_move.wav
# End Source File
# Begin Source File

SOURCE=.\menu_select.wav
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
