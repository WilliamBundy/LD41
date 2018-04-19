/* wpl -- William's Platform Layer
 *
 */

// Basic platform detection
//
// If you want to do this yourself, or specify something special, your choices are:
// WPL_WINDOWS or WPL_LINUX
// On WPL_LINUX, you have to use WPL_SDL_BACKEND
// On WPL_WINDOWS, you can use either WPL_WIN32_BACKEND or WPL_SDL_BACKEND

#if !defined(WPL_WINDOWS) && !defined(WPL_LINUX)
#if _MSC_VER
#define WPL_WINDOWS
#else
#define WPL_LINUX
#endif 
#endif

#if !defined(WPL_WIN32_BACKEND) && !defined(WPL_SDL_BACKEND)
#ifdef WPL_WINDOWS
#define WPL_WIN32_BACKEND
#else
#define WPL_SDL_BACKEND
#endif
#endif

// Safe CRT headers
#include <stdarg.h>
#include <stdint.h>
#include <intrin.h>

// Global header
#include "wpl.h"
#include "wplBackend.h"

// Stuff the backend relies on
#include "wplInput.c"

#ifdef WPL_WIN32_BACKEND
//#error "WPL: Using Win32 Backend"
#include "wplBackend_Win32.c"
#elif WPL_SDL_BACKEND
//#error "WPL: Using SDL2 Backend"
#include "wplBackend_SDL.c"
#else
#error "WPL: No backend defined"
#endif

// Stuff that relies on the backend
#include "wplRender.c"
#include "wplArchive.c"
#include "wplUtil.c"

// Other functions
wWindowDef wDefineWindow(string title)
{
	wWindowDef def;
	memset(&def, 0, sizeof(wWindowDef));
	def.title = title;
	def.posCentered = 1;
	def.resizeable = 1;
	def.glVersion = 33;
	def.width = 1280;
	def.height = 720;
	return def;
}

void wInitState(wState* state, wInputState* input)
{
	//TODO(will): null assert error checking
	memset(state, 0, sizeof(wState));
	memset(input, 0, sizeof(wInputState));

	state->input = input;
}

