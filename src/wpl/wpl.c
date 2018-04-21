/* wpl -- William's Platform Layer
 *
 */

// Global header
#include "wpl.h"
#include "wplBackend.h"

// Stuff the backend relies on
#include "wplInput.c"

#ifdef WPL_WIN32_BACKEND
//#error "WPL: Using Win32 Backend"
#include "wplBackend_Win32.c"
#else
#ifdef WPL_SDL_BACKEND
//#error "WPL: Using SDL2 Backend"
#include "wplBackend_SDL.c"
#else
#error "WPL: No backend defined"
#endif
#endif

// Stuff that relies on the backend
#include "wplRender.c"
#include "wplFileHandling.c"
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

