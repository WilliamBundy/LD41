/****************************************
 * wb_gl_loader.h 
 *
 * A single-header OpenGL loader, using 
 * Win32 or SDL as its backend.
 *
 * Can also create contexts for Win32
 * (because dealing with that yourself is terrible)
 *
 * Sample Usage:
 *
 * #define WB_GL_USE_ALL_VERSIONS
 * #define WB_GL_WIN32
 * #define WB_GL_LOADER_IMPLEMENTATION
 * ...
 * {
 *     wbgl_win32_create_context(windowDC, 3, 3, 1);
 *     wbgl_load_all(NULL);
 * }
 *
 * #define WB_GL_SDL
 * If you wish to use this loader out-of-the-box on Linux or OSX
 * Or, define your own wbgl__load_proc.
 * With the SDL route, you'll have to make your own context,
 * but this is comparatively easy and sane.
 * 
 * Handles OpenGL by general groups: 
 *  - legacy (2.1 and previous)
 *  - compat (3.1)
 *  - core   (3.2 and 3.3)
 *  - modern (4.0 - 4.5)
 * Use WB_GL_USE_***** to use a subset, or 
 * WB_GL_USE_ALL_VERSIONS for all of them. 
 *
 * #define WB_GL_USE_CORE
 * #define WB_GL_USE_ALL_VERSIONS
 *
 * The latter is recommended for simplicity; 
 * there's no issue declaring unused function 
 * pointers, though they might fail to load 
 * depending on the context version and 
 * drivers. As long as you don't use a procedure
 * that isn't loaded, everything will be fine.
 *
 * Other options:
 *
 * #define WB_GL_NO_TYPES
 * Don't define any of the GL**** types
 * You'll have to define them yourself in order to use them.
 *
 * #define WB_GL_NO_STDINT
 * Use generic C types instead of specific stdint sized types.
 * There's a GLuint64 type that gets defined as "unsigned long long"
 * This might not work on gcc or clang; use WB_GL_NO_TYPES instead
 *
 * #define WB_GL_NO_INCLUDES
 * Don't include anything. This applies to Windows.h and Wingdi.h
 * for the win32 path, and stddef.h for all paths.
 *
 * #define WB_GL_GLPROC_API
 * If you want your GL procs to have a specific storage class,
 * assign this. By default it's __stdcall if implementing the 
 * library, and extern __stdcall otherwise. 
 * (__stdcall does nothing on 64 bit; if it's not defined,
 * just #define __stdcall as nothing.)
 *
 * #define WB_GL_LOADER_API
 * If you want any a specific storage class on the loader 
 * functions (eg, static). It's extern if not an implementation, 
 * and nothing otherwise.
 *
 * #define WB_GL_MAX_ERRORS 256
 * Defines the maximum number of errors to be stored in the
 * wbgl_ErrorContext struct. 
 * 
 *
 ***************************************/

/* Include guards are so 1980's */
#pragma once

#ifdef WB_GL_USE_ALL_VERSIONS
#define WB_GL_USE_LEGACY
#define WB_GL_USE_COMPAT
#define WB_GL_USE_CORE
#define WB_GL_USE_MODERN
#endif

#ifndef WB_GL_LOADER_IMPLEMENTATION

#ifndef WB_GL_GLPROC_API
#define WB_GL_GLPROC_API extern 
#endif
#ifndef WB_GL_LOADER_API
#define WB_GL_LOADER_API extern
#endif

#else

#ifndef WB_GL_GLPROC_API
#define WB_GL_GLPROC_API 
#endif
#ifndef WB_GL_LOADER_API
#define WB_GL_LOADER_API
#endif

#endif

#ifndef WB_GL_NO_TYPES
#ifndef WB_GL_NO_INCLUDES
#include <stddef.h>
#endif
#ifdef WG_GL_NO_STDINT
typedef unsigned int  GLenum;
typedef float         GLfloat;
typedef int           GLint;
typedef int           GLsizei;
typedef unsigned int  GLbitfield;
typedef double        GLdouble;
typedef unsigned int  GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef char          GLchar;
typedef char          GLbyte;
typedef ptrdiff_t     GLintptr;
typedef ptrdiff_t     GLsizeiptr;
typedef void          GLvoid;
typedef float         GLclampf;
typedef short int              GLshort;
typedef unsigned short int     GLshort;
typedef void *GLsync;
typedef unsigned long long GLuint64;
typedef void* GLDEBUGPROC;
#else
typedef uint32_t      GLenum;
typedef float         GLfloat;
typedef int32_t       GLint;
typedef int16_t       GLshort;
typedef uint16_t      GLushort;
typedef int64_t       GLint64;
typedef int32_t       GLsizei;
typedef uint32_t      GLbitfield;
typedef double        GLdouble;
typedef uint32_t      GLuint;
typedef uint8_t       GLboolean;
typedef uint8_t       GLubyte;
typedef int8_t        GLchar;
typedef int8_t        GLbyte;
typedef ptrdiff_t     GLintptr;
typedef ptrdiff_t     GLsizeiptr;
typedef void          GLvoid;
typedef float         GLclampf;
typedef void *GLsync;
typedef void* GLDEBUGPROC;
typedef uint64_t GLuint64;
#endif
#endif

#ifndef WB_GL_MAX_ERRORS
#define WB_GL_MAX_ERRORS 256
#endif

struct wbgl_ErrorContext
{
	int error_count;
	int failed_size;
	const char* failed[WB_GL_MAX_ERRORS];
};

extern int wbgl_load_all(struct wbgl_ErrorContext*);
extern void* wbgl__load_proc(const char*, struct wbgl_ErrorContext*, void*);
#ifdef WB_GL_WIN32
#ifndef WB_GL_NO_INCLUDES
#include <Windows.h>
#include <Wingdi.h>
#endif
extern HGLRC wbgl_win32_create_context(HDC windowDC, int major, int minor, int core);
#endif

#ifdef WB_GL_IMPLEMENTATION
static void wbgl__load_proc_error(const char* name, struct wbgl_ErrorContext* ctx)
{
	if(!ctx) return;
	ctx->error_count++;
	if(ctx->error_count < WB_GL_MAX_ERRORS) {
		ctx->failed[ctx->error_count - 1] = name;
		ctx->failed_size++;
	}
}

#ifdef WB_GL_WIN32
#ifndef WB_GL_NO_INCLUDES
#include <Windows.h>
#include <Wingdi.h>
#endif
WB_GL_LOADER_API
void* wbgl__load_proc(const char* name, struct wbgl_ErrorContext* ctx, void* userdata)
{
	HMODULE gldll = (HMODULE)userdata;
	void* p = (void*)wglGetProcAddress(name);
	long long int err = (long long int)p;
	if(err == 0 || err == 1 || err == 2 || err == 3 || err == -1) {
		p = (void*)GetProcAddress(gldll, name);
		if(!p) {
			wbgl__load_proc_error(name, ctx);
		}
    }
	return p;
}

#define WB_GL__WGL_PROC_LIST \
	WB_GL__WGLPROC(HGLRC WINAPI, wglCreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int* attribList) \
	WB_GL__WGLPROC(BOOL WINAPI, wglSwapIntervalEXT, int interval) \
	WB_GL__WGLPROC(BOOL WINAPI, wglChoosePixelFormatARB, HDC hDC, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats) \

#define WB_GL__WGLPROC(ret, name, ...) typedef ret name##Proc(__VA_ARGS__); static name##Proc *name;
WB_GL__WGL_PROC_LIST
#undef WB_GL__WGLPROC

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x1
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x2
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

WB_GL_LOADER_API
HGLRC wbgl_win32_create_context(HDC windowDC, int major, int minor, int core)
{
	{
		WNDCLASSA windowClass = {0};
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = DefWindowProcA;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.lpszClassName = "wb_gl_loader_class";
		if(!RegisterClassA(&windowClass)) {
			return NULL;
		}

		HWND window = CreateWindowExA(0,
				windowClass.lpszClassName,
				"wb_gl_loader_window", 
				0,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				0, 0, GetModuleHandle(NULL), 0);

		if(!window) {
			return NULL;
		}

		HDC windowDC = GetDC(window);
		PIXELFORMATDESCRIPTOR pfd = {0};
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 24;
		pfd.cAlphaBits = 8;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pfi = ChoosePixelFormat(windowDC, &pfd);
		PIXELFORMATDESCRIPTOR spfd;
		DescribePixelFormat(windowDC, pfi, sizeof(spfd), &spfd);
		if(!SetPixelFormat(windowDC, pfi, &spfd)) {
			return NULL;
		}

		HGLRC glctx = wglCreateContext(windowDC);
		if(!wglMakeCurrent(windowDC, glctx)) {
			return NULL;
		}

#define WB_GL__WGLPROC(ret, name, ...) name = (name##Proc *)wglGetProcAddress(#name); 
		WB_GL__WGL_PROC_LIST;
#undef  WB_GL__WGLPROC

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glctx);
		ReleaseDC(window, windowDC);
		DestroyWindow(window);
	}

	const int pixelAttribs[] =  {
		WGL_DRAW_TO_WINDOW_ARB, 1,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_SUPPORT_OPENGL_ARB, 1,
		WGL_DOUBLE_BUFFER_ARB, 1,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		0
	};

	int pfi;
	unsigned int count;
	if(!wglChoosePixelFormatARB(windowDC, pixelAttribs, 0, 1, &pfi, &count)) {
		return NULL;
	}

	PIXELFORMATDESCRIPTOR spfd;
	DescribePixelFormat(windowDC, pfi, sizeof(spfd), &spfd);
	if(!SetPixelFormat(windowDC, pfi, &spfd)) {
		return NULL;
	}

	const int glAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB,
			0,
		WGL_CONTEXT_PROFILE_MASK_ARB, core ? 1 : 2,
		0
	};
	int ver = 0;

	HGLRC glctx = wglCreateContextAttribsARB(windowDC, 0, glAttribs);
	if(!glctx) {
		return NULL;
	}

	if(!wglMakeCurrent(windowDC, glctx)) {
		return NULL;
	}

	wglSwapIntervalEXT(1);
	return glctx;
}
#endif
#ifdef WB_GL_SDL
WB_GL_LOADER_API
void* wbgl__load_proc(const char* name, struct wbgl_ErrorContext* ctx, void* userdata)
{
	void* proc = SDL_GL_GetProcAddress(name);
	if(!proc) {
		wbgl__load_proc_error(name, ctx);
	}
	return proc;
}
#endif

#endif

#define WB_GL__PROC_LIST_LEGACY \
	WB_GL__GLPROC(void, CullFace, GLenum mode) \
	WB_GL__GLPROC(void, FrontFace, GLenum mode) \
	WB_GL__GLPROC(void, Hint, GLenum target, GLenum mode) \
	WB_GL__GLPROC(void, LineWidth, GLfloat width) \
	WB_GL__GLPROC(void, PointSize, GLfloat size) \
	WB_GL__GLPROC(void, PolygonMode, GLenum face, GLenum mode) \
	WB_GL__GLPROC(void, Scissor, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, TexParameterf, GLenum target, GLenum pname, GLfloat param) \
	WB_GL__GLPROC(void, TexParameterfv, GLenum target, GLenum pname, const GLfloat *params) \
	WB_GL__GLPROC(void, TexParameteri, GLenum target, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, TexParameteriv, GLenum target, GLenum pname, const GLint *params) \
	WB_GL__GLPROC(void, TexImage1D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, TexImage2D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, DrawBuffer, GLenum buf) \
	WB_GL__GLPROC(void, Clear, GLbitfield mask) \
	WB_GL__GLPROC(void, ClearColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
	WB_GL__GLPROC(void, ClearStencil, GLint s) \
	WB_GL__GLPROC(void, ClearDepth, GLdouble depth) \
	WB_GL__GLPROC(void, StencilMask, GLuint mask) \
	WB_GL__GLPROC(void, ColorMask, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) \
	WB_GL__GLPROC(void, DepthMask, GLboolean flag) \
	WB_GL__GLPROC(void, Disable, GLenum cap) \
	WB_GL__GLPROC(void, Enable, GLenum cap) \
	WB_GL__GLPROC(void, Finish, void) \
	WB_GL__GLPROC(void, Flush, void) \
	WB_GL__GLPROC(void, BlendFunc, GLenum sfactor, GLenum dfactor) \
	WB_GL__GLPROC(void, LogicOp, GLenum opcode) \
	WB_GL__GLPROC(void, StencilFunc, GLenum func, GLint ref, GLuint mask) \
	WB_GL__GLPROC(void, StencilOp, GLenum fail, GLenum zfail, GLenum zpass) \
	WB_GL__GLPROC(void, DepthFunc, GLenum func) \
	WB_GL__GLPROC(void, PixelStoref, GLenum pname, GLfloat param) \
	WB_GL__GLPROC(void, PixelStorei, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, ReadBuffer, GLenum src) \
	WB_GL__GLPROC(void, ReadPixels, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) \
	WB_GL__GLPROC(void, GetBooleanv, GLenum pname, GLboolean *data) \
	WB_GL__GLPROC(void, GetDoublev, GLenum pname, GLdouble *data) \
	WB_GL__GLPROC(GLenum, GetError, void) \
	WB_GL__GLPROC(void, GetFloatv, GLenum pname, GLfloat *data) \
	WB_GL__GLPROC(void, GetIntegerv, GLenum pname, GLint *data) \
	WB_GL__GLPROC(const GLubyte *, GetString, GLenum name) \
	WB_GL__GLPROC(void, GetTexImage, GLenum target, GLint level, GLenum format, GLenum type, void *pixels) \
	WB_GL__GLPROC(void, GetTexParameterfv, GLenum target, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetTexParameteriv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetTexLevelParameterfv, GLenum target, GLint level, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetTexLevelParameteriv, GLenum target, GLint level, GLenum pname, GLint *params) \
	WB_GL__GLPROC(GLboolean, IsEnabled, GLenum cap) \
	WB_GL__GLPROC(void, DepthRange, GLdouble near, GLdouble far) \
	WB_GL__GLPROC(void, Viewport, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, DrawArrays, GLenum mode, GLint first, GLsizei count) \
	WB_GL__GLPROC(void, DrawElements, GLenum mode, GLsizei count, GLenum type, const void *indices) \
	WB_GL__GLPROC(void, PolygonOffset, GLfloat factor, GLfloat units) \
	WB_GL__GLPROC(void, CopyTexImage1D, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) \
	WB_GL__GLPROC(void, CopyTexImage2D, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) \
	WB_GL__GLPROC(void, CopyTexSubImage1D, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) \
	WB_GL__GLPROC(void, CopyTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, TexSubImage1D, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, TexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, BindTexture, GLenum target, GLuint texture) \
	WB_GL__GLPROC(void, DeleteTextures, GLsizei n, const GLuint *textures) \
	WB_GL__GLPROC(void, GenTextures, GLsizei n, GLuint *textures) \
	WB_GL__GLPROC(GLboolean, IsTexture, GLuint texture) \
	WB_GL__GLPROC(void, DrawRangeElements, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) \
	WB_GL__GLPROC(void, TexImage3D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, TexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, CopyTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, ActiveTexture, GLenum texture) \
	WB_GL__GLPROC(void, SampleCoverage, GLfloat value, GLboolean invert) \
	WB_GL__GLPROC(void, CompressedTexImage3D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTexImage2D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTexImage1D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTexSubImage1D, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, GetCompressedTexImage, GLenum target, GLint level, void *img) \
	WB_GL__GLPROC(void, BlendFuncSeparate, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) \
	WB_GL__GLPROC(void, MultiDrawArrays, GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount) \
	WB_GL__GLPROC(void, MultiDrawElements, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount) \
	WB_GL__GLPROC(void, PointParameterf, GLenum pname, GLfloat param) \
	WB_GL__GLPROC(void, PointParameterfv, GLenum pname, const GLfloat *params) \
	WB_GL__GLPROC(void, PointParameteri, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, PointParameteriv, GLenum pname, const GLint *params) \
	WB_GL__GLPROC(void, BlendColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
	WB_GL__GLPROC(void, BlendEquation, GLenum mode) \
	WB_GL__GLPROC(void, GenQueries, GLsizei n, GLuint *ids) \
	WB_GL__GLPROC(void, DeleteQueries, GLsizei n, const GLuint *ids) \
	WB_GL__GLPROC(GLboolean, IsQuery, GLuint id) \
	WB_GL__GLPROC(void, BeginQuery, GLenum target, GLuint id) \
	WB_GL__GLPROC(void, EndQuery, GLenum target) \
	WB_GL__GLPROC(void, GetQueryiv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetQueryObjectiv, GLuint id, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetQueryObjectuiv, GLuint id, GLenum pname, GLuint *params) \
	WB_GL__GLPROC(void, BindBuffer, GLenum target, GLuint buffer) \
	WB_GL__GLPROC(void, DeleteBuffers, GLsizei n, const GLuint *buffers) \
	WB_GL__GLPROC(void, GenBuffers, GLsizei n, GLuint *buffers) \
	WB_GL__GLPROC(GLboolean, IsBuffer, GLuint buffer) \
	WB_GL__GLPROC(void, BufferData, GLenum target, GLsizeiptr size, const void *data, GLenum usage) \
	WB_GL__GLPROC(void, BufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void *data) \
	WB_GL__GLPROC(void, GetBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, void *data) \
	WB_GL__GLPROC(void *, MapBuffer, GLenum target, GLenum access) \
	WB_GL__GLPROC(GLboolean, UnmapBuffer, GLenum target) \
	WB_GL__GLPROC(void, GetBufferParameteriv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetBufferPointerv, GLenum target, GLenum pname, void **params) \
	WB_GL__GLPROC(void, BlendEquationSeparate, GLenum modeRGB, GLenum modeAlpha) \
	WB_GL__GLPROC(void, DrawBuffers, GLsizei n, const GLenum *bufs) \
	WB_GL__GLPROC(void, StencilOpSeparate, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) \
	WB_GL__GLPROC(void, StencilFuncSeparate, GLenum face, GLenum func, GLint ref, GLuint mask) \
	WB_GL__GLPROC(void, StencilMaskSeparate, GLenum face, GLuint mask) \
	WB_GL__GLPROC(void, AttachShader, GLuint program, GLuint shader) \
	WB_GL__GLPROC(void, BindAttribLocation, GLuint program, GLuint index, const GLchar *name) \
	WB_GL__GLPROC(void, CompileShader, GLuint shader) \
	WB_GL__GLPROC(GLuint, CreateProgram, void) \
	WB_GL__GLPROC(GLuint, CreateShader, GLenum type) \
	WB_GL__GLPROC(void, DeleteProgram, GLuint program) \
	WB_GL__GLPROC(void, DeleteShader, GLuint shader) \
	WB_GL__GLPROC(void, DetachShader, GLuint program, GLuint shader) \
	WB_GL__GLPROC(void, DisableVertexAttribArray, GLuint index) \
	WB_GL__GLPROC(void, EnableVertexAttribArray, GLuint index) \
	WB_GL__GLPROC(void, GetActiveAttrib, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) \
	WB_GL__GLPROC(void, GetActiveUniform, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) \
	WB_GL__GLPROC(void, GetAttachedShaders, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) \
	WB_GL__GLPROC(GLint, GetAttribLocation, GLuint program, const GLchar *name) \
	WB_GL__GLPROC(void, GetProgramiv, GLuint program, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
	WB_GL__GLPROC(void, GetShaderiv, GLuint shader, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
	WB_GL__GLPROC(void, GetShaderSource, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) \
	WB_GL__GLPROC(GLint, GetUniformLocation, GLuint program, const GLchar *name) \
	WB_GL__GLPROC(void, GetUniformfv, GLuint program, GLint location, GLfloat *params) \
	WB_GL__GLPROC(void, GetUniformiv, GLuint program, GLint location, GLint *params) \
	WB_GL__GLPROC(void, GetVertexAttribdv, GLuint index, GLenum pname, GLdouble *params) \
	WB_GL__GLPROC(void, GetVertexAttribfv, GLuint index, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetVertexAttribiv, GLuint index, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetVertexAttribPointerv, GLuint index, GLenum pname, void **pointer) \
	WB_GL__GLPROC(GLboolean, IsProgram, GLuint program) \
	WB_GL__GLPROC(GLboolean, IsShader, GLuint shader) \
	WB_GL__GLPROC(void, LinkProgram, GLuint program) \
	WB_GL__GLPROC(void, ShaderSource, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) \
	WB_GL__GLPROC(void, UseProgram, GLuint program) \
	WB_GL__GLPROC(void, Uniform1f, GLint location, GLfloat v0) \
	WB_GL__GLPROC(void, Uniform2f, GLint location, GLfloat v0, GLfloat v1) \
	WB_GL__GLPROC(void, Uniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
	WB_GL__GLPROC(void, Uniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
	WB_GL__GLPROC(void, Uniform1i, GLint location, GLint v0) \
	WB_GL__GLPROC(void, Uniform2i, GLint location, GLint v0, GLint v1) \
	WB_GL__GLPROC(void, Uniform3i, GLint location, GLint v0, GLint v1, GLint v2) \
	WB_GL__GLPROC(void, Uniform4i, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
	WB_GL__GLPROC(void, Uniform1fv, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, Uniform2fv, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, Uniform3fv, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, Uniform4fv, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, Uniform1iv, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, Uniform2iv, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, Uniform3iv, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, Uniform4iv, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, UniformMatrix2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ValidateProgram, GLuint program) \
	WB_GL__GLPROC(void, VertexAttrib1d, GLuint index, GLdouble x) \
	WB_GL__GLPROC(void, VertexAttrib1dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttrib1f, GLuint index, GLfloat x) \
	WB_GL__GLPROC(void, VertexAttrib1fv, GLuint index, const GLfloat *v) \
	WB_GL__GLPROC(void, VertexAttrib1s, GLuint index, GLshort x) \
	WB_GL__GLPROC(void, VertexAttrib1sv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttrib2d, GLuint index, GLdouble x, GLdouble y) \
	WB_GL__GLPROC(void, VertexAttrib2dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttrib2f, GLuint index, GLfloat x, GLfloat y) \
	WB_GL__GLPROC(void, VertexAttrib2fv, GLuint index, const GLfloat *v) \
	WB_GL__GLPROC(void, VertexAttrib2s, GLuint index, GLshort x, GLshort y) \
	WB_GL__GLPROC(void, VertexAttrib2sv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttrib3d, GLuint index, GLdouble x, GLdouble y, GLdouble z) \
	WB_GL__GLPROC(void, VertexAttrib3dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttrib3f, GLuint index, GLfloat x, GLfloat y, GLfloat z) \
	WB_GL__GLPROC(void, VertexAttrib3fv, GLuint index, const GLfloat *v) \
	WB_GL__GLPROC(void, VertexAttrib3s, GLuint index, GLshort x, GLshort y, GLshort z) \
	WB_GL__GLPROC(void, VertexAttrib3sv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttrib4Nbv, GLuint index, const GLbyte *v) \
	WB_GL__GLPROC(void, VertexAttrib4Niv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttrib4Nsv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttrib4Nub, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) \
	WB_GL__GLPROC(void, VertexAttrib4Nubv, GLuint index, const GLubyte *v) \
	WB_GL__GLPROC(void, VertexAttrib4Nuiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttrib4Nusv, GLuint index, const GLushort *v) \
	WB_GL__GLPROC(void, VertexAttrib4bv, GLuint index, const GLbyte *v) \
	WB_GL__GLPROC(void, VertexAttrib4d, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) \
	WB_GL__GLPROC(void, VertexAttrib4dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttrib4f, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) \
	WB_GL__GLPROC(void, VertexAttrib4fv, GLuint index, const GLfloat *v) \
	WB_GL__GLPROC(void, VertexAttrib4iv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttrib4s, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) \
	WB_GL__GLPROC(void, VertexAttrib4sv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttrib4ubv, GLuint index, const GLubyte *v) \
	WB_GL__GLPROC(void, VertexAttrib4uiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttrib4usv, GLuint index, const GLushort *v) \
	WB_GL__GLPROC(void, VertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) \
	WB_GL__GLPROC(void, UniformMatrix2x3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix3x2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix2x4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix4x2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix3x4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, UniformMatrix4x3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \


#define WB_GL__PROC_LIST_COMPAT \
	WB_GL__GLPROC(void, ColorMaski, GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) \
	WB_GL__GLPROC(void, GetBooleani_v, GLenum target, GLuint index, GLboolean *data) \
	WB_GL__GLPROC(void, GetIntegeri_v, GLenum target, GLuint index, GLint *data) \
	WB_GL__GLPROC(void, Enablei, GLenum target, GLuint index) \
	WB_GL__GLPROC(void, Disablei, GLenum target, GLuint index) \
	WB_GL__GLPROC(GLboolean, IsEnabledi, GLenum target, GLuint index) \
	WB_GL__GLPROC(void, BeginTransformFeedback, GLenum primitiveMode) \
	WB_GL__GLPROC(void, EndTransformFeedback, void) \
	WB_GL__GLPROC(void, BindBufferRange, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) \
	WB_GL__GLPROC(void, BindBufferBase, GLenum target, GLuint index, GLuint buffer) \
	WB_GL__GLPROC(void, TransformFeedbackVaryings, GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode) \
	WB_GL__GLPROC(void, GetTransformFeedbackVarying, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) \
	WB_GL__GLPROC(void, ClampColor, GLenum target, GLenum clamp) \
	WB_GL__GLPROC(void, BeginConditionalRender, GLuint id, GLenum mode) \
	WB_GL__GLPROC(void, EndConditionalRender, void) \
	WB_GL__GLPROC(void, VertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) \
	WB_GL__GLPROC(void, GetVertexAttribIiv, GLuint index, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetVertexAttribIuiv, GLuint index, GLenum pname, GLuint *params) \
	WB_GL__GLPROC(void, VertexAttribI1i, GLuint index, GLint x) \
	WB_GL__GLPROC(void, VertexAttribI2i, GLuint index, GLint x, GLint y) \
	WB_GL__GLPROC(void, VertexAttribI3i, GLuint index, GLint x, GLint y, GLint z) \
	WB_GL__GLPROC(void, VertexAttribI4i, GLuint index, GLint x, GLint y, GLint z, GLint w) \
	WB_GL__GLPROC(void, VertexAttribI1ui, GLuint index, GLuint x) \
	WB_GL__GLPROC(void, VertexAttribI2ui, GLuint index, GLuint x, GLuint y) \
	WB_GL__GLPROC(void, VertexAttribI3ui, GLuint index, GLuint x, GLuint y, GLuint z) \
	WB_GL__GLPROC(void, VertexAttribI4ui, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) \
	WB_GL__GLPROC(void, VertexAttribI1iv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttribI2iv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttribI3iv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttribI4iv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, VertexAttribI1uiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttribI2uiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttribI3uiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttribI4uiv, GLuint index, const GLuint *v) \
	WB_GL__GLPROC(void, VertexAttribI4bv, GLuint index, const GLbyte *v) \
	WB_GL__GLPROC(void, VertexAttribI4sv, GLuint index, const GLshort *v) \
	WB_GL__GLPROC(void, VertexAttribI4ubv, GLuint index, const GLubyte *v) \
	WB_GL__GLPROC(void, VertexAttribI4usv, GLuint index, const GLushort *v) \
	WB_GL__GLPROC(void, GetUniformuiv, GLuint program, GLint location, GLuint *params) \
	WB_GL__GLPROC(void, BindFragDataLocation, GLuint program, GLuint color, const GLchar *name) \
	WB_GL__GLPROC(GLint, GetFragDataLocation, GLuint program, const GLchar *name) \
	WB_GL__GLPROC(void, Uniform1ui, GLint location, GLuint v0) \
	WB_GL__GLPROC(void, Uniform2ui, GLint location, GLuint v0, GLuint v1) \
	WB_GL__GLPROC(void, Uniform3ui, GLint location, GLuint v0, GLuint v1, GLuint v2) \
	WB_GL__GLPROC(void, Uniform4ui, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) \
	WB_GL__GLPROC(void, Uniform1uiv, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, Uniform2uiv, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, Uniform3uiv, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, Uniform4uiv, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, TexParameterIiv, GLenum target, GLenum pname, const GLint *params) \
	WB_GL__GLPROC(void, TexParameterIuiv, GLenum target, GLenum pname, const GLuint *params) \
	WB_GL__GLPROC(void, GetTexParameterIiv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetTexParameterIuiv, GLenum target, GLenum pname, GLuint *params) \
	WB_GL__GLPROC(void, ClearBufferiv, GLenum buffer, GLint drawbuffer, const GLint *value) \
	WB_GL__GLPROC(void, ClearBufferuiv, GLenum buffer, GLint drawbuffer, const GLuint *value) \
	WB_GL__GLPROC(void, ClearBufferfv, GLenum buffer, GLint drawbuffer, const GLfloat *value) \
	WB_GL__GLPROC(void, ClearBufferfi, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) \
	WB_GL__GLPROC(const GLubyte *, GetStringi, GLenum name, GLuint index) \
	WB_GL__GLPROC(GLboolean, IsRenderbuffer, GLuint renderbuffer) \
	WB_GL__GLPROC(void, BindRenderbuffer, GLenum target, GLuint renderbuffer) \
	WB_GL__GLPROC(void, DeleteRenderbuffers, GLsizei n, const GLuint *renderbuffers) \
	WB_GL__GLPROC(void, GenRenderbuffers, GLsizei n, GLuint *renderbuffers) \
	WB_GL__GLPROC(void, RenderbufferStorage, GLenum target, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, GetRenderbufferParameteriv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(GLboolean, IsFramebuffer, GLuint framebuffer) \
	WB_GL__GLPROC(void, BindFramebuffer, GLenum target, GLuint framebuffer) \
	WB_GL__GLPROC(void, DeleteFramebuffers, GLsizei n, const GLuint *framebuffers) \
	WB_GL__GLPROC(void, GenFramebuffers, GLsizei n, GLuint *framebuffers) \
	WB_GL__GLPROC(GLenum, CheckFramebufferStatus, GLenum target) \
	WB_GL__GLPROC(void, FramebufferTexture1D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
	WB_GL__GLPROC(void, FramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
	WB_GL__GLPROC(void, FramebufferTexture3D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) \
	WB_GL__GLPROC(void, FramebufferRenderbuffer, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) \
	WB_GL__GLPROC(void, GetFramebufferAttachmentParameteriv, GLenum target, GLenum attachment, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GenerateMipmap, GLenum target) \
	WB_GL__GLPROC(void, BlitFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) \
	WB_GL__GLPROC(void, RenderbufferStorageMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, FramebufferTextureLayer, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) \
	WB_GL__GLPROC(void *, MapBufferRange, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) \
	WB_GL__GLPROC(void, FlushMappedBufferRange, GLenum target, GLintptr offset, GLsizeiptr length) \
	WB_GL__GLPROC(void, BindVertexArray, GLuint array) \
	WB_GL__GLPROC(void, DeleteVertexArrays, GLsizei n, const GLuint *arrays) \
	WB_GL__GLPROC(void, GenVertexArrays, GLsizei n, GLuint *arrays) \
	WB_GL__GLPROC(GLboolean, IsVertexArray, GLuint array) \
	WB_GL__GLPROC(void, DrawArraysInstanced, GLenum mode, GLint first, GLsizei count, GLsizei instancecount) \
	WB_GL__GLPROC(void, DrawElementsInstanced, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) \
	WB_GL__GLPROC(void, TexBuffer, GLenum target, GLenum internalformat, GLuint buffer) \
	WB_GL__GLPROC(void, PrimitiveRestartIndex, GLuint index) \
	WB_GL__GLPROC(void, CopyBufferSubData, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) \
	WB_GL__GLPROC(void, GetUniformIndices, GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices) \
	WB_GL__GLPROC(void, GetActiveUniformsiv, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetActiveUniformName, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName) \
	WB_GL__GLPROC(GLuint, GetUniformBlockIndex, GLuint program, const GLchar *uniformBlockName) \
	WB_GL__GLPROC(void, GetActiveUniformBlockiv, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetActiveUniformBlockName, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) \
	WB_GL__GLPROC(void, UniformBlockBinding, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) \


#define WB_GL__PROC_LIST_CORE \
	WB_GL__GLPROC(void, DrawElementsBaseVertex, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) \
	WB_GL__GLPROC(void, DrawRangeElementsBaseVertex, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex) \
	WB_GL__GLPROC(void, DrawElementsInstancedBaseVertex, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex) \
	WB_GL__GLPROC(void, MultiDrawElementsBaseVertex, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex) \
	WB_GL__GLPROC(void, ProvokingVertex, GLenum mode) \
	WB_GL__GLPROC(GLsync, FenceSync, GLenum condition, GLbitfield flags) \
	WB_GL__GLPROC(GLboolean, IsSync, GLsync sync) \
	WB_GL__GLPROC(void, DeleteSync, GLsync sync) \
	WB_GL__GLPROC(GLenum, ClientWaitSync, GLsync sync, GLbitfield flags, GLuint64 timeout) \
	WB_GL__GLPROC(void, WaitSync, GLsync sync, GLbitfield flags, GLuint64 timeout) \
	WB_GL__GLPROC(void, GetInteger64v, GLenum pname, GLint64 *data) \
	WB_GL__GLPROC(void, GetSynciv, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values) \
	WB_GL__GLPROC(void, GetInteger64i_v, GLenum target, GLuint index, GLint64 *data) \
	WB_GL__GLPROC(void, GetBufferParameteri64v, GLenum target, GLenum pname, GLint64 *params) \
	WB_GL__GLPROC(void, FramebufferTexture, GLenum target, GLenum attachment, GLuint texture, GLint level) \
	WB_GL__GLPROC(void, TexImage2DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, TexImage3DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, GetMultisamplefv, GLenum pname, GLuint index, GLfloat *val) \
	WB_GL__GLPROC(void, SampleMaski, GLuint maskNumber, GLbitfield mask) \
	WB_GL__GLPROC(void, BindFragDataLocationIndexed, GLuint program, GLuint colorNumber, GLuint index, const GLchar *name) \
	WB_GL__GLPROC(GLint, GetFragDataIndex, GLuint program, const GLchar *name) \
	WB_GL__GLPROC(void, GenSamplers, GLsizei count, GLuint *samplers) \
	WB_GL__GLPROC(void, DeleteSamplers, GLsizei count, const GLuint *samplers) \
	WB_GL__GLPROC(GLboolean, IsSampler, GLuint sampler) \
	WB_GL__GLPROC(void, BindSampler, GLuint unit, GLuint sampler) \
	WB_GL__GLPROC(void, SamplerParameteri, GLuint sampler, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, SamplerParameteriv, GLuint sampler, GLenum pname, const GLint *param) \
	WB_GL__GLPROC(void, SamplerParameterf, GLuint sampler, GLenum pname, GLfloat param) \
	WB_GL__GLPROC(void, SamplerParameterfv, GLuint sampler, GLenum pname, const GLfloat *param) \
	WB_GL__GLPROC(void, SamplerParameterIiv, GLuint sampler, GLenum pname, const GLint *param) \
	WB_GL__GLPROC(void, SamplerParameterIuiv, GLuint sampler, GLenum pname, const GLuint *param) \
	WB_GL__GLPROC(void, GetSamplerParameteriv, GLuint sampler, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetSamplerParameterIiv, GLuint sampler, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetSamplerParameterfv, GLuint sampler, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetSamplerParameterIuiv, GLuint sampler, GLenum pname, GLuint *params) \
	WB_GL__GLPROC(void, QueryCounter, GLuint id, GLenum target) \
	WB_GL__GLPROC(void, GetQueryObjecti64v, GLuint id, GLenum pname, GLint64 *params) \
	WB_GL__GLPROC(void, GetQueryObjectui64v, GLuint id, GLenum pname, GLuint64 *params) \
	WB_GL__GLPROC(void, VertexAttribDivisor, GLuint index, GLuint divisor) \
	WB_GL__GLPROC(void, VertexAttribP1ui, GLuint index, GLenum type, GLboolean normalized, GLuint value) \
	WB_GL__GLPROC(void, VertexAttribP1uiv, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) \
	WB_GL__GLPROC(void, VertexAttribP2ui, GLuint index, GLenum type, GLboolean normalized, GLuint value) \
	WB_GL__GLPROC(void, VertexAttribP2uiv, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) \
	WB_GL__GLPROC(void, VertexAttribP3ui, GLuint index, GLenum type, GLboolean normalized, GLuint value) \
	WB_GL__GLPROC(void, VertexAttribP3uiv, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) \
	WB_GL__GLPROC(void, VertexAttribP4ui, GLuint index, GLenum type, GLboolean normalized, GLuint value) \
	WB_GL__GLPROC(void, VertexAttribP4uiv, GLuint index, GLenum type, GLboolean normalized, const GLuint *value) \
	WB_GL__GLPROC(void, VertexP2ui, GLenum type, GLuint value) \
	WB_GL__GLPROC(void, VertexP2uiv, GLenum type, const GLuint *value) \
	WB_GL__GLPROC(void, VertexP3ui, GLenum type, GLuint value) \
	WB_GL__GLPROC(void, VertexP3uiv, GLenum type, const GLuint *value) \
	WB_GL__GLPROC(void, VertexP4ui, GLenum type, GLuint value) \
	WB_GL__GLPROC(void, VertexP4uiv, GLenum type, const GLuint *value) \
	WB_GL__GLPROC(void, TexCoordP1ui, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, TexCoordP1uiv, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, TexCoordP2ui, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, TexCoordP2uiv, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, TexCoordP3ui, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, TexCoordP3uiv, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, TexCoordP4ui, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, TexCoordP4uiv, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, MultiTexCoordP1ui, GLenum texture, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, MultiTexCoordP1uiv, GLenum texture, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, MultiTexCoordP2ui, GLenum texture, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, MultiTexCoordP2uiv, GLenum texture, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, MultiTexCoordP3ui, GLenum texture, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, MultiTexCoordP3uiv, GLenum texture, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, MultiTexCoordP4ui, GLenum texture, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, MultiTexCoordP4uiv, GLenum texture, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, NormalP3ui, GLenum type, GLuint coords) \
	WB_GL__GLPROC(void, NormalP3uiv, GLenum type, const GLuint *coords) \
	WB_GL__GLPROC(void, ColorP3ui, GLenum type, GLuint color) \
	WB_GL__GLPROC(void, ColorP3uiv, GLenum type, const GLuint *color) \
	WB_GL__GLPROC(void, ColorP4ui, GLenum type, GLuint color) \
	WB_GL__GLPROC(void, ColorP4uiv, GLenum type, const GLuint *color) \
	WB_GL__GLPROC(void, SecondaryColorP3ui, GLenum type, GLuint color) \
	WB_GL__GLPROC(void, SecondaryColorP3uiv, GLenum type, const GLuint *color) \


#define WB_GL__PROC_LIST_MODERN \
	WB_GL__GLPROC(void, MinSampleShading, GLfloat value) \
	WB_GL__GLPROC(void, BlendEquationi, GLuint buf, GLenum mode) \
	WB_GL__GLPROC(void, BlendEquationSeparatei, GLuint buf, GLenum modeRGB, GLenum modeAlpha) \
	WB_GL__GLPROC(void, BlendFunci, GLuint buf, GLenum src, GLenum dst) \
	WB_GL__GLPROC(void, BlendFuncSeparatei, GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) \
	WB_GL__GLPROC(void, DrawArraysIndirect, GLenum mode, const void *indirect) \
	WB_GL__GLPROC(void, DrawElementsIndirect, GLenum mode, GLenum type, const void *indirect) \
	WB_GL__GLPROC(void, Uniform1d, GLint location, GLdouble x) \
	WB_GL__GLPROC(void, Uniform2d, GLint location, GLdouble x, GLdouble y) \
	WB_GL__GLPROC(void, Uniform3d, GLint location, GLdouble x, GLdouble y, GLdouble z) \
	WB_GL__GLPROC(void, Uniform4d, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) \
	WB_GL__GLPROC(void, Uniform1dv, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, Uniform2dv, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, Uniform3dv, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, Uniform4dv, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix2dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix3dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix4dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix2x3dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix2x4dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix3x2dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix3x4dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix4x2dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, UniformMatrix4x3dv, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, GetUniformdv, GLuint program, GLint location, GLdouble *params) \
	WB_GL__GLPROC(GLint, GetSubroutineUniformLocation, GLuint program, GLenum shadertype, const GLchar *name) \
	WB_GL__GLPROC(GLuint, GetSubroutineIndex, GLuint program, GLenum shadertype, const GLchar *name) \
	WB_GL__GLPROC(void, GetActiveSubroutineUniformiv, GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values) \
	WB_GL__GLPROC(void, GetActiveSubroutineUniformName, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name) \
	WB_GL__GLPROC(void, GetActiveSubroutineName, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name) \
	WB_GL__GLPROC(void, UniformSubroutinesuiv, GLenum shadertype, GLsizei count, const GLuint *indices) \
	WB_GL__GLPROC(void, GetUniformSubroutineuiv, GLenum shadertype, GLint location, GLuint *params) \
	WB_GL__GLPROC(void, GetProgramStageiv, GLuint program, GLenum shadertype, GLenum pname, GLint *values) \
	WB_GL__GLPROC(void, PatchParameteri, GLenum pname, GLint value) \
	WB_GL__GLPROC(void, PatchParameterfv, GLenum pname, const GLfloat *values) \
	WB_GL__GLPROC(void, BindTransformFeedback, GLenum target, GLuint id) \
	WB_GL__GLPROC(void, DeleteTransformFeedbacks, GLsizei n, const GLuint *ids) \
	WB_GL__GLPROC(void, GenTransformFeedbacks, GLsizei n, GLuint *ids) \
	WB_GL__GLPROC(GLboolean, IsTransformFeedback, GLuint id) \
	WB_GL__GLPROC(void, PauseTransformFeedback, void) \
	WB_GL__GLPROC(void, ResumeTransformFeedback, void) \
	WB_GL__GLPROC(void, DrawTransformFeedback, GLenum mode, GLuint id) \
	WB_GL__GLPROC(void, DrawTransformFeedbackStream, GLenum mode, GLuint id, GLuint stream) \
	WB_GL__GLPROC(void, BeginQueryIndexed, GLenum target, GLuint index, GLuint id) \
	WB_GL__GLPROC(void, EndQueryIndexed, GLenum target, GLuint index) \
	WB_GL__GLPROC(void, GetQueryIndexediv, GLenum target, GLuint index, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, ReleaseShaderCompiler, void) \
	WB_GL__GLPROC(void, ShaderBinary, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) \
	WB_GL__GLPROC(void, GetShaderPrecisionFormat, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) \
	WB_GL__GLPROC(void, DepthRangef, GLfloat n, GLfloat f) \
	WB_GL__GLPROC(void, ClearDepthf, GLfloat d) \
	WB_GL__GLPROC(void, GetProgramBinary, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) \
	WB_GL__GLPROC(void, ProgramBinary, GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) \
	WB_GL__GLPROC(void, ProgramParameteri, GLuint program, GLenum pname, GLint value) \
	WB_GL__GLPROC(void, UseProgramStages, GLuint pipeline, GLbitfield stages, GLuint program) \
	WB_GL__GLPROC(void, ActiveShaderProgram, GLuint pipeline, GLuint program) \
	WB_GL__GLPROC(GLuint, CreateShaderProgramv, GLenum type, GLsizei count, const GLchar *const*strings) \
	WB_GL__GLPROC(void, BindProgramPipeline, GLuint pipeline) \
	WB_GL__GLPROC(void, DeleteProgramPipelines, GLsizei n, const GLuint *pipelines) \
	WB_GL__GLPROC(void, GenProgramPipelines, GLsizei n, GLuint *pipelines) \
	WB_GL__GLPROC(GLboolean, IsProgramPipeline, GLuint pipeline) \
	WB_GL__GLPROC(void, GetProgramPipelineiv, GLuint pipeline, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, ProgramUniform1i, GLuint program, GLint location, GLint v0) \
	WB_GL__GLPROC(void, ProgramUniform1iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, ProgramUniform1f, GLuint program, GLint location, GLfloat v0) \
	WB_GL__GLPROC(void, ProgramUniform1fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniform1d, GLuint program, GLint location, GLdouble v0) \
	WB_GL__GLPROC(void, ProgramUniform1dv, GLuint program, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniform1ui, GLuint program, GLint location, GLuint v0) \
	WB_GL__GLPROC(void, ProgramUniform1uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, ProgramUniform2i, GLuint program, GLint location, GLint v0, GLint v1) \
	WB_GL__GLPROC(void, ProgramUniform2iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, ProgramUniform2f, GLuint program, GLint location, GLfloat v0, GLfloat v1) \
	WB_GL__GLPROC(void, ProgramUniform2fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniform2d, GLuint program, GLint location, GLdouble v0, GLdouble v1) \
	WB_GL__GLPROC(void, ProgramUniform2dv, GLuint program, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniform2ui, GLuint program, GLint location, GLuint v0, GLuint v1) \
	WB_GL__GLPROC(void, ProgramUniform2uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, ProgramUniform3i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2) \
	WB_GL__GLPROC(void, ProgramUniform3iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, ProgramUniform3f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
	WB_GL__GLPROC(void, ProgramUniform3fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniform3d, GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2) \
	WB_GL__GLPROC(void, ProgramUniform3dv, GLuint program, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniform3ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) \
	WB_GL__GLPROC(void, ProgramUniform3uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, ProgramUniform4i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
	WB_GL__GLPROC(void, ProgramUniform4iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
	WB_GL__GLPROC(void, ProgramUniform4f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
	WB_GL__GLPROC(void, ProgramUniform4fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniform4d, GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) \
	WB_GL__GLPROC(void, ProgramUniform4dv, GLuint program, GLint location, GLsizei count, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniform4ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) \
	WB_GL__GLPROC(void, ProgramUniform4uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2x3dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3x2dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix2x4dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4x2dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix3x4dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ProgramUniformMatrix4x3dv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) \
	WB_GL__GLPROC(void, ValidateProgramPipeline, GLuint pipeline) \
	WB_GL__GLPROC(void, GetProgramPipelineInfoLog, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
	WB_GL__GLPROC(void, VertexAttribL1d, GLuint index, GLdouble x) \
	WB_GL__GLPROC(void, VertexAttribL2d, GLuint index, GLdouble x, GLdouble y) \
	WB_GL__GLPROC(void, VertexAttribL3d, GLuint index, GLdouble x, GLdouble y, GLdouble z) \
	WB_GL__GLPROC(void, VertexAttribL4d, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) \
	WB_GL__GLPROC(void, VertexAttribL1dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttribL2dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttribL3dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttribL4dv, GLuint index, const GLdouble *v) \
	WB_GL__GLPROC(void, VertexAttribLPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) \
	WB_GL__GLPROC(void, GetVertexAttribLdv, GLuint index, GLenum pname, GLdouble *params) \
	WB_GL__GLPROC(void, ViewportArrayv, GLuint first, GLsizei count, const GLfloat *v) \
	WB_GL__GLPROC(void, ViewportIndexedf, GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) \
	WB_GL__GLPROC(void, ViewportIndexedfv, GLuint index, const GLfloat *v) \
	WB_GL__GLPROC(void, ScissorArrayv, GLuint first, GLsizei count, const GLint *v) \
	WB_GL__GLPROC(void, ScissorIndexed, GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, ScissorIndexedv, GLuint index, const GLint *v) \
	WB_GL__GLPROC(void, DepthRangeArrayv, GLuint first, GLsizei count, const GLdouble *v) \
	WB_GL__GLPROC(void, DepthRangeIndexed, GLuint index, GLdouble n, GLdouble f) \
	WB_GL__GLPROC(void, GetFloati_v, GLenum target, GLuint index, GLfloat *data) \
	WB_GL__GLPROC(void, GetDoublei_v, GLenum target, GLuint index, GLdouble *data) \
	WB_GL__GLPROC(void, DrawArraysInstancedBaseInstance, GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance) \
	WB_GL__GLPROC(void, DrawElementsInstancedBaseInstance, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance) \
	WB_GL__GLPROC(void, DrawElementsInstancedBaseVertexBaseInstance, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance) \
	WB_GL__GLPROC(void, GetInternalformativ, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params) \
	WB_GL__GLPROC(void, GetActiveAtomicCounterBufferiv, GLuint program, GLuint bufferIndex, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, BindImageTexture, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) \
	WB_GL__GLPROC(void, MemoryBarrier, GLbitfield barriers) \
	WB_GL__GLPROC(void, TexStorage1D, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) \
	WB_GL__GLPROC(void, TexStorage2D, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, TexStorage3D, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) \
	WB_GL__GLPROC(void, DrawTransformFeedbackInstanced, GLenum mode, GLuint id, GLsizei instancecount) \
	WB_GL__GLPROC(void, DrawTransformFeedbackStreamInstanced, GLenum mode, GLuint id, GLuint stream, GLsizei instancecount) \
	WB_GL__GLPROC(void, ClearBufferData, GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void, ClearBufferSubData, GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void, DispatchCompute, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) \
	WB_GL__GLPROC(void, DispatchComputeIndirect, GLintptr indirect) \
	WB_GL__GLPROC(void, CopyImageSubData, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) \
	WB_GL__GLPROC(void, FramebufferParameteri, GLenum target, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, GetFramebufferParameteriv, GLenum target, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetInternalformati64v, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params) \
	WB_GL__GLPROC(void, InvalidateTexSubImage, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth) \
	WB_GL__GLPROC(void, InvalidateTexImage, GLuint texture, GLint level) \
	WB_GL__GLPROC(void, InvalidateBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr length) \
	WB_GL__GLPROC(void, InvalidateBufferData, GLuint buffer) \
	WB_GL__GLPROC(void, InvalidateFramebuffer, GLenum target, GLsizei numAttachments, const GLenum *attachments) \
	WB_GL__GLPROC(void, InvalidateSubFramebuffer, GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, MultiDrawArraysIndirect, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride) \
	WB_GL__GLPROC(void, MultiDrawElementsIndirect, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride) \
	WB_GL__GLPROC(void, GetProgramInterfaceiv, GLuint program, GLenum programInterface, GLenum pname, GLint *params) \
	WB_GL__GLPROC(GLuint, GetProgramResourceIndex, GLuint program, GLenum programInterface, const GLchar *name) \
	WB_GL__GLPROC(void, GetProgramResourceName, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) \
	WB_GL__GLPROC(void, GetProgramResourceiv, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) \
	WB_GL__GLPROC(GLint, GetProgramResourceLocation, GLuint program, GLenum programInterface, const GLchar *name) \
	WB_GL__GLPROC(GLint, GetProgramResourceLocationIndex, GLuint program, GLenum programInterface, const GLchar *name) \
	WB_GL__GLPROC(void, ShaderStorageBlockBinding, GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) \
	WB_GL__GLPROC(void, TexBufferRange, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) \
	WB_GL__GLPROC(void, TexStorage2DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, TexStorage3DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, TextureView, GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) \
	WB_GL__GLPROC(void, BindVertexBuffer, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) \
	WB_GL__GLPROC(void, VertexAttribFormat, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexAttribIFormat, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexAttribLFormat, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexAttribBinding, GLuint attribindex, GLuint bindingindex) \
	WB_GL__GLPROC(void, VertexBindingDivisor, GLuint bindingindex, GLuint divisor) \
	WB_GL__GLPROC(void, DebugMessageControl, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled) \
	WB_GL__GLPROC(void, DebugMessageInsert, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf) \
	WB_GL__GLPROC(void, DebugMessageCallback, GLDEBUGPROC callback, const void *userParam) \
	WB_GL__GLPROC(GLuint, GetDebugMessageLog, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog) \
	WB_GL__GLPROC(void, PushDebugGroup, GLenum source, GLuint id, GLsizei length, const GLchar *message) \
	WB_GL__GLPROC(void, PopDebugGroup, void) \
	WB_GL__GLPROC(void, ObjectLabel, GLenum identifier, GLuint name, GLsizei length, const GLchar *label) \
	WB_GL__GLPROC(void, GetObjectLabel, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label) \
	WB_GL__GLPROC(void, ObjectPtrLabel, const void *ptr, GLsizei length, const GLchar *label) \
	WB_GL__GLPROC(void, GetObjectPtrLabel, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label) \
	WB_GL__GLPROC(void, BufferStorage, GLenum target, GLsizeiptr size, const void *data, GLbitfield flags) \
	WB_GL__GLPROC(void, ClearTexImage, GLuint texture, GLint level, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void, ClearTexSubImage, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void, BindBuffersBase, GLenum target, GLuint first, GLsizei count, const GLuint *buffers) \
	WB_GL__GLPROC(void, BindBuffersRange, GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes) \
	WB_GL__GLPROC(void, BindTextures, GLuint first, GLsizei count, const GLuint *textures) \
	WB_GL__GLPROC(void, BindSamplers, GLuint first, GLsizei count, const GLuint *samplers) \
	WB_GL__GLPROC(void, BindImageTextures, GLuint first, GLsizei count, const GLuint *textures) \
	WB_GL__GLPROC(void, BindVertexBuffers, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides) \
	WB_GL__GLPROC(void, ClipControl, GLenum origin, GLenum depth) \
	WB_GL__GLPROC(void, CreateTransformFeedbacks, GLsizei n, GLuint *ids) \
	WB_GL__GLPROC(void, TransformFeedbackBufferBase, GLuint xfb, GLuint index, GLuint buffer) \
	WB_GL__GLPROC(void, TransformFeedbackBufferRange, GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) \
	WB_GL__GLPROC(void, GetTransformFeedbackiv, GLuint xfb, GLenum pname, GLint *param) \
	WB_GL__GLPROC(void, GetTransformFeedbacki_v, GLuint xfb, GLenum pname, GLuint index, GLint *param) \
	WB_GL__GLPROC(void, GetTransformFeedbacki64_v, GLuint xfb, GLenum pname, GLuint index, GLint64 *param) \
	WB_GL__GLPROC(void, CreateBuffers, GLsizei n, GLuint *buffers) \
	WB_GL__GLPROC(void, NamedBufferStorage, GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags) \
	WB_GL__GLPROC(void, NamedBufferData, GLuint buffer, GLsizeiptr size, const void *data, GLenum usage) \
	WB_GL__GLPROC(void, NamedBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data) \
	WB_GL__GLPROC(void, CopyNamedBufferSubData, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) \
	WB_GL__GLPROC(void, ClearNamedBufferData, GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void, ClearNamedBufferSubData, GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data) \
	WB_GL__GLPROC(void *, MapNamedBuffer, GLuint buffer, GLenum access) \
	WB_GL__GLPROC(void *, MapNamedBufferRange, GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) \
	WB_GL__GLPROC(GLboolean, UnmapNamedBuffer, GLuint buffer) \
	WB_GL__GLPROC(void, FlushMappedNamedBufferRange, GLuint buffer, GLintptr offset, GLsizeiptr length) \
	WB_GL__GLPROC(void, GetNamedBufferParameteriv, GLuint buffer, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetNamedBufferParameteri64v, GLuint buffer, GLenum pname, GLint64 *params) \
	WB_GL__GLPROC(void, GetNamedBufferPointerv, GLuint buffer, GLenum pname, void **params) \
	WB_GL__GLPROC(void, GetNamedBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr size, void *data) \
	WB_GL__GLPROC(void, CreateFramebuffers, GLsizei n, GLuint *framebuffers) \
	WB_GL__GLPROC(void, NamedFramebufferRenderbuffer, GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) \
	WB_GL__GLPROC(void, NamedFramebufferParameteri, GLuint framebuffer, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, NamedFramebufferTexture, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) \
	WB_GL__GLPROC(void, NamedFramebufferTextureLayer, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) \
	WB_GL__GLPROC(void, NamedFramebufferDrawBuffer, GLuint framebuffer, GLenum buf) \
	WB_GL__GLPROC(void, NamedFramebufferDrawBuffers, GLuint framebuffer, GLsizei n, const GLenum *bufs) \
	WB_GL__GLPROC(void, NamedFramebufferReadBuffer, GLuint framebuffer, GLenum src) \
	WB_GL__GLPROC(void, InvalidateNamedFramebufferData, GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments) \
	WB_GL__GLPROC(void, InvalidateNamedFramebufferSubData, GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, ClearNamedFramebufferiv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value) \
	WB_GL__GLPROC(void, ClearNamedFramebufferuiv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value) \
	WB_GL__GLPROC(void, ClearNamedFramebufferfv, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value) \
	WB_GL__GLPROC(void, ClearNamedFramebufferfi, GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) \
	WB_GL__GLPROC(void, BlitNamedFramebuffer, GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) \
	WB_GL__GLPROC(GLenum, CheckNamedFramebufferStatus, GLuint framebuffer, GLenum target) \
	WB_GL__GLPROC(void, GetNamedFramebufferParameteriv, GLuint framebuffer, GLenum pname, GLint *param) \
	WB_GL__GLPROC(void, GetNamedFramebufferAttachmentParameteriv, GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, CreateRenderbuffers, GLsizei n, GLuint *renderbuffers) \
	WB_GL__GLPROC(void, NamedRenderbufferStorage, GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, NamedRenderbufferStorageMultisample, GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, GetNamedRenderbufferParameteriv, GLuint renderbuffer, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, CreateTextures, GLenum target, GLsizei n, GLuint *textures) \
	WB_GL__GLPROC(void, TextureBuffer, GLuint texture, GLenum internalformat, GLuint buffer) \
	WB_GL__GLPROC(void, TextureBufferRange, GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) \
	WB_GL__GLPROC(void, TextureStorage1D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) \
	WB_GL__GLPROC(void, TextureStorage2D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, TextureStorage3D, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) \
	WB_GL__GLPROC(void, TextureStorage2DMultisample, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, TextureStorage3DMultisample, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) \
	WB_GL__GLPROC(void, TextureSubImage1D, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, TextureSubImage2D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, TextureSubImage3D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) \
	WB_GL__GLPROC(void, CompressedTextureSubImage1D, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTextureSubImage2D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CompressedTextureSubImage3D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) \
	WB_GL__GLPROC(void, CopyTextureSubImage1D, GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) \
	WB_GL__GLPROC(void, CopyTextureSubImage2D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, CopyTextureSubImage3D, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
	WB_GL__GLPROC(void, TextureParameterf, GLuint texture, GLenum pname, GLfloat param) \
	WB_GL__GLPROC(void, TextureParameterfv, GLuint texture, GLenum pname, const GLfloat *param) \
	WB_GL__GLPROC(void, TextureParameteri, GLuint texture, GLenum pname, GLint param) \
	WB_GL__GLPROC(void, TextureParameterIiv, GLuint texture, GLenum pname, const GLint *params) \
	WB_GL__GLPROC(void, TextureParameterIuiv, GLuint texture, GLenum pname, const GLuint *params) \
	WB_GL__GLPROC(void, TextureParameteriv, GLuint texture, GLenum pname, const GLint *param) \
	WB_GL__GLPROC(void, GenerateTextureMipmap, GLuint texture) \
	WB_GL__GLPROC(void, BindTextureUnit, GLuint unit, GLuint texture) \
	WB_GL__GLPROC(void, GetTextureImage, GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(void, GetCompressedTextureImage, GLuint texture, GLint level, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(void, GetTextureLevelParameterfv, GLuint texture, GLint level, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetTextureLevelParameteriv, GLuint texture, GLint level, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetTextureParameterfv, GLuint texture, GLenum pname, GLfloat *params) \
	WB_GL__GLPROC(void, GetTextureParameterIiv, GLuint texture, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, GetTextureParameterIuiv, GLuint texture, GLenum pname, GLuint *params) \
	WB_GL__GLPROC(void, GetTextureParameteriv, GLuint texture, GLenum pname, GLint *params) \
	WB_GL__GLPROC(void, CreateVertexArrays, GLsizei n, GLuint *arrays) \
	WB_GL__GLPROC(void, DisableVertexArrayAttrib, GLuint vaobj, GLuint index) \
	WB_GL__GLPROC(void, EnableVertexArrayAttrib, GLuint vaobj, GLuint index) \
	WB_GL__GLPROC(void, VertexArrayElementBuffer, GLuint vaobj, GLuint buffer) \
	WB_GL__GLPROC(void, VertexArrayVertexBuffer, GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) \
	WB_GL__GLPROC(void, VertexArrayVertexBuffers, GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides) \
	WB_GL__GLPROC(void, VertexArrayAttribBinding, GLuint vaobj, GLuint attribindex, GLuint bindingindex) \
	WB_GL__GLPROC(void, VertexArrayAttribFormat, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexArrayAttribIFormat, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexArrayAttribLFormat, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) \
	WB_GL__GLPROC(void, VertexArrayBindingDivisor, GLuint vaobj, GLuint bindingindex, GLuint divisor) \
	WB_GL__GLPROC(void, GetVertexArrayiv, GLuint vaobj, GLenum pname, GLint *param) \
	WB_GL__GLPROC(void, GetVertexArrayIndexediv, GLuint vaobj, GLuint index, GLenum pname, GLint *param) \
	WB_GL__GLPROC(void, GetVertexArrayIndexed64iv, GLuint vaobj, GLuint index, GLenum pname, GLint64 *param) \
	WB_GL__GLPROC(void, CreateSamplers, GLsizei n, GLuint *samplers) \
	WB_GL__GLPROC(void, CreateProgramPipelines, GLsizei n, GLuint *pipelines) \
	WB_GL__GLPROC(void, CreateQueries, GLenum target, GLsizei n, GLuint *ids) \
	WB_GL__GLPROC(void, GetQueryBufferObjecti64v, GLuint id, GLuint buffer, GLenum pname, GLintptr offset) \
	WB_GL__GLPROC(void, GetQueryBufferObjectiv, GLuint id, GLuint buffer, GLenum pname, GLintptr offset) \
	WB_GL__GLPROC(void, GetQueryBufferObjectui64v, GLuint id, GLuint buffer, GLenum pname, GLintptr offset) \
	WB_GL__GLPROC(void, GetQueryBufferObjectuiv, GLuint id, GLuint buffer, GLenum pname, GLintptr offset) \
	WB_GL__GLPROC(void, MemoryBarrierByRegion, GLbitfield barriers) \
	WB_GL__GLPROC(void, GetTextureSubImage, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(void, GetCompressedTextureSubImage, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(GLenum, GetGraphicsResetStatus, void) \
	WB_GL__GLPROC(void, GetnCompressedTexImage, GLenum target, GLint lod, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(void, GetnTexImage, GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels) \
	WB_GL__GLPROC(void, GetnUniformdv, GLuint program, GLint location, GLsizei bufSize, GLdouble *params) \
	WB_GL__GLPROC(void, GetnUniformfv, GLuint program, GLint location, GLsizei bufSize, GLfloat *params) \
	WB_GL__GLPROC(void, GetnUniformiv, GLuint program, GLint location, GLsizei bufSize, GLint *params) \
	WB_GL__GLPROC(void, GetnUniformuiv, GLuint program, GLint location, GLsizei bufSize, GLuint *params) \
	WB_GL__GLPROC(void, ReadnPixels, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data) \
	WB_GL__GLPROC(void, GetnMapdv, GLenum target, GLenum query, GLsizei bufSize, GLdouble *v) \
	WB_GL__GLPROC(void, GetnMapfv, GLenum target, GLenum query, GLsizei bufSize, GLfloat *v) \
	WB_GL__GLPROC(void, GetnMapiv, GLenum target, GLenum query, GLsizei bufSize, GLint *v) \
	WB_GL__GLPROC(void, GetnPixelMapfv, GLenum map, GLsizei bufSize, GLfloat *values) \
	WB_GL__GLPROC(void, GetnPixelMapuiv, GLenum map, GLsizei bufSize, GLuint *values) \
	WB_GL__GLPROC(void, GetnPixelMapusv, GLenum map, GLsizei bufSize, GLushort *values) \
	WB_GL__GLPROC(void, GetnPolygonStipple, GLsizei bufSize, GLubyte *pattern) \
	WB_GL__GLPROC(void, GetnColorTable, GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table) \
	WB_GL__GLPROC(void, GetnConvolutionFilter, GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image) \
	WB_GL__GLPROC(void, GetnSeparableFilter, GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span) \
	WB_GL__GLPROC(void, GetnHistogram, GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values) \
	WB_GL__GLPROC(void, GetnMinmax, GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values) \
	WB_GL__GLPROC(void, TextureBarrier, void) \


#define WB_GL__GLPROC(ret, name, ...) \
	 typedef ret wbgl_##name##Proc(__VA_ARGS__); \
	 WB_GL_GLPROC_API wbgl_##name##Proc* gl##name;

#ifdef WB_GL_USE_LEGACY
WB_GL__PROC_LIST_LEGACY
#endif
#ifdef WB_GL_USE_COMPAT
WB_GL__PROC_LIST_COMPAT
#endif
#ifdef WB_GL_USE_CORE
WB_GL__PROC_LIST_CORE
#endif
#ifdef WB_GL_USE_MODERN
WB_GL__PROC_LIST_MODERN
#endif
#undef WB_GL__GLPROC

#ifdef WB_GL_IMPLEMENTATION

#define WB_GL__GLPROC(ret, name, ...) \
	 wbgl_##name##Proc* gl##name;

#ifdef WB_GL_USE_LEGACY
WB_GL__PROC_LIST_LEGACY
#endif
#ifdef WB_GL_USE_COMPAT
WB_GL__PROC_LIST_COMPAT
#endif
#ifdef WB_GL_USE_CORE
WB_GL__PROC_LIST_CORE
#endif
#ifdef WB_GL_USE_MODERN
WB_GL__PROC_LIST_MODERN
#endif
#undef WB_GL__GLPROC

WB_GL_LOADER_API
int wbgl_load_all(struct wbgl_ErrorContext* in_ctx)
{
	struct wbgl_ErrorContext* ctx;
	struct wbgl_ErrorContext lctx;
	if(!in_ctx) ctx = &lctx;
	else ctx = in_ctx;

#ifdef WB_GL_WIN32
	void* userdata = LoadLibraryA("opengl32.dll");
#else
	void* userdata = NULL;
#endif

#define WB_GL__GLPROC(ret, name, ...) gl##name = (wbgl_##name##Proc*)wbgl__load_proc("gl" #name, ctx, userdata);

	#ifdef WB_GL_USE_LEGACY
		WB_GL__PROC_LIST_LEGACY
	#endif

	#ifdef WB_GL_USE_COMPAT
		WB_GL__PROC_LIST_COMPAT
	#endif

	#ifdef WB_GL_USE_CORE
		WB_GL__PROC_LIST_CORE
	#endif

	#ifdef WB_GL_USE_MODERN
		WB_GL__PROC_LIST_MODERN
	#endif

	return ctx->error_count;
}
#endif

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_NONE 0
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_POINT_SIZE 0x0B11
#define GL_POINT_SIZE_RANGE 0x0B12
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_WIDTH 0x0B21
#define GL_LINE_WIDTH_RANGE 0x0B22
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_POLYGON_MODE 0x0B40
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC 0x0B74
#define GL_STENCIL_TEST 0x0B90
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_VIEWPORT 0x0BA2
#define GL_DITHER 0x0BD0
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_LOGIC_OP_MODE 0x0BF0
#define GL_COLOR_LOGIC_OP 0x0BF2
#define GL_DRAW_BUFFER 0x0C01
#define GL_READ_BUFFER 0x0C02
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_DOUBLEBUFFER 0x0C32
#define GL_STEREO 0x0C33
#define GL_LINE_SMOOTH_HINT 0x0C52
#define GL_POLYGON_SMOOTH_HINT 0x0C53
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_SUBPIXEL_BITS 0x0D50
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_POLYGON_OFFSET_UNITS 0x2A00
#define GL_POLYGON_OFFSET_POINT 0x2A01
#define GL_POLYGON_OFFSET_LINE 0x2A02
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_TEXTURE_BINDING_1D 0x8068
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_RED_SIZE 0x805C
#define GL_TEXTURE_GREEN_SIZE 0x805D
#define GL_TEXTURE_BLUE_SIZE 0x805E
#define GL_TEXTURE_ALPHA_SIZE 0x805F
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_CLEAR 0x1500
#define GL_AND 0x1501
#define GL_AND_REVERSE 0x1502
#define GL_COPY 0x1503
#define GL_AND_INVERTED 0x1504
#define GL_NOOP 0x1505
#define GL_XOR 0x1506
#define GL_OR 0x1507
#define GL_NOR 0x1508
#define GL_EQUIV 0x1509
#define GL_INVERT 0x150A
#define GL_OR_REVERSE 0x150B
#define GL_COPY_INVERTED 0x150C
#define GL_OR_INVERTED 0x150D
#define GL_NAND 0x150E
#define GL_SET 0x150F
#define GL_TEXTURE 0x1702
#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_STENCIL_INDEX 0x1901
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_PROXY_TEXTURE_1D 0x8063
#define GL_PROXY_TEXTURE_2D 0x8064
#define GL_REPEAT 0x2901
#define GL_R3_G3_B2 0x2A10
#define GL_RGB4 0x804F
#define GL_RGB5 0x8050
#define GL_RGB8 0x8051
#define GL_RGB10 0x8052
#define GL_RGB12 0x8053
#define GL_RGB16 0x8054
#define GL_RGBA2 0x8055
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_RGBA12 0x805A
#define GL_RGBA16 0x805B
#define GL_CURRENT_BIT 0x00000001
#define GL_POINT_BIT 0x00000002
#define GL_LINE_BIT 0x00000004
#define GL_POLYGON_BIT 0x00000008
#define GL_POLYGON_STIPPLE_BIT 0x00000010
#define GL_PIXEL_MODE_BIT 0x00000020
#define GL_LIGHTING_BIT 0x00000040
#define GL_FOG_BIT 0x00000080
#define GL_ACCUM_BUFFER_BIT 0x00000200
#define GL_VIEWPORT_BIT 0x00000800
#define GL_TRANSFORM_BIT 0x00001000
#define GL_ENABLE_BIT 0x00002000
#define GL_HINT_BIT 0x00008000
#define GL_EVAL_BIT 0x00010000
#define GL_LIST_BIT 0x00020000
#define GL_TEXTURE_BIT 0x00040000
#define GL_SCISSOR_BIT 0x00080000
#define GL_ALL_ATTRIB_BITS 0xFFFFFFFF
#define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS 0xFFFFFFFF
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON 0x0009
#define GL_ACCUM 0x0100
#define GL_LOAD 0x0101
#define GL_RETURN 0x0102
#define GL_MULT 0x0103
#define GL_ADD 0x0104
#define GL_AUX0 0x0409
#define GL_AUX1 0x040A
#define GL_AUX2 0x040B
#define GL_AUX3 0x040C
#define GL_2D 0x0600
#define GL_3D 0x0601
#define GL_3D_COLOR 0x0602
#define GL_3D_COLOR_TEXTURE 0x0603
#define GL_4D_COLOR_TEXTURE 0x0604
#define GL_PASS_THROUGH_TOKEN 0x0700
#define GL_POINT_TOKEN 0x0701
#define GL_LINE_TOKEN 0x0702
#define GL_POLYGON_TOKEN 0x0703
#define GL_BITMAP_TOKEN 0x0704
#define GL_DRAW_PIXEL_TOKEN 0x0705
#define GL_COPY_PIXEL_TOKEN 0x0706
#define GL_LINE_RESET_TOKEN 0x0707
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#define GL_COEFF 0x0A00
#define GL_ORDER 0x0A01
#define GL_DOMAIN 0x0A02
#define GL_PIXEL_MAP_I_TO_I 0x0C70
#define GL_PIXEL_MAP_S_TO_S 0x0C71
#define GL_PIXEL_MAP_I_TO_R 0x0C72
#define GL_PIXEL_MAP_I_TO_G 0x0C73
#define GL_PIXEL_MAP_I_TO_B 0x0C74
#define GL_PIXEL_MAP_I_TO_A 0x0C75
#define GL_PIXEL_MAP_R_TO_R 0x0C76
#define GL_PIXEL_MAP_G_TO_G 0x0C77
#define GL_PIXEL_MAP_B_TO_B 0x0C78
#define GL_PIXEL_MAP_A_TO_A 0x0C79
#define GL_VERTEX_ARRAY_POINTER 0x808E
#define GL_NORMAL_ARRAY_POINTER 0x808F
#define GL_COLOR_ARRAY_POINTER 0x8090
#define GL_INDEX_ARRAY_POINTER 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#define GL_SELECTION_BUFFER_POINTER 0x0DF3
#define GL_CURRENT_COLOR 0x0B00
#define GL_CURRENT_INDEX 0x0B01
#define GL_CURRENT_NORMAL 0x0B02
#define GL_CURRENT_TEXTURE_COORDS 0x0B03
#define GL_CURRENT_RASTER_COLOR 0x0B04
#define GL_CURRENT_RASTER_INDEX 0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#define GL_CURRENT_RASTER_POSITION 0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#define GL_CURRENT_RASTER_DISTANCE 0x0B09
#define GL_POINT_SMOOTH 0x0B10
#define GL_LINE_STIPPLE 0x0B24
#define GL_LINE_STIPPLE_PATTERN 0x0B25
#define GL_LINE_STIPPLE_REPEAT 0x0B26
#define GL_LIST_MODE 0x0B30
#define GL_MAX_LIST_NESTING 0x0B31
#define GL_LIST_BASE 0x0B32
#define GL_LIST_INDEX 0x0B33
#define GL_POLYGON_STIPPLE 0x0B42
#define GL_EDGE_FLAG 0x0B43
#define GL_LIGHTING 0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_SHADE_MODEL 0x0B54
#define GL_COLOR_MATERIAL_FACE 0x0B55
#define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#define GL_COLOR_MATERIAL 0x0B57
#define GL_FOG 0x0B60
#define GL_FOG_INDEX 0x0B61
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_MODE 0x0B65
#define GL_FOG_COLOR 0x0B66
#define GL_ACCUM_CLEAR_VALUE 0x0B80
#define GL_MATRIX_MODE 0x0BA0
#define GL_NORMALIZE 0x0BA1
#define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_TEXTURE_STACK_DEPTH 0x0BA5
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_TEXTURE_MATRIX 0x0BA8
#define GL_ATTRIB_STACK_DEPTH 0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#define GL_ALPHA_TEST 0x0BC0
#define GL_ALPHA_TEST_FUNC 0x0BC1
#define GL_ALPHA_TEST_REF 0x0BC2 #define GL_INDEX_LOGIC_OP 0x0BF1
#define GL_LOGIC_OP 0x0BF1
#define GL_AUX_BUFFERS 0x0C00
#define GL_INDEX_CLEAR_VALUE 0x0C20
#define GL_INDEX_WRITEMASK 0x0C21
#define GL_INDEX_MODE 0x0C30
#define GL_RGBA_MODE 0x0C31
#define GL_RENDER_MODE 0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_POINT_SMOOTH_HINT 0x0C51
#define GL_FOG_HINT 0x0C54
#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GEN_R 0x0C62
#define GL_TEXTURE_GEN_Q 0x0C63
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#define GL_MAP_COLOR 0x0D10
#define GL_MAP_STENCIL 0x0D11
#define GL_INDEX_SHIFT 0x0D12
#define GL_INDEX_OFFSET 0x0D13
#define GL_RED_SCALE 0x0D14
#define GL_RED_BIAS 0x0D15
#define GL_ZOOM_X 0x0D16
#define GL_ZOOM_Y 0x0D17
#define GL_GREEN_SCALE 0x0D18
#define GL_GREEN_BIAS 0x0D19
#define GL_BLUE_SCALE 0x0D1A
#define GL_BLUE_BIAS 0x0D1B
#define GL_ALPHA_SCALE 0x0D1C
#define GL_ALPHA_BIAS 0x0D1D
#define GL_DEPTH_SCALE 0x0D1E
#define GL_DEPTH_BIAS 0x0D1F
#define GL_MAX_EVAL_ORDER 0x0D30
#define GL_MAX_LIGHTS 0x0D31
#define GL_MAX_CLIP_PLANES 0x0D32
#define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#define GL_MAX_NAME_STACK_DEPTH 0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#define GL_INDEX_BITS 0x0D51
#define GL_RED_BITS 0x0D52
#define GL_GREEN_BITS 0x0D53
#define GL_BLUE_BITS 0x0D54
#define GL_ALPHA_BITS 0x0D55
#define GL_DEPTH_BITS 0x0D56
#define GL_STENCIL_BITS 0x0D57
#define GL_ACCUM_RED_BITS 0x0D58
#define GL_ACCUM_GREEN_BITS 0x0D59
#define GL_ACCUM_BLUE_BITS 0x0D5A
#define GL_ACCUM_ALPHA_BITS 0x0D5B
#define GL_NAME_STACK_DEPTH 0x0D70
#define GL_AUTO_NORMAL 0x0D80
#define GL_MAP1_COLOR_4 0x0D90
#define GL_MAP1_INDEX 0x0D91
#define GL_MAP1_NORMAL 0x0D92
#define GL_MAP1_TEXTURE_COORD_1 0x0D93
#define GL_MAP1_TEXTURE_COORD_2 0x0D94
#define GL_MAP1_TEXTURE_COORD_3 0x0D95
#define GL_MAP1_TEXTURE_COORD_4 0x0D96
#define GL_MAP1_VERTEX_3 0x0D97
#define GL_MAP1_VERTEX_4 0x0D98
#define GL_MAP2_COLOR_4 0x0DB0
#define GL_MAP2_INDEX 0x0DB1
#define GL_MAP2_NORMAL 0x0DB2
#define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#define GL_MAP2_VERTEX_3 0x0DB7
#define GL_MAP2_VERTEX_4 0x0DB8
#define GL_MAP1_GRID_DOMAIN 0x0DD0
#define GL_MAP1_GRID_SEGMENTS 0x0DD1
#define GL_MAP2_GRID_DOMAIN 0x0DD2
#define GL_MAP2_GRID_SEGMENTS 0x0DD3
#define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#define GL_SELECTION_BUFFER_SIZE 0x0DF4
#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_INDEX_ARRAY 0x8077
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_EDGE_FLAG_ARRAY 0x8079
#define GL_VERTEX_ARRAY_SIZE 0x807A
#define GL_VERTEX_ARRAY_TYPE 0x807B
#define GL_VERTEX_ARRAY_STRIDE 0x807C
#define GL_NORMAL_ARRAY_TYPE 0x807E
#define GL_NORMAL_ARRAY_STRIDE 0x807F
#define GL_COLOR_ARRAY_SIZE 0x8081
#define GL_COLOR_ARRAY_TYPE 0x8082
#define GL_COLOR_ARRAY_STRIDE 0x8083
#define GL_INDEX_ARRAY_TYPE 0x8085
#define GL_INDEX_ARRAY_STRIDE 0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#define GL_TEXTURE_COMPONENTS 0x1003
#define GL_TEXTURE_BORDER 0x1005
#define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#define GL_TEXTURE_INTENSITY_SIZE 0x8061
#define GL_TEXTURE_PRIORITY 0x8066
#define GL_TEXTURE_RESIDENT 0x8067
#define GL_AMBIENT 0x1200
#define GL_DIFFUSE 0x1201
#define GL_SPECULAR 0x1202
#define GL_POSITION 0x1203
#define GL_SPOT_DIRECTION 0x1204
#define GL_SPOT_EXPONENT 0x1205
#define GL_SPOT_CUTOFF 0x1206
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_LINEAR_ATTENUATION 0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209
#define GL_COMPILE 0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#define GL_2_BYTES 0x1407
#define GL_3_BYTES 0x1408
#define GL_4_BYTES 0x1409
#define GL_EMISSION 0x1600
#define GL_SHININESS 0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_COLOR_INDEXES 0x1603
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_COLOR_INDEX 0x1900
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_BITMAP 0x1A00
#define GL_RENDER 0x1C00
#define GL_FEEDBACK 0x1C01
#define GL_SELECT 0x1C02
#define GL_FLAT 0x1D00
#define GL_SMOOTH 0x1D01
#define GL_S 0x2000
#define GL_T 0x2001
#define GL_R 0x2002
#define GL_Q 0x2003
#define GL_MODULATE 0x2100
#define GL_DECAL 0x2101
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_TEXTURE_ENV 0x2300
#define GL_EYE_LINEAR 0x2400
#define GL_OBJECT_LINEAR 0x2401
#define GL_SPHERE_MAP 0x2402
#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_OBJECT_PLANE 0x2501
#define GL_EYE_PLANE 0x2502
#define GL_CLAMP 0x2900
#define GL_ALPHA4 0x803B
#define GL_ALPHA8 0x803C
#define GL_ALPHA12 0x803D
#define GL_ALPHA16 0x803E
#define GL_LUMINANCE4 0x803F
#define GL_LUMINANCE8 0x8040
#define GL_LUMINANCE12 0x8041
#define GL_LUMINANCE16 0x8042
#define GL_LUMINANCE4_ALPHA4 0x8043
#define GL_LUMINANCE6_ALPHA2 0x8044
#define GL_LUMINANCE8_ALPHA8 0x8045
#define GL_LUMINANCE12_ALPHA4 0x8046
#define GL_LUMINANCE12_ALPHA12 0x8047
#define GL_LUMINANCE16_ALPHA16 0x8048
#define GL_INTENSITY 0x8049
#define GL_INTENSITY4 0x804A
#define GL_INTENSITY8 0x804B
#define GL_INTENSITY12 0x804C
#define GL_INTENSITY16 0x804D
#define GL_V2F 0x2A20
#define GL_V3F 0x2A21
#define GL_C4UB_V2F 0x2A22
#define GL_C4UB_V3F 0x2A23
#define GL_C3F_V3F 0x2A24
#define GL_N3F_V3F 0x2A25
#define GL_C4F_N3F_V3F 0x2A26
#define GL_T2F_V3F 0x2A27
#define GL_T4F_V4F 0x2A28
#define GL_T2F_C4UB_V3F 0x2A29
#define GL_T2F_C3F_V3F 0x2A2A
#define GL_T2F_N3F_V3F 0x2A2B
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#define GL_T4F_C4F_N3F_V4F 0x2A2D
#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#define GL_RESCALE_NORMAL 0x803A
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_SINGLE_COLOR 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_MULTISAMPLE 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_MULTISAMPLE_BIT 0x20000000
#define GL_NORMAL_MAP 0x8511
#define GL_REFLECTION_MAP 0x8512
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE2_ALPHA 0x858A
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND2_ALPHA 0x859A
#define GL_RGB_SCALE 0x8573
#define GL_ADD_SIGNED 0x8574
#define GL_INTERPOLATE 0x8575
#define GL_SUBTRACT 0x84E7
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_COMPARE_R_TO_TEXTURE 0x884E
#define GL_FUNC_ADD 0x8006
#define GL_FUNC_SUBTRACT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_MIN 0x8007
#define GL_MAX 0x8008
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914
#define GL_SRC1_ALPHA 0x8589
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_FOG_COORD_SRC 0x8450
#define GL_FOG_COORD 0x8451
#define GL_CURRENT_FOG_COORD 0x8453
#define GL_FOG_COORD_ARRAY_TYPE 0x8454
#define GL_FOG_COORD_ARRAY_STRIDE 0x8455
#define GL_FOG_COORD_ARRAY_POINTER 0x8456
#define GL_FOG_COORD_ARRAY 0x8457
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#define GL_SRC0_RGB 0x8580
#define GL_SRC1_RGB 0x8581
#define GL_SRC2_RGB 0x8582
#define GL_SRC0_ALPHA 0x8588
#define GL_SRC2_ALPHA 0x858A
#define GL_BLEND_EQUATION_RGB 0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB_ALPHA 0x8C42
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_COMPRESSED_SRGB 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_SLUMINANCE_ALPHA 0x8C44
#define GL_SLUMINANCE8_ALPHA8 0x8C45
#define GL_SLUMINANCE 0x8C46
#define GL_SLUMINANCE8 0x8C47
#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_CLIP_DISTANCE0 0x3000
#define GL_CLIP_DISTANCE1 0x3001
#define GL_CLIP_DISTANCE2 0x3002
#define GL_CLIP_DISTANCE3 0x3003
#define GL_CLIP_DISTANCE4 0x3004
#define GL_CLIP_DISTANCE5 0x3005
#define GL_CLIP_DISTANCE6 0x3006
#define GL_CLIP_DISTANCE7 0x3007
#define GL_MAX_CLIP_DISTANCES 0x0D32
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_CONTEXT_FLAGS 0x821E
#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG 0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#define GL_CLAMP_READ_COLOR 0x891C
#define GL_FIXED_ONLY 0x891D
#define GL_MAX_VARYING_COMPONENTS 0x8B4B
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#define GL_RGB9_E5 0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define GL_TEXTURE_SHARED_SIZE 0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED 0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_SEPARATE_ATTRIBS 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B
#define GL_SAMPLER_1D_ARRAY 0x8DC0
#define GL_SAMPLER_2D_ARRAY 0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#define GL_INT_SAMPLER_1D 0x8DC9
#define GL_INT_SAMPLER_2D 0x8DCA
#define GL_INT_SAMPLER_3D 0x8DCB
#define GL_INT_SAMPLER_CUBE 0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#define GL_QUERY_WAIT 0x8E13
#define GL_QUERY_NO_WAIT 0x8E14
#define GL_QUERY_BY_REGION_WAIT 0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT 0x8E16
#define GL_BUFFER_ACCESS_FLAGS 0x911F
#define GL_BUFFER_MAP_LENGTH 0x9120
#define GL_BUFFER_MAP_OFFSET 0x9121
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT 0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#define GL_DEPTH_STENCIL 0x84F9
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_TEXTURE_STENCIL_SIZE 0x88F1
#define GL_TEXTURE_RED_TYPE 0x8C10
#define GL_TEXTURE_GREEN_TYPE 0x8C11
#define GL_TEXTURE_BLUE_TYPE 0x8C12
#define GL_TEXTURE_ALPHA_TYPE 0x8C13
#define GL_TEXTURE_DEPTH_TYPE 0x8C16
#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_COLOR_ATTACHMENT16 0x8CF0
#define GL_COLOR_ATTACHMENT17 0x8CF1
#define GL_COLOR_ATTACHMENT18 0x8CF2
#define GL_COLOR_ATTACHMENT19 0x8CF3
#define GL_COLOR_ATTACHMENT20 0x8CF4
#define GL_COLOR_ATTACHMENT21 0x8CF5
#define GL_COLOR_ATTACHMENT22 0x8CF6
#define GL_COLOR_ATTACHMENT23 0x8CF7
#define GL_COLOR_ATTACHMENT24 0x8CF8
#define GL_COLOR_ATTACHMENT25 0x8CF9
#define GL_COLOR_ATTACHMENT26 0x8CFA
#define GL_COLOR_ATTACHMENT27 0x8CFB
#define GL_COLOR_ATTACHMENT28 0x8CFC
#define GL_COLOR_ATTACHMENT29 0x8CFD
#define GL_COLOR_ATTACHMENT30 0x8CFE
#define GL_COLOR_ATTACHMENT31 0x8CFF
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#define GL_STENCIL_INDEX1 0x8D46
#define GL_STENCIL_INDEX4 0x8D47
#define GL_STENCIL_INDEX8 0x8D48
#define GL_STENCIL_INDEX16 0x8D49
#define GL_RENDERBUFFER_RED_SIZE 0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES 0x8D57
#define GL_INDEX 0x8222
#define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_HALF_FLOAT 0x140B
#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define GL_COMPRESSED_RG_RGTC2 0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_R16 0x822A
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#define GL_CLAMP_VERTEX_COLOR 0x891A
#define GL_CLAMP_FRAGMENT_COLOR 0x891B
#define GL_ALPHA_INTEGER 0x8D97
#define GL_SAMPLER_2D_RECT 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#define GL_SAMPLER_BUFFER 0x8DC2
#define GL_INT_SAMPLER_2D_RECT 0x8DCD
#define GL_INT_SAMPLER_BUFFER 0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#define GL_TEXTURE_BINDING_BUFFER 0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE 0x84F8
#define GL_R8_SNORM 0x8F94
#define GL_RG8_SNORM 0x8F95
#define GL_RGB8_SNORM 0x8F96
#define GL_RGBA8_SNORM 0x8F97
#define GL_R16_SNORM 0x8F98
#define GL_RG16_SNORM 0x8F99
#define GL_RGB16_SNORM 0x8F9A
#define GL_RGBA16_SNORM 0x8F9B
#define GL_SIGNED_NORMALIZED 0x8F9C
#define GL_PRIMITIVE_RESTART 0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX 0x8F9E
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_UNIFORM_BUFFER_BINDING 0x8A28
#define GL_UNIFORM_BUFFER_START 0x8A29
#define GL_UNIFORM_BUFFER_SIZE 0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS 0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS 0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS 0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS 0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS 0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE 0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#define GL_UNIFORM_TYPE 0x8A37
#define GL_UNIFORM_SIZE 0x8A38
#define GL_UNIFORM_NAME_LENGTH 0x8A39
#define GL_UNIFORM_BLOCK_INDEX 0x8A3A
#define GL_UNIFORM_OFFSET 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE 0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE 0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR 0x8A3E
#define GL_UNIFORM_BLOCK_BINDING 0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE 0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH 0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX 0xFFFFFFFF
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_LINES_ADJACENCY 0x000A
#define GL_LINE_STRIP_ADJACENCY 0x000B
#define GL_TRIANGLES_ADJACENCY 0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#define GL_PROGRAM_POINT_SIZE 0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_GEOMETRY_VERTICES_OUT 0x8916
#define GL_GEOMETRY_INPUT_TYPE 0x8917
#define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_DEPTH_CLAMP 0x864F
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION 0x8E4D
#define GL_LAST_VERTEX_CONVENTION 0x8E4E
#define GL_PROVOKING_VERTEX 0x8E4F
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#define GL_MAX_SERVER_WAIT_TIMEOUT 0x9111
#define GL_OBJECT_TYPE 0x9112
#define GL_SYNC_CONDITION 0x9113
#define GL_SYNC_STATUS 0x9114
#define GL_SYNC_FLAGS 0x9115
#define GL_SYNC_FENCE 0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_UNSIGNALED 0x9118
#define GL_SIGNALED 0x9119
#define GL_ALREADY_SIGNALED 0x911A
#define GL_TIMEOUT_EXPIRED 0x911B
#define GL_CONDITION_SATISFIED 0x911C
#define GL_WAIT_FAILED 0x911D
#define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFF
#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#define GL_SAMPLE_POSITION 0x8E50
#define GL_SAMPLE_MASK 0x8E51
#define GL_SAMPLE_MASK_VALUE 0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS 0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE 0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#define GL_TEXTURE_SAMPLES 0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#define GL_MAX_INTEGER_SAMPLES 0x9110
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR 0x88FE
#define GL_SRC1_COLOR 0x88F9
#define GL_ONE_MINUS_SRC1_COLOR 0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA 0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS 0x88FC
#define GL_ANY_SAMPLES_PASSED 0x8C2F
#define GL_SAMPLER_BINDING 0x8919
#define GL_RGB10_A2UI 0x906F
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#define GL_TIME_ELAPSED 0x88BF
#define GL_TIMESTAMP 0x8E28
#define GL_INT_2_10_10_10_REV 0x8D9F
