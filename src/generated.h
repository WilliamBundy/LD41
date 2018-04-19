typedef union vf32x4 vf32x4;
typedef struct wSarFile wSarFile;
typedef struct wSarHeader wSarHeader;
typedef struct wSarArchive wSarArchive;
typedef struct wMemoryInfo wMemoryInfo;
typedef struct wMemoryArena wMemoryArena;
typedef struct wMemoryPool wMemoryPool;
typedef struct wTaggedHeapArena wTaggedHeapArena;
typedef struct wTaggedHeap wTaggedHeap;
typedef struct wMixerSample wMixerSample;
typedef struct wMixerStream wMixerStream;
typedef struct wMixerVoice wMixerVoice;
typedef struct wMixer wMixer;
typedef struct wWindowDef wWindowDef;
typedef struct wWindow wWindow;
typedef struct wInputState wInputState;
typedef struct wAudioState wAudioState;
typedef struct wState wState;
typedef struct wShaderComponent wShaderComponent;
typedef struct wShader wShader;
typedef struct wTexture wTexture;
typedef struct wRenderBatch wRenderBatch;
typedef struct wGlyph wGlyph;
typedef struct wGlyphImage wGlyphImage;
typedef struct wFontInfo wFontInfo;
typedef struct WidgetStyle WidgetStyle;
typedef struct Widget Widget;
typedef struct Gui Gui;
struct Sprite_;
struct SpriteBatch_;
typedef struct Game Game;
typedef struct createGraphicsDependencies createGraphicsDependencies;
typedef enum ButtonState ButtonState;
typedef enum WidgetKinds WidgetKinds;
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
typedef void(* wMixerStreamProc)( wMixerSample* sample, void* userdata);
typedef const char* string;

union vf32x4
{
	vf128 v;
	f32 f[4];
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
	char *base;
	wSarHeader *header;
	char *description;
	wSarFile *files;
};

struct wMemoryInfo
{
	usize totalMemory;
	usize commitSize;
	usize pageSize;
	isize commitFlags;
};

struct wMemoryArena
{
	const char *name;
	void *start;
	void *head;
	void *end;
	void *tempStart;
	void *tempHead;
	wMemoryInfo info;
	isize align;
	isize flags;
};

struct wMemoryPool
{
	usize elementSize;
	isize count;
	isize capacity;
	void *slots;
	const char *name;
	void **freeList;
	wMemoryArena *alloc;
	isize lastFilled;
	isize flags;
};

struct wTaggedHeapArena
{
	isize tag;
	wTaggedHeapArena *next;
	void *head;
	void *end;
	char buffer;
};

struct wTaggedHeap
{
	const char *name;
	wMemoryPool pool;
	wTaggedHeapArena *arenas[64];
	wMemoryInfo info;
	usize arenaSize;
	usize align;
	isize flags;
};

struct wMixerSample
{
	u32 length;
	u32 frequency;
	void *data;
};

struct wMixerStream
{
	void *userdata;
	wMixerStreamProc callback;
	wMixerSample sample;
};

struct wMixerVoice
{
	wMixerSample *sample;
	wMixerStream *stream;
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
	wMixerVoice *voices;
};

struct wWindowDef
{
	string title;
	i64 width;
	i64 height;
	i64 posCentered;
	i64 posUndefined;
	i64 x;
	i64 y;
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
	i8 *basePath;
	const u8 *vertShader;
	const u8 *fragShader;
	void *windowHandle;
};

struct wInputState
{
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
	wInputState *input;
	i64 width;
	i64 height;
	i64 hasFocus;
	i64 mouseX;
	i64 mouseY;
	i64 exitEvent;
};

struct wShaderComponent
{
	string name;
	i32 loc;
	i32 divisor;
	i32 type;
	i32 count;
	usize ptr;
};

struct wShader
{
	u32 vert;
	u32 frag;
	u32 program;
	i32 targetVersion;
	i32 defaultDivisor;
	i32 stride;
	i32 attribCount;
	i32 uniformCount;
	wShaderComponent attribs[Shader_MaxAttribs];
	wShaderComponent uniforms[Shader_MaxUniforms];
};

struct wTexture
{
	i64 w;
	i64 h;
	u8 *pixels;
	u32 glIndex;
};

struct wRenderBatch
{
	wTexture *texture;
	wShader *shader;
	u32 vao;
	u32 vbo;
	isize elementSize;
	isize elementCount;
	isize instanceSize;
	isize indicesCount;
	isize startOffset;
	void *data;
	u32 *indices;
	i32 clearOnDraw;
	i32 renderCall;
	i32 blend;
	i32 primitiveMode;
};

struct wGlyph
{
	int character;
	f32 width;
	f32 height;
	f32 x;
	f32 y;
	f32 advance;
	f32 l;
	f32 b;
	f32 r;
	f32 t;
};

struct wGlyphImage
{
	i32 x;
	i32 y;
	i32 w;
	i32 h;
	f32 bbx;
	f32 bby;
};

struct wFontInfo
{
	i32 sizeX;
	i32 sizeY;
	i32 scale;
	i32 offsetX;
	i32 offsetY;
	i32 pxRange;
	i32 lineSpacing;
	i32 atlasX;
	i32 atlasY;
	wGlyph glyphs[96];
	wGlyphImage images[96];
	f32 kerning[96][96];
};

struct WidgetStyle
{
	u32 color;
};

struct Widget
{
	i16 kind;
	i16 style;
	i32 flags;
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	union {
		struct {
			char chars[15];
			char nullTerm;
		};
		struct {
			char *text;
			isize textLength;
		};
	};
	i32 links[4];
	i32 *output;
};

struct Gui
{
	struct Widget *current;
	struct Widget *widgets;
	struct WidgetStyles *styles;
	i32 count;
	i32 capacity;
};

typedef struct Sprite_
{
	i32 flags;
	u32 color;
	f32 x;
	f32 y;
	f32 z;
	f32 angle;
	f32 w;
	f32 h;
	f32 cx;
	f32 cy;
	i16 tx;
	i16 ty;
	i16 tw;
	i16 th;
} Sprite;

typedef struct SpriteBatch_
{
	wRenderBatch batch;
	Sprite *sprites;
	isize count;
	isize capacity;
	f32 x;
	f32 y;
	f32 vw;
	f32 vh;
	f32 scale;
	u32 tint;
	f32 itw;
	f32 ith;
} SpriteBatch;

struct Game
{
	wMemoryInfo memInfo;
	wMemoryArena *arena;
	wWindow window;
	wState state;
	wInputState input;
	wShader *shader;
	wTexture *texture;
	SpriteBatch *batch;
};

struct createGraphicsDependencies
{
	isize textureSize;
	u8 textureData wLoadLocalFile game *window;
	u8 textureData wLoadLocalFile game (null);
	u8 textureData wLoadLocalFile game textureSize;
	u8 textureData wLoadLocalFile game arena;
	game texture wArenaPush game arena;
	game texture wArenaPush game wTexture;
	wInitTexture game texture;
	wInitTexture game textureData;
	wInitTexture game textureSize;
	wUploadTexture game texture;
	game shader wArenaPush game arena;
	game shader wArenaPush game wShader;
	wInitShader game shader;
	wInitShader game Sprite;
	game shader defaultDivisor;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_Int;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game flags;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_NormalizedByte;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game color;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_Float;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game x;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_Float;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game angle;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_Float;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game w;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_Float;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game cx;
	wCreateAttrib game shader;
	wCreateAttrib game (null);
	wCreateAttrib game wShader_FloatShort;
	wCreateAttrib game (null);
	wCreateAttrib game Sprite;
	wCreateAttrib game tx;
	wCreateUniform game shader;
	wCreateUniform game (null);
	wCreateUniform game wShader_Float;
	wCreateUniform game (null);
	wCreateUniform game SpriteBatch;
	wCreateUniform game x;
	wCreateUniform game shader;
	wCreateUniform game (null);
	wCreateUniform game wShader_Float;
	wCreateUniform game (null);
	wCreateUniform game SpriteBatch;
	wCreateUniform game vw;
	wCreateUniform game shader;
	wCreateUniform game (null);
	wCreateUniform game wShader_Float;
	wCreateUniform game (null);
	wCreateUniform game SpriteBatch;
	wCreateUniform game scale;
	wCreateUniform game shader;
	wCreateUniform game (null);
	wCreateUniform game wShader_NormalizedByte;
	wCreateUniform game (null);
	wCreateUniform game SpriteBatch;
	wCreateUniform game tint;
	wCreateUniform game shader;
	wCreateUniform game (null);
	wCreateUniform game wShader_Float;
	wCreateUniform game (null);
	wCreateUniform game SpriteBatch;
	wCreateUniform game itw;
	wAddSourceToShader game shader;
	wAddSourceToShader game GL33_vert;
	wAddSourceToShader game wShader_Vertex;
	wAddSourceToShader game shader;
	wAddSourceToShader game GL33_frag;
	wAddSourceToShader game wShader_Frag;
	wFinalizeShader game shader;
};

void initSprite(Sprite* s, i32 flags, u32 color, f32 x, f32 y, f32 z, f32 angle, f32 w, f32 h, f32 cx, f32 cy, i16 tx, i16 ty, i16 tw, i16 th);
void createGraphicsDependencies();
SpriteBatch* createSpriteBatch(isize cap, wMemoryArena* arena);
void drawSprites(SpriteBatch* batch);
void addSquare(f32 x, f32 y);
void update();
void GameMain();
int main(int argc, char** argv);
