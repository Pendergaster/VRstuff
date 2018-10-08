@echo off

REM ox- nopee optimointi, 0t nopeutta , oB2 inlineany, Oi instribdsdsds
REM Od ei optimisaatioita
REM CLAGS= -Ox -Ot -OB2 -Oi
REM nologo ei turhaa printtiä / /MD common runtime multithreaded   /   /link alottaa linkkaamisen / 
REM -LD -> buildaa .dll -MD jälkee

if not defined DEV_ENV (
		CALL "%VS140COMNTOOLS%/../../VC/vcvarsall.bat" x64
		)
set DEV_ENV=???

set includes=-I"../include" -I"../Shared" -I"..\..\..\PakkiUtils" -I"..\vrheaders"
set game_includes=-I"../Shared"
set lib_path="../libraries/"
set libs=glfw3.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  Shell32.lib assimp-vc140-mt.lib LibOVR.lib
SETLOCAL


IF /I "%1"=="build_engine" (
		cls
		SET BUILD_DIR=DebugBin
		pushd DebugBin
		set CLAGS= -Od
		REM -LD -> buildaa .dll -MD jälkee
		cl %CLAGS% -nologo -Z7 -W4 -wd4201 /EHsc /DEBUG ..\src\main.cpp   %includes% /MD /link %libs% -LIBPATH:../libraries 
		popd
		)

IF /I "%1"=="build_test" (
		pushd TestBin
		set CLAGS= -Od
		cl %CLAGS% -Z7 /EHsc /DEBUG ..\src\test.cpp  %includes% /MD /link %libs% -LIBPATH:../libraries 
		popd
		)



IF /I "%1"=="build_game" (
		pushd DebugBin
		cl %CLAGS% -nologo %game_includes% ..\game\game.c  /MD /LD /link  %libs% -LIBPATH:../libraries  
		popd
		)


IF /I "%1"=="run" (
		cls
		chdir %~dp0
		DebugBin\main.exe
		popd
		)

		REM TestBin\test.exe
IF /I "%1"=="run_test" (
		REM chdir %~dp0
		REM pushd TestBin
		TestBin\test.exe
		REM popd
		)

ENDLOCAL
