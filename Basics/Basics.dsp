# Microsoft Developer Studio Project File - Name="Basics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Basics - Win32 Multithread DLL Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Basics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Basics.mak" CFG="Basics - Win32 Multithread DLL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Basics - Win32 Singlethread Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Basics - Win32 Mulithread Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Basics - Win32 Multithread DLL Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Basics - Win32 Singlethread Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Basics - Win32 Multithread Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Basics - Win32 Multithread DLL Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Basics - Win32 Singlethread Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Basics___Win32_Singlethread_Release"
# PROP BASE Intermediate_Dir "Basics___Win32_Singlethread_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Basics___Win32_Singlethread_Release"
# PROP Intermediate_Dir "Basics___Win32_Singlethread_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "inc" /I "..\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "inc" /I "inc\zlib" /I "..\inc\1.0" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-st.lib"

!ELSEIF  "$(CFG)" == "Basics - Win32 Mulithread Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Basics___Win32_Mulithread_Release"
# PROP BASE Intermediate_Dir "Basics___Win32_Mulithread_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Basics___Win32_Mulithread_Release"
# PROP Intermediate_Dir "Basics___Win32_Mulithread_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "inc" /I "..\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "inc" /I "inc\zlib" /I "..\inc\1.0" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-mt.lib"

!ELSEIF  "$(CFG)" == "Basics - Win32 Multithread DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Basics___Win32_Multithread_DLL_Release"
# PROP BASE Intermediate_Dir "Basics___Win32_Multithread_DLL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Basics___Win32_Multithread_DLL_Release"
# PROP Intermediate_Dir "Basics___Win32_Multithread_DLL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "inc" /I "..\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "inc" /I "inc\zlib" /I "..\inc\1.0" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-mtd.lib"

!ELSEIF  "$(CFG)" == "Basics - Win32 Singlethread Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Basics___Win32_Singlethread_Debug"
# PROP BASE Intermediate_Dir "Basics___Win32_Singlethread_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Basics___Win32_Singlethread_Debug"
# PROP Intermediate_Dir "Basics___Win32_Singlethread_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "inc" /I "..\inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "inc\zlib" /I "." /I "inc" /I "..\inc\1.0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0d.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-st-d.lib"

!ELSEIF  "$(CFG)" == "Basics - Win32 Multithread Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Basics___Win32_Multithread_Debug"
# PROP BASE Intermediate_Dir "Basics___Win32_Multithread_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Basics___Win32_Multithread_Debug"
# PROP Intermediate_Dir "Basics___Win32_Multithread_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "inc" /I "..\inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "inc" /I "inc\zlib" /I "..\inc\1.0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0d.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-mt-d.lib"

!ELSEIF  "$(CFG)" == "Basics - Win32 Multithread DLL Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Basics___Win32_Multithread_DLL_Debug"
# PROP BASE Intermediate_Dir "Basics___Win32_Multithread_DLL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Basics___Win32_Multithread_DLL_Debug"
# PROP Intermediate_Dir "Basics___Win32_Multithread_DLL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "inc" /I "..\inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "inc" /I "inc\zlib" /I "..\inc\1.0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\Basics-1.0d.lib"
# ADD LIB32 /nologo /out:"..\lib\1.0\Basics-mtd-d.lib"

!ENDIF 

# Begin Target

# Name "Basics - Win32 Singlethread Release"
# Name "Basics - Win32 Mulithread Release"
# Name "Basics - Win32 Multithread DLL Release"
# Name "Basics - Win32 Singlethread Debug"
# Name "Basics - Win32 Multithread Debug"
# Name "Basics - Win32 Multithread DLL Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ZLib Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\src\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\src\zlib\zutil.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Basics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BufferedReader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BufferedWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ByteArrayReader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ByteArrayWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CompressedReader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CompressedWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DataReader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DataWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EventDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ProgressReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Reader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdioReader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdioWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Trf.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Vec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Writer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ZLib Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\inc\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\inc\zlib\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\inc\1.0\Basics.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\EventDispatcher.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\Exceptions.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\Reader.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\Trf.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\Vec.h
# End Source File
# Begin Source File

SOURCE=..\inc\1.0\Writer.h
# End Source File
# End Group
# End Target
# End Project
