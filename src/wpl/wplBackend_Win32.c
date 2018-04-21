#ifndef WPL_WINDOWS
#error "Win32 Backend only functions under WPL_WINDOWS"
#endif

#include <Windows.h>
#include <Wingdi.h>
#include <Shlwapi.h>


#define WB_ALLOC_IMPLEMENTATION
#define WB_ALLOC_CUSTOM_INTEGER_TYPES
#define WB_ALLOC_BACKEND_API static
#include "thirdparty/wb_alloc.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "thirdparty/stb_sprintf.h"
#define snprintf stbsp_snprintf
#define vsnprintf stbsp_vsnprintf

#define WB_GL_IMPLEMENTATION
#define WB_GL_NO_INCLUDES
#define WB_GL_USE_ALL_VERSIONS
#define WB_GL_WIN32
#include "thirdparty/wb_gl_loader.h"

#include "wplCRT.c"

static int lastQuitEvent = 0;
static wInputState* lastInputState = NULL;

static 
void handleKey(i32 state, i32 code)
{
	if(!lastInputState) return;
	lastInputState->keys[code] = state ? Button_JustDown : Button_JustUp;
}

static 
void handleMouse(i32 state, i32 code)
{
	if(!lastInputState) return;
	lastInputState->mouse[code] = state ? Button_JustDown : Button_JustUp;
}

static 
void handleMouseWheel(i32 wheel)
{
	if(!lastInputState) return;
	lastInputState->mouseWheel = (f32)wheel / (f32)WHEEL_DELTA;
}

static
LRESULT CALLBACK windowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_QUIT:
		case WM_CLOSE:
			lastQuitEvent = 1;
			break;

		case WM_KEYDOWN:
			if(!(lParam & (1 << 30)))  {
				handleKey(1, (int)wParam);
			}
			break;

		case WM_KEYUP:
				handleKey(0, (int)wParam);
			break;

		case WM_MOUSEWHEEL:
			handleMouseWheel(HIWORD(wParam));
			break;

		case WM_LBUTTONDOWN:
			handleMouse(1, wMouseLeft); 
			break;
		case WM_MBUTTONDOWN:
			handleMouse(1, wMouseMiddle); 
			break;
		case WM_RBUTTONDOWN:
			handleMouse(1, wMouseRight); 
			break;
		case WM_XBUTTONDOWN:
			if(HIWORD(wParam) == XBUTTON1) {
				handleMouse(1, wMouseX1); 
			} else if(HIWORD(wParam) == XBUTTON2) {
				handleMouse(1, wMouseX2); 
			}
			break;

		case WM_LBUTTONUP:
			handleMouse(0, wMouseLeft); 
			break;
		case WM_MBUTTONUP:
			handleMouse(0, wMouseMiddle); 
			break;
		case WM_RBUTTONUP:
			handleMouse(0, wMouseRight); 
			break;
		case WM_XBUTTONUP:
			if(HIWORD(wParam) == XBUTTON1) {
				handleMouse(0, wMouseX1); 
			} else if(HIWORD(wParam) == XBUTTON2) {
				handleMouse(0, wMouseX2); 
			}
			break;


		default:
			return DefWindowProcA(window, message, wParam, lParam);
	}

	return 0;
}

typedef struct
{
	HWND wnd;
	HDC windowDC;
	HGLRC glContext;
} wWin32Window;

void wQuit()
{
	ExitProcess(0);
}


i64 wCreateWindow(wWindowDef* def, wWindow* window)
{
	wWin32Window* w32win = malloc(sizeof(wWin32Window));
	window->windowHandle = w32win;
	HANDLE module = GetModuleHandle(NULL);
	WNDCLASSA windowClass = {0};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hInstance = module;
	windowClass.lpfnWndProc = windowCallback;
	windowClass.lpszClassName = "wplWindowClass";

	if(!RegisterClassA(&windowClass)) {
		wLogError(0, "Failed to register window class");
		return 1;
	}

	if(def->width == 0) {
		def->width = 1280;
	} 

	if(def->height == 0) {
		def->height = 720;
	}
	i64 wposx, wposy;
	if(def->posCentered) {
		wposx = (GetSystemMetrics(SM_CXSCREEN) - def->width) / 2;
		wposy = (GetSystemMetrics(SM_CYSCREEN) - def->height) / 2;
	} else if(def->posUndefined) {
		wposx = CW_USEDEFAULT;
		wposy = CW_USEDEFAULT;
	} else {
		wposx = def->x;
		wposy = def->y;
	}

	HWND wnd = CreateWindowExA(0, 
			windowClass.lpszClassName,
			def->title,
			WS_OVERLAPPEDWINDOW,
			wposx, wposy,
			def->width, def->height,
			0, 0, 
			windowClass.hInstance,
			0);
	HDC windowDC = GetDC(wnd);
	w32win->wnd = wnd;
	w32win->windowDC = windowDC;
	w32win->glContext = wbgl_win32_create_context(windowDC, 3, 3, 1);
	i32 ret = wbgl_load_all(NULL);

	if(!def->hidden) {
		wShowWindow(window);
	}

	{
		char filename[1024];
		u32 size = GetModuleFileNameA(module, filename, 1024);
		for(u32 find = size; find > 0; find--) {
			if(filename[find] == '\\') {
				size = find;
				break;
			}
		}
		filename[size+1] = '\0';
		window->basePath = malloc(size+1);
		memcpy(window->basePath, filename, size+1);
	}

	return 0;
}

void wShowWindow(wWindow* window)
{
	HWND wnd = ((wWin32Window*)window->windowHandle)->wnd;
	if(ShowWindow(wnd, SW_SHOWNORMAL) != 0) {
		//return 1;
	}
}

i64 wUpdate(wWindow* window, wState* state)
{
	wInputUpdate(state->input);
	
	HWND wnd = ((wWin32Window*)window->windowHandle)->wnd;
	MSG message;
	while(PeekMessage(&message, wnd, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	//TODO(will):
	// update state with new width/height, global mouse pos, event data
	RECT r;
	GetClientRect(wnd, &r);
	state->width = r.right - r.left;
	state->height = r.bottom - r.top;
	glViewport(0, 0, state->width, state->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	state->exitEvent = lastQuitEvent;
	
	return 1;
}

i64 wRender(wWindow* window)
{
	HDC windowDC = ((wWin32Window*)window->windowHandle)->windowDC;
	SwapBuffers(windowDC);
	//TODO(will): perform manual timing here
	return 0;
}


u8* wLoadFile(string filename, isize* sizeOut, wMemoryArena* alloc)
{
	HANDLE file = CreateFile(filename, 
			GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER largeSize;
	if(!GetFileSizeEx(file, &largeSize)) {
		CloseHandle(file);
		return NULL;
	}
	isize size = (isize)largeSize.QuadPart;  
	u8* buffer = wArenaPush(alloc, size+1);
	
	if(!ReadFile(file, buffer, (u32)size, NULL, NULL)) {
		CloseHandle(file);
		return NULL;
	}

	if(sizeOut) {
		*sizeOut = size;	
	}
	CloseHandle(file);
	return buffer;
}

//returns actual number of bytes loaded;
isize wLoadSizedFile(string filename, u8* buffer, isize bufferSize)
{
	HANDLE file = CreateFile(filename, 
			GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER largeSize;
	if(!GetFileSizeEx(file, &largeSize)) {
		CloseHandle(file);
		return 0;
	}
	// this dance to avoid a signed/unsigned mismatch
	u32 sizeu = (u32)largeSize.QuadPart;  
	isize size = (isize)sizeu;
	if(size > bufferSize) size = bufferSize;
	
	if(!ReadFile(file, buffer, size, NULL, NULL)) {
		CloseHandle(file);
		return 0;
	}
	CloseHandle(file);
	return size;
}

u8* wLoadLocalFile(wWindow* window, string filename, isize* sizeOut, wMemoryArena* arena)
{
	char buf[1024];
	snprintf(buf, 1024, "%s%s", window->basePath, filename);
	return wLoadFile(buf, sizeOut, arena);
}

isize wLoadLocalSizedFile(
		wWindow* window, string filename,
		u8* buffer, isize bufferSize)
{
	char buf[1024];
	snprintf(buf, 1024, "%s%s", window->basePath, filename);
	return wLoadSizedFile(buf, buffer, bufferSize);
}

wFileHandle wGetFileHandle(string filename)
{
	u32 access = GENERIC_READ;
	u32 share = FILE_SHARE_READ;
	u32 create = OPEN_EXISTING;
	u32 flags = FILE_ATTRIBUTE_NORMAL;
	wFileHandle file = CreateFileA(
			filename,
			access,
			share, 
			NULL,
			create,
			flags,
			NULL
			);
	if(file == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	return file;
}

void wCloseFileHandle(wFileHandle file)
{
	CloseHandle(file);
}

isize wGetFileSize(wFileHandle file)
{
	LARGE_INTEGER i;
	GetFileSizeEx(file, &i);
	return (isize)i.QuadPart + 1;
}

isize wGetFileModifiedTime(wFileHandle file)
{
	LARGE_INTEGER i;
	FILETIME t;
	GetFileTime(file, NULL, NULL, &t);
	i.u.LowPart = t.dwLowDateTime;
	i.u.HighPart = t.dwHighDateTime;
	return i.QuadPart;
}


