ctime=usr\bin\ctime.exe
lineify=usr\bin\lineify.exe
disabled=/wd4477\
		 /wd4244\
		 /wd4334\
		 /wd4305\
		 /wd4101\
		 /D_CRT_SECURE_NO_WARNINGS
.SILENT:
all: start shaders libwin32 game libsdl sdlgame end


libsdl: 
	echo WPL_SDL_BACKEND
	cl /nologo /c /TC /Zi /MT /Gd /EHsc /W3 /fp:fast $(disabled) \
		/I"usr/include" /DWPL_SDL_BACKEND src/wpl/wpl.c /Fd"bin/wplsdl.pdb" 
	lib /NOLOGO /SUBSYSTEM:WINDOWS /LIBPATH:"usr/lib" \
		wpl.obj /OUT:"usr/lib/wplsdl.lib"

libwin32: 
	echo WPL_WIN32_BACKEND
	cl /nologo /c /TC /Zi /Gd /EHsc /Gs16777216 /F16777216 \
		/Gm- /GS- /W3 /fp:fast $(disabled) \
		/I"usr/include" /DWPL_REPLACE_CRT /DWPL_USE_WIN32 src/wpl/wpl.c /Fd"bin/wpl.pdb" 
	lib /NOLOGO /SUBSYSTEM:WINDOWS /LIBPATH:"usr/lib" \
		/NODEFAULTLIB wpl.obj  /OUT:"usr/lib/wpl.lib"

sar:
	cl /nologo /TC /Zi /Gd /MT /EHsc /W3 /fp:fast $(disabled) \
		src\sartool\sar.c /Fe"usr/bin/sar.exe" /Fd"sar.pdb" \
	/link /NOLOGO /INCREMENTAL:NO /SUBSYSTEM:CONSOLE \
		setargv.obj Shell32.lib user32.lib \

sdlgame: 
	echo SDL Game
	cl /nologo /TC /Zi /MT /Gd /EHsc /W3 /fp:fast $(disabled) \
		src/main.c /DWPL_SDL_BACKEND \
		/Fe"bin/wplsdltest.exe" /Fd"bin/wplsdltest.pdb" \
	/link /NOLOGO /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /LIBPATH:"usr/lib"\
		kernel32.lib user32.lib opengl32.lib gdi32.lib wplsdl.lib SDL2.lib

game: 
	echo Win32 Game
	cl /nologo /TC /Zi /Gd /EHsc /W3 /F16777216 \
		/Gs16777216 /Gm- /GS- /fp:fast $(disabled) \
		src/main.c \
		/Fe"bin/wpltest.exe" /Fd"bin/wpltest.pdb" \
	/link /NOLOGO /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /NODEFAULTLIB \
	/STACK:16777216,16777216 /entry:GameMain /LIBPATH:"usr/lib"\
		kernel32.lib user32.lib opengl32.lib gdi32.lib wpl.lib 

shaders:
	$(lineify) -d src\shaders\* > src\shaders.h

start:
	$(ctime) -begin usr/bin/wpltest.ctm

end:
	del *.obj >nul 2>&1
	$(ctime) -end usr/bin/wpltest.ctm
