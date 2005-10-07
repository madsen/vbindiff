# Microsoft Developer Studio Project File - Name="vbindiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vbindiff - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vbindiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vbindiff.mak" CFG="vbindiff - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vbindiff - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vbindiff - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vbindiff - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /Ob2 /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"config.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "vbindiff - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"config.h" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "vbindiff - Win32 Release"
# Name "vbindiff - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ConWin.cpp
# End Source File
# Begin Source File

SOURCE=..\GetOpt\GetOpt.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"config.h"
# End Source File
# Begin Source File

SOURCE=..\vbindiff.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\ConWin.hpp
# End Source File
# Begin Source File

SOURCE=.\FileIO.hpp
# End Source File
# Begin Source File

SOURCE=..\GetOpt\GetOpt.hpp
# End Source File
# Begin Source File

SOURCE=..\tables.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\vbindiff.rc
# End Source File
# End Group
# Begin Group "Build Process"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\configure.ac
# End Source File
# Begin Source File

SOURCE=..\tools\version.h.tt

!IF  "$(CFG)" == "vbindiff - Win32 Release"

USERDEP__VERSI="../configure.ac"	
# Begin Custom Build - Generating version.h from configure.ac...
ProjDir=.
InputPath=..\tools\version.h.tt

"$(ProjDir)/version.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl ..\tools\genfile.pl $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "vbindiff - Win32 Debug"

USERDEP__VERSI="../configure.ac"	
# Begin Custom Build - Generating version.h from configure.ac...
ProjDir=.
InputPath=..\tools\version.h.tt

"$(ProjDir)/version.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl ..\tools\genfile.pl $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
