@echo OFF
REM !!! First initialize vcvars !!!

setlocal EnableDelayedExpansion
set srcPath=%~dp0src\
set objPath=%~dp0obj\
set binPath=%~dp0bin\
set P7logger_path="E:\Projects\libP7"
set P7logger_lib="%P7logger_path%\build32\Sources\MinSizeRel\p7.lib"
REM -------------- SET COMP FLAGS -------------- 
set CompilerFlags=/EHsc /W3 /wd4101 /wd4201 /wd4477 /nologo /Fo%objPath% /Zi /Zc:wchar_t /std:c++latest /MD
REM -------------- SET COMP FLAGS -------------- 


REM -------------- SET ADDITIONAL INCLUDE DIRs -------------- 
set CompilerFlags=%CompilerFlags% /I %P7logger_path%\Headers /I ../include
REM -------------- SET ADDITIONAL INCLUDE DIRs -------------- 


REM -------------- SET MACRO DEFs -------------- 
set CompilerFlags=%CompilerFlags% /D _CRT_SECURE_NO_WARNINGS /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING /D ODBC_STATIC
REM -------------- SET MACRO DEFs -------------- 


REM -------------- SET LINKER FLAGS -------------- 
set LinkerFlags=/SUBSYSTEM:CONSOLE /incremental:no /NOLOGO /OUT:%binPath%bin.exe
REM -------------- SET LINKER FLAGS -------------- 


REM -------------- SET UNITS -------------- 
set TUnits=main.cpp odbc.cpp odbc_data.cpp logger.cpp
REM -------------- SET UNITS --------------


REM -------------- SET DEPENDENCIES --------------  
set Depends=%P7logger_lib% "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" "Ws2_32.lib"
REM -------------- SET DEPENDENCIES --------------  


if not exist %objPath% mkdir %objPath%
if not exist %binPath% mkdir %binPath%

cd %srcPath%

cl %TUnits% %CompilerFlags% /link %LinkerFlags% %Depends%

del vc140.pdb

cd ..