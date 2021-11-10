# Microsoft Developer Studio Generated NMAKE File, Based on Sub Mare Imperium Derelict.dsp
!IF "$(CFG)" == ""
CFG=Sub Mare Imperium Derelict - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Sub Mare Imperium Derelict - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Sub Mare Imperium Derelict - Win32 Release" && "$(CFG)" != "Sub Mare Imperium Derelict - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Sub Mare Imperium Derelict.exe"


CLEAN :
	-@erase "$(INTDIR)\CGDIRenderer.obj"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\Core.obj"
	-@erase "$(INTDIR)\CPackedFileReader.obj"
	-@erase "$(INTDIR)\Crawler.obj"
	-@erase "$(INTDIR)\CreditsScreen.obj"
	-@erase "$(INTDIR)\CRenderer.obj"
	-@erase "$(INTDIR)\CRenderer_Rasterization.obj"
	-@erase "$(INTDIR)\CRenderer_Tesselation.obj"
	-@erase "$(INTDIR)\CTile3DProperties.obj"
	-@erase "$(INTDIR)\Derelict.obj"
	-@erase "$(INTDIR)\Dungeon.obj"
	-@erase "$(INTDIR)\EDirection_Utils.obj"
	-@erase "$(INTDIR)\Engine.obj"
	-@erase "$(INTDIR)\Events.obj"
	-@erase "$(INTDIR)\FixP.obj"
	-@erase "$(INTDIR)\GameMenu.obj"
	-@erase "$(INTDIR)\Globals.obj"
	-@erase "$(INTDIR)\HackingScreen.obj"
	-@erase "$(INTDIR)\HelpScreen.obj"
	-@erase "$(INTDIR)\LoadBitmap.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\MainMenu.obj"
	-@erase "$(INTDIR)\MapWithCharKey.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Sub Mare Imperium.res"
	-@erase "$(INTDIR)\UI.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Vec.obj"
	-@erase "$(INTDIR)\VisibilityStrategy.obj"
	-@erase "$(OUTDIR)\Sub Mare Imperium Derelict.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /O2 /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\space-trashman-blues\core\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x816 /fo"$(INTDIR)\Sub Mare Imperium.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sub Mare Imperium Derelict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\Sub Mare Imperium Derelict.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CGDIRenderer.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\Core.obj" \
	"$(INTDIR)\CPackedFileReader.obj" \
	"$(INTDIR)\Crawler.obj" \
	"$(INTDIR)\CreditsScreen.obj" \
	"$(INTDIR)\CRenderer.obj" \
	"$(INTDIR)\CRenderer_Rasterization.obj" \
	"$(INTDIR)\CRenderer_Tesselation.obj" \
	"$(INTDIR)\CTile3DProperties.obj" \
	"$(INTDIR)\Derelict.obj" \
	"$(INTDIR)\Dungeon.obj" \
	"$(INTDIR)\EDirection_Utils.obj" \
	"$(INTDIR)\Engine.obj" \
	"$(INTDIR)\Events.obj" \
	"$(INTDIR)\FixP.obj" \
	"$(INTDIR)\GameMenu.obj" \
	"$(INTDIR)\Globals.obj" \
	"$(INTDIR)\HackingScreen.obj" \
	"$(INTDIR)\HelpScreen.obj" \
	"$(INTDIR)\LoadBitmap.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\MainMenu.obj" \
	"$(INTDIR)\MapWithCharKey.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\UI.obj" \
	"$(INTDIR)\Vec.obj" \
	"$(INTDIR)\VisibilityStrategy.obj" \
	"$(INTDIR)\Sub Mare Imperium.res"

"$(OUTDIR)\Sub Mare Imperium Derelict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Sub Mare Imperium Derelict.exe"


CLEAN :
	-@erase "$(INTDIR)\CGDIRenderer.obj"
	-@erase "$(INTDIR)\Common.obj"
	-@erase "$(INTDIR)\Core.obj"
	-@erase "$(INTDIR)\CPackedFileReader.obj"
	-@erase "$(INTDIR)\Crawler.obj"
	-@erase "$(INTDIR)\CreditsScreen.obj"
	-@erase "$(INTDIR)\CRenderer.obj"
	-@erase "$(INTDIR)\CRenderer_Rasterization.obj"
	-@erase "$(INTDIR)\CRenderer_Tesselation.obj"
	-@erase "$(INTDIR)\CTile3DProperties.obj"
	-@erase "$(INTDIR)\Derelict.obj"
	-@erase "$(INTDIR)\Dungeon.obj"
	-@erase "$(INTDIR)\EDirection_Utils.obj"
	-@erase "$(INTDIR)\Engine.obj"
	-@erase "$(INTDIR)\Events.obj"
	-@erase "$(INTDIR)\FixP.obj"
	-@erase "$(INTDIR)\GameMenu.obj"
	-@erase "$(INTDIR)\Globals.obj"
	-@erase "$(INTDIR)\HackingScreen.obj"
	-@erase "$(INTDIR)\HelpScreen.obj"
	-@erase "$(INTDIR)\LoadBitmap.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\MainMenu.obj"
	-@erase "$(INTDIR)\MapWithCharKey.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Sub Mare Imperium.res"
	-@erase "$(INTDIR)\UI.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Vec.obj"
	-@erase "$(INTDIR)\VisibilityStrategy.obj"
	-@erase "$(OUTDIR)\Sub Mare Imperium Derelict.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /ZI /Od /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\space-trashman-blues\core\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sub Mare Imperium.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sub Mare Imperium Derelict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib shell32.lib winmm.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /out:"$(OUTDIR)\Sub Mare Imperium Derelict.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CGDIRenderer.obj" \
	"$(INTDIR)\Common.obj" \
	"$(INTDIR)\Core.obj" \
	"$(INTDIR)\CPackedFileReader.obj" \
	"$(INTDIR)\Crawler.obj" \
	"$(INTDIR)\CreditsScreen.obj" \
	"$(INTDIR)\CRenderer.obj" \
	"$(INTDIR)\CRenderer_Rasterization.obj" \
	"$(INTDIR)\CRenderer_Tesselation.obj" \
	"$(INTDIR)\CTile3DProperties.obj" \
	"$(INTDIR)\Derelict.obj" \
	"$(INTDIR)\Dungeon.obj" \
	"$(INTDIR)\EDirection_Utils.obj" \
	"$(INTDIR)\Engine.obj" \
	"$(INTDIR)\Events.obj" \
	"$(INTDIR)\FixP.obj" \
	"$(INTDIR)\GameMenu.obj" \
	"$(INTDIR)\Globals.obj" \
	"$(INTDIR)\HackingScreen.obj" \
	"$(INTDIR)\HelpScreen.obj" \
	"$(INTDIR)\LoadBitmap.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\MainMenu.obj" \
	"$(INTDIR)\MapWithCharKey.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\UI.obj" \
	"$(INTDIR)\Vec.obj" \
	"$(INTDIR)\VisibilityStrategy.obj" \
	"$(INTDIR)\Sub Mare Imperium.res"

"$(OUTDIR)\Sub Mare Imperium Derelict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Sub Mare Imperium Derelict.dep")
!INCLUDE "Sub Mare Imperium Derelict.dep"
!ELSE 
!MESSAGE Warning: cannot find "Sub Mare Imperium Derelict.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release" || "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"
SOURCE=..\..\base3d\src\Win32Version\CGDIRenderer.c

"$(INTDIR)\CGDIRenderer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\Common.c

"$(INTDIR)\Common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Core.c

"$(INTDIR)\Core.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\CPackedFileReader.c

"$(INTDIR)\CPackedFileReader.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\Crawler.c

"$(INTDIR)\Crawler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\CreditsScreen.c

"$(INTDIR)\CreditsScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\CRenderer.c

"$(INTDIR)\CRenderer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\CRenderer_Rasterization.c

"$(INTDIR)\CRenderer_Rasterization.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\CRenderer_Tesselation.c

"$(INTDIR)\CRenderer_Tesselation.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\CTile3DProperties.c

"$(INTDIR)\CTile3DProperties.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Derelict.c

"$(INTDIR)\Derelict.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\Dungeon.c

"$(INTDIR)\Dungeon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\EDirection_Utils.c

"$(INTDIR)\EDirection_Utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\Engine.c

"$(INTDIR)\Engine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\Events.c

"$(INTDIR)\Events.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\FixP.c

"$(INTDIR)\FixP.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\GameMenu.c

"$(INTDIR)\GameMenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\Globals.c

"$(INTDIR)\Globals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\HackingScreen.c

"$(INTDIR)\HackingScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\HelpScreen.c

"$(INTDIR)\HelpScreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\LoadBitmap.c

"$(INTDIR)\LoadBitmap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\Main.c

!IF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /O2 /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\space-trashman-blues\core\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sub Mare Imperium Derelict - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /ZI /Od /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\base3d\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\menu\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\SoundSystem" /I "c:\space-trashman-blues\space-trashman-blues\core\include" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium Derelict" /I "c:\space-trashman-blues\space-trashman-blues\mx_frontend\VC6\Sub Mare Imperium" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MSVC" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\menu\src\MainMenu.c

"$(INTDIR)\MainMenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\MapWithCharKey.c

"$(INTDIR)\MapWithCharKey.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\core\src\Parser.c

"$(INTDIR)\Parser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\menu\src\UI.c

"$(INTDIR)\UI.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\Vec.c

"$(INTDIR)\Vec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\base3d\src\VisibilityStrategy.c

"$(INTDIR)\VisibilityStrategy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Sub Mare Imperium.rc"

"$(INTDIR)\Sub Mare Imperium.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

