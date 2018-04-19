@echo off
set msvcdir="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\"
if not defined DevEnvDir call %msvcdir%vcvars64.bat >nul

if not exist bin mkdir bin
copy usr\lib\*.dll bin\ >nul 2>&1

rem Kill task if it's running
taskkill /IM MonsterTrainer.exe 1>NUL 2>&1

set wirmpht=usr\bin\wirmpht.exe
if exist %wirmpht% %wirmpht% -p -s -t src\main.c > src\generated.h

nmake /nologo -k -f windows.mak
set lasterror=%errorlevel%

if "%~1"=="run" goto run
goto Quit

:run
if %lasterror% LEQ 0 start bin\wpltest.exe

:Quit
