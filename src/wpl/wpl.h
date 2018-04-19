#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <intrin.h>
#define VariadicArgs ...

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;

typedef __m128 vf128;
typedef __m128i vi128;

union vf32x4
{
	vf128 v;
	f32 f[4];
};
typedef union vf32x4 vf32x4;

#define ShuffleToByte(x, y, z, w) (((x)<<6) | ((y)<<4) | ((z)<<2) | w)
#define vfShuffle(a, x, y, z, w) _mm_shuffle_ps(a, a,\
		ShuffleToByte(x, y, z, w))
#define vfShuffle2(a, b, ax, ay, bz, bw) _mm_shuffle_ps(a, b, \
		ShuffleToByte(x, y, z, w))

#define Math_Tau 6.283185307179586f
#define Math_DegToRad (Math_Tau / 360.0f)
#define Math_RadToDeg (360.0f / Math_Tau)

#define Shader_MaxAttribs 16
#define Shader_MaxUniforms 16

#define Arena_Normal 0
#define Arena_FixedSIze 1
#define Arena_Stack 2
#define Arena_Extended 4
#define Arena_NoZeroMemory 8
#define Arena_NoRecommit 16 

#define Pool_Normal 0
#define Pool_FixedSize 1
#define Pool_Compacting 2
#define Pool_NoZeroMemory 4
#define Pool_NoDoubleFreeCheck 8

#define Tagged_Normal 0
#define Tagged_FixedSize 1
#define Tagged_NoZeroMemory 2
#define Tagged_NoSetCommitSize 4
#define Tagged_SearchForBestFit 8

typedef struct wMemoryInfo wMemoryInfo;
typedef struct wMemoryArena wMemoryArena;
typedef struct wMemoryPool wMemoryPool;
typedef struct wTaggedHeapArena wTaggedHeapArena;
typedef struct wTaggedHeap wTaggedHeap;

typedef struct wWindowDef wWindowDef;
typedef struct wWindow wWindow;
typedef struct wInputState wInputState;
typedef struct wState wState;

typedef struct wSprite wSprite;
typedef struct wVertex wVertex;
typedef struct wRenderGroup wRenderGroup;
typedef struct wRenderBatch wRenderBatch;
typedef struct wShaderComponent wShaderComponent;
typedef struct wShader wShader;
typedef struct wTexture wTexture;

typedef struct wMixer wMixer;
typedef struct wMixerVoice wMixerVoice;
typedef struct wMixerStream wMixerStream;
typedef struct wMixerSample wMixerSample;
typedef void (*wMixerStreamProc)(wMixerSample* sample, void* userdata);

typedef struct wSlice wSlice;
typedef struct wSpriteList wSpriteList;
typedef struct wGlyph wGlyph;
typedef struct wGlyphImage wGlyphImage;
typedef struct wFontInfo wFontInfo;

typedef const char* string;

/* optional CRT replacement header */
#ifdef WPL_REPLACE_CRT
void* memset(void* s, i32 ivalue, usize size);
i32 memcmp(const void* s1, const void* s2, usize size);
void* memcpy(void *dest, const void *source, usize size);
usize strlen(const char* c);
void* malloc(usize size);
void free(void* mem);
void* realloc(void* mem, usize size);
i32 abs(i32 x);
i32 printf(string fmt, VariadicArgs);
i32 fprintf(void* file, string fmt, VariadicArgs);
i32 vfprintf(void* file, string fmt, va_list args);
extern void* stderr;
extern void* stdout;
#endif

/* utility */

void wLogError(i32 errorClass, string fmt, VariadicArgs);

/* s-archive types */

#define wSar_Magic (0x77536172)
#define wSar_Version (101)
#pragma pack(push, 4)
#define wSar_NameLen (55)
typedef struct wSarId wSarId;
typedef struct wSarHeader wSarHeader;
typedef struct wSarFile wSarFile;
typedef struct wSarArchive wSarArchive;
typedef struct wSarEditingArchive wSarEditingArchive;

struct wSarId
{
	u64 hash;
	char name[wSar_NameLen], zero;
};

struct wSarFile
{
	wSarId id;
	u32 kind;
	u32 version;
	u64 compressedSize;
	u64 fullSize;
	u64 location;
};

struct wSarHeader
{
	u32 magic;
	u32 version;
	u64 unused[3];

	wSarId id;
	u64 archiveSize;
	u64 fileCount;
	u64 fileTableLocation;
	u64 descriptionLength;
};

struct wSarArchive
{
	char* base;
	wSarHeader* header;
	char* description;
	wSarFile* files;
};

#pragma pack(pop)


/* wb_alloc types */

struct wMemoryInfo
{
	usize totalMemory, commitSize, pageSize;
	isize commitFlags;
};

struct wMemoryArena
{
	const char* name;
	void *start, *head, *end;
	void *tempStart, *tempHead;
	wMemoryInfo info;
	isize align;
	isize flags;
};

struct wMemoryPool
{
	usize elementSize; 
	isize count, capacity;
	void* slots;
	const char* name;
	void** freeList;
	wMemoryArena* alloc;
	isize lastFilled;
	isize flags;
};

struct wTaggedHeapArena
{
	isize tag;
	wTaggedHeapArena *next;
	void *head, *end;
	char buffer;
};

struct wTaggedHeap
{
	const char* name;
	wMemoryPool pool;
	wTaggedHeapArena* arenas[64];
	wMemoryInfo info;
	usize arenaSize, align;
	isize flags;
};

/* inherited sts_mixer types */
struct wMixerSample
{
	//in samples (4 bytes)
	u32 length;
	u32 frequency;        
	void* data;
};

struct wMixerStream 
{
	void* userdata;         
	wMixerStreamProc callback;         
	wMixerSample sample;           
};

struct wMixerVoice 
{
	wMixerSample* sample;
	wMixerStream* stream;
	f32 position;
	f32 gain;
	f32 pitch;
	f32 pan;
	i32 state;
};

struct wMixer
{
	f32 gain; 
	u32 frequency;
	
	isize voiceCount;
	wMixerVoice* voices;
};

/* core w types */

struct wWindowDef
{
	string title;
	i64 width, height;

	// Position information
	// If centered == 1, 
	// center the window on monitorIndex
	// else, use x, y
	i64 posCentered;
	i64 posUndefined;
	//i64 monitorIndex;
	i64 x, y;

	i64 resizeable;
	i64 borderless;
	i64 hidden;

	i64 glVersion;
};

struct wWindow
{
	i64 refreshRate;
	i64 glVersion;
	i64 lastTicks;
	i64 elapsedTicks;
	i8* basePath;
	const u8 *vertShader, *fragShader;
	void* windowHandle;
};

#define wMouseLeft 0
#define wMouseRight 1
#define wMouseMiddle 2
#define wMouseX1 3
#define wMouseX2 4
struct wInputState
{
	//i8* keyboard;
	//i8 scancodes[512];
	//i8 reps[256];
	i8 keys[256];
	i8 mouse[16];
	f32 mouseWheel;
};

struct wAudioState
{
	wMixer mixer;
};

struct wState
{
	wInputState* input;
	i64 width, height;
	i64 hasFocus;
	i64 mouseX, mouseY;
	i64 exitEvent;
};

/* Graphics */

struct wShaderComponent
{
	string name;
	i32 loc, divisor;

	i32 type, count;
	usize ptr;
};

enum {
	//Shader kinds
	wShader_Vertex,
	wShader_Frag,
	//Component kinds
	wShader_Attrib,
	wShader_Uniform,
	//Component types
	wShader_Float,
	wShader_Double,
	//Passed with glVertexAttributeIPointer
	wShader_Int,
	wShader_Short,
	wShader_Byte,
	//Passed with normalize set to true
	wShader_NormalizedInt,
	wShader_NormalizedShort,
	wShader_NormalizedByte,
	//Converted to floats on the GPU
	wShader_FloatInt,
	wShader_FloatShort,
	wShader_FloatByte,
	wShader_Mat22,	
	wShader_Mat33,
	wShader_Mat44
};

enum {
	wRenderBatch_Arrays,
	wRenderBatch_Elements,
	wRenderBatch_ArraysInstanced,
	wRenderBatch_ElementsInstanced,
};

enum {
	wRenderBatch_BlendNormal,
	wRenderBatch_BlendPremultiplied,
	wRenderBatch_BlendNone,
};

enum {
	wRenderBatch_Triangles,
	wRenderBatch_TriangleStrip,
	wRenderBatch_TriangleFan,
	wRenderBatch_Lines,
	wRenderBatch_LineStrip,
	wRenderBatch_LineLoop
};

struct wShader
{
	u32 vert, frag, program;
	i32 targetVersion;

	i32 defaultDivisor;
	i32 stride;
	
	i32 attribCount, uniformCount;
	wShaderComponent attribs[Shader_MaxAttribs];
	wShaderComponent uniforms[Shader_MaxUniforms];
};

struct wTexture
{
	i64 w, h;
	u8* pixels;
	u32 glIndex;
};

struct wRenderBatch
{
	wTexture* texture;
	wShader* shader;

	u32 vao, vbo;

	isize elementSize, elementCount, instanceSize;
	isize indicesCount;
	isize startOffset;
	void* data;
	u32* indices;

	i32 clearOnDraw;
	i32 renderCall;
	i32 blend;

	i32 primitiveMode;
};

enum ButtonState
{
	Button_JustUp = -1,
	Button_Up = 0,
	Button_Down = 1,
	Button_JustDown = 2
};

#pragma pack(push, 4)
struct wGlyph
{
	int character;
	f32 width, height;
	f32 x, y;
	f32 advance;
	f32 l, b, r, t;
};

struct wGlyphImage
{
	i32 x, y, w, h;
	f32 bbx, bby;
};

struct wFontInfo
{
	i32 sizeX, sizeY;
	i32 scale;
	i32 offsetX, offsetY;
	i32 pxRange;
	i32 lineSpacing;
	i32 atlasX, atlasY;

	wGlyph glyphs[96];
	wGlyphImage images[96];
	f32 kerning[96][96];
};
#pragma pack(pop)

usize wDecompressMemToMem(
		void *output,
		usize outSize,
		const void *input,
		usize inSize,
		i32 flags);

wWindowDef wDefineWindow(string title);
i64 wCreateWindow(wWindowDef* def, wWindow* window);

void wInitState(wState* state, wInputState* input);

void wShowWindow();
i64 wUpdate(wWindow* window, wState* state);
i64 wRender(wWindow* window);

/* input system */

/* TODO(will) drastically simplify this :) */

i64 wKeyIsDown(wInputState* wInput, i64 keycode);
i64 wKeyIsUp(wInputState* wInput, i64 keycode);
i64 wKeyIsJustDown(wInputState* wInput, i64 keycode);
i64 wKeyIsJustUp(wInputState* wInput, i64 keycode);
i64 wMouseIsDown(wInputState* wInput, i64 btn);
i64 wMouseIsUp(wInputState* wInput, i64 btn);
i64 wMouseIsJustDown(wInputState* wInput, i64 btn);
i64 wMouseIsJustUp(wInputState* wInput, i64 btn);
f32 wGetMouseWheel(wInputState* wInput);

/* Graphics */

/* TODO(will): add batch/new shader related functions here */
/* TODO(will): make shader hotloading simple + easy */

void wInitShader(wShader* shader, i32 stride);
i32 wAddAttribToShader(wShader* shader, wShaderComponent* attrib);
wShaderComponent* wCreateAttrib(wShader* shader, 
		string name, i32 type, i32 count, usize ptr);
i32 wAddUniformToShader(wShader* shader, wShaderComponent* uniform);
wShaderComponent* wCreateUniform(wShader* shader,
		string name, i32 type, i32 count, usize ptr);
i32 wFinalizeShader(wShader* shader);
i32 wAddSourceToShader(wShader* shader, string src, i32 kind);




void wInitBatch(wRenderBatch* batch,
		wTexture* texture, wShader* shader,
		i32 renderCall, i32 primitiveMode, 
		isize elementSize, isize instanceSize,
		void* data, u32* indices);
void wConstructBatchGraphicsState(wRenderBatch* batch);
void wDrawBatch(wState* state, wRenderBatch* batch, void* uniformData);


//wTexture* wLoadTexture(wWindow* window, string filename, wMemoryArena* arena);
i32 wInitTexture(wTexture* texture, void* data, isize size);
void wUploadTexture(wTexture* texture);

/* Utility */
void* wCopyMemory(void *dest, const void *source, i64 size);
void wCopyMemoryBlock(void* dest, const void* source, 
		i32 sx, i32 sy, i32 sw, i32 sh,
		i32 dx, i32 dy, i32 dw, i32 dh,
		i32 size, i32 border);

/* File Handling */

// TODO(will) add wSar functions here
u8* wLoadFile(string filename, isize* sizeOut, wMemoryArena* alloc);
isize wLoadSizedFile(string filename, u8* buffer, isize bufferSize);
u8* wLoadLocalFile(wWindow* window, string filename, isize* sizeOut, wMemoryArena* arena);
isize wLoadLocalSizedFile(
		wWindow* window, string filename,
		u8* buffer, isize bufferSize);

// TODO(will) simple screenshot function
void wWriteImage(string filename, i64 w, i64 h, void* data);

/* Fonts */

wFontInfo* wLoadFontInfo(wWindow* window, char* filename, wMemoryArena* arena);
// TODO(will) convenience functions that make it easy to implement nice font
// 				layout/rendering on the client side.
// Stuff like:
//  - wFontGetKerningPair
//  - wFontLayoutGlyph
//  - wFontGetGlyphRect
// Essentially things that encapsulate the weirdness that goes on in the 
// render text function of the old batch (though we'll provide that too)

/* wb_alloc interface */

wMemoryInfo wGetMemoryInfo();

void wArenaInit(wMemoryArena* arena, wMemoryInfo info, isize flags);
wMemoryArena* wArenaBootstrap(wMemoryInfo info, isize flags);
void wArenaFixedSizeInit(wMemoryArena* arena, void* buffer, isize size, isize flags);
wMemoryArena* wArenaFixedSizeBootstrap(void* buffer, usize size, isize flags);
void* wArenaPushEx(wMemoryArena* arena, isize size, isize extended);
void* wArenaPush(wMemoryArena* arena, isize size);
void wArenaPop(wMemoryArena* arena);
void wArenaStartTemp(wMemoryArena* arena);
void wArenaEndTemp(wMemoryArena* arena);
void wArenaClear(wMemoryArena* arena);
void wArenaDestroy(wMemoryArena* arena);
 
void* wPoolRetrieve(wMemoryPool* pool);
void wPoolRelease(wMemoryPool* pool, void* ptr);
void wPoolInit(wMemoryPool* pool,wMemoryArena* alloc, usize elementSize, isize flags);
wMemoryPool* wPoolBootstrap(wMemoryInfo info,isize elementSize, isize flags);
wMemoryPool* wPoolFixedSizeBootstrap(
		isize elementSize, 
		void* buffer, usize size,
		isize flags);
 
void wTaggedInit(wTaggedHeap* heap, wMemoryArena* arena, isize lsize, isize flags);
wTaggedHeap* wTaggedBootstrap(wMemoryInfo info, isize arenaSize, isize flags);
wTaggedHeap* wTaggedFixedSizeBootstrap(isize arenaSize, 
		void* buffer, isize bufferSize, 
		isize flags);
void* wTaggedAlloc(wTaggedHeap* heap, isize tag, usize size);
void wTaggedFree(wTaggedHeap* heap, isize tag);

/* wplMixer interface */

void wMixerInit(wMixer* mixer, unsigned int frequency, int audio_format);
int wMixerGetActiveVoices(wMixer* mixer);
int wMixerPlaySample(wMixer* mixer, wMixerSample* sample, float gain, float pitch, float pan);
int wMixerPlayStream(wMixer* mixer, wMixerStream* stream, float gain);
void wMixerStopVoice(wMixer* mixer, int voice);
void wMixerStopSample(wMixer* mixer, wMixerSample* sample);
void wMixerStopStream(wMixer* mixer, wMixerStream* stream);
void wMixerMixAudio(wMixer* mixer, void* output, unsigned int samples);

/* s-archive interface */
u64 wHashBuffer(const char* buf, isize length);
u64 wHashString(string s);
wSarArchive* wSarLoad(void* file, wMemoryArena* alloc);
isize wSarGetFileIndexByHash(wSarArchive* archive, u64 key);
wSarFile* wSarGetFile(wSarArchive* archive, string name);
void* wSarGetFileData(wSarArchive* archive, string name, 
		isize* sizeOut, wMemoryArena* arena);
