@echo off

REM ox- nopee optimointi, 0t nopeutta , oB2 inlineany, Oi instribdsdsds
REM Od ei optimisaatioita
REM -OB2
SET CLAGS= -Od -W4 -wd4201 -wd4505
REM -Ox -Ot  -Oi -W4 -wd4201
REM nologo ei turhaa printtiä / /MD common runtime multithreaded   /   /link alottaa linkkaamisen / 
REM -LD -> buildaa .dll -MD jälkee

if not defined DEV_ENV (
		CALL "%VS140COMNTOOLS%/../../VC/vcvarsall.bat" x64
		)
set DEV_ENV=???

set includes=-I"../include" -I"../Shared" -I"..\..\..\PakkiUtils" -I"..\vrheaders"
set game_includes=-I"../../shared" -I"../../../../PakkiUtils" -I"../include" -I"../../include"
set lib_path="../libraries/"
set libs=glfw3.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  Shell32.lib assimp-vc140-mt.lib LibOVR.lib 
set gamelibs=OpenAL32.lib Raknet.lib

SETLOCAL


IF /I "%1"=="build_engine" (
		cls
		SET BUILD_DIR=DebugBin
		pushd DebugBin
		set CLAGS= -Od
		REM -LD -> buildaa .dll -MD jälkee
		REM 
		cl %CLAGS% -Z7 -nologo  -W4 -wd4201 /EHsc /DEBUG /fp:fast ..\src\main.cpp   %includes% /MD /link %libs% -LIBPATH:../libraries 
		popd
		)

IF /I "%1"=="build_test" (
		cls
		pushd TestBin
		set CLAGS= -Od
		cl %CLAGS% -Z7 /EHsc /DEBUG ..\src\test.cpp  %includes% /MD /link %libs% -LIBPATH:../libraries 
		popd
		)



IF /I "%1"=="build_game" (
		echo "WAITING FOR PDB ..." > .lock
		cd game
		pushd DebugBin
		REM -LD -> buildaa .dll -MD jälkee
		REM nologo ei turhaa printtiä / /MD common runtime multithreaded   /   /link alottaa linkkaamisen / 
		cl -Z7 -Od -nologo -W4 -wd4201 %game_includes% ..\src\game.cpp  /MD /LD /EHcs /link  %gamelibs% -LIBPATH:../libraries  
		popd
		cd ../
		del .lock
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
		echo "Running test"
		TestBin\test.exe
		REM popd
		)

ENDLOCAL
