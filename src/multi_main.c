#include "wpl/wpl.h"

#include "ui.c"
#include "shaders.h"

typedef struct
{
	f32 flags;
	u32 color;
	f32 x, y, z;
	f32 angle;
	f32 w, h;
	f32 cx, cy;
	i16 tx, ty, tw, th;
} Sprite;

void initSprite(Sprite* s,
		f32 flags, u32 color, 
		f32 x, f32 y, f32 z,
		f32 angle, f32 w, f32 h, f32 cx, f32 cy, 
		i16 tx, i16 ty, i16 tw, i16 th)
{
	s->flags = flags;
	s->color = color;
	s->x = x;
	s->y = y;
	s->z = z;
	s->angle = angle;
	s->w = w;
	s->h = h;
	s->cx = cx;
	s->cy = cy;
	s->tx = tx;
	s->ty = ty;
	s->tw = tw;
	s->th = th;
}

typedef struct
{
	wRenderBatch batch;
	Sprite* sprites;
	isize count, capacity;

	f32 x, y;
	f32 vw, vh;
	f32 scale;
	u32 tint;
	f32 itw, ith;
} SpriteBatch;

struct Game
{
	wMemoryInfo memInfo;
	wMemoryArena* arena;
	wWindow window;
	wState state;
	wInputState input;


	wShader* shader;
	wTexture* texture;
	SpriteBatch* batch;
};
struct Game game;
wMemoryArena* arena;

void createGraphicsDependencies()
{
	isize textureSize = 0;
	u8* textureData = wLoadLocalFile(
			&game.window, 
			"texture.png", 
			&textureSize, 
			game.arena);
	game.texture = wArenaPush(game.arena, sizeof(wTexture));
//#ifndef WPL_EMSCRIPTEN
	wInitTexture(game.texture, textureData, textureSize);
	wUploadTexture(game.texture);
//#endif

	game.shader = wArenaPush(game.arena, sizeof(wShader));
	wInitShader(game.shader, sizeof(Sprite));
	game.shader->defaultDivisor = 1;

	wCreateAttrib(game.shader,
			"vFlags", wShader_Float, 1, offsetof(Sprite, flags));
	wCreateAttrib(game.shader, 
			"vColor", wShader_NormalizedByte, 4, offsetof(Sprite, color));
	wCreateAttrib(game.shader,
			"vPos", wShader_Float, 3, offsetof(Sprite, x));
	wCreateAttrib(game.shader,
			"vAngle", wShader_Float, 1, offsetof(Sprite, angle));
	wCreateAttrib(game.shader,
			"vSize", wShader_Float, 2, offsetof(Sprite, w));
	wCreateAttrib(game.shader, 
			"vCenter", wShader_Float, 2, offsetof(Sprite, cx));
	wCreateAttrib(game.shader, 
			"vTexture", wShader_FloatShort, 4, offsetof(Sprite, tx));

	wCreateUniform(game.shader, 
			"uOffset", wShader_Float, 2, offsetof(SpriteBatch, x));
	wCreateUniform(game.shader, 
			"uViewport", wShader_Float, 2, offsetof(SpriteBatch, vw));
	wCreateUniform(game.shader, 
			"uScale", wShader_Float, 1, offsetof(SpriteBatch, scale));
	wCreateUniform(game.shader, 
			"uTint", wShader_NormalizedByte, 4, offsetof(SpriteBatch, tint));
	wCreateUniform(game.shader, 
			"uInvTextureSize", wShader_Float, 2, offsetof(SpriteBatch, itw));

	wAddSourceToShader(game.shader, EGL3_vert, wShader_Vertex);
	wAddSourceToShader(game.shader, EGL3_frag, wShader_Frag);

	wFinalizeShader(game.shader);
}

SpriteBatch* createSpriteBatch(isize cap, wMemoryArena* arena)
{
	SpriteBatch* batch = wArenaPush(arena, sizeof(SpriteBatch));
	batch->sprites = wArenaPush(arena, sizeof(Sprite) * cap);
	batch->capacity = cap;
	wInitBatch(&batch->batch,
			game.texture, game.shader,
			wRenderBatch_ArraysInstanced, wRenderBatch_TriangleStrip,
			sizeof(Sprite), 4,
			batch->sprites, NULL);
	wConstructBatchGraphicsState(&batch->batch);

	batch->scale = 1.0;
	batch->tint = 0xFFFFFFFF;
	return batch;
}

void drawSprites(SpriteBatch* batch)
{
	wRenderBatch* rb = &batch->batch;
	rb->elementCount = batch->count;
	batch->vw = game.state.width;
	batch->vh = game.state.height;
	batch->itw = 1.0f / rb->texture->w;
	batch->ith = 1.0f / rb->texture->h;

	wDrawBatch(&game.state, rb, batch);

	batch->count = 0;
}

f32 t = 0;
void addSquare(f32 x, f32 y)
{
	Sprite s = {0};
	initSprite(&s, 
			0, 0xFFFFFFFF,
			x, y, 0, t,
			32, 32,
			0, 0,
			0, 0, 256, 256);
	game.batch->sprites[game.batch->count++] = s;
}

void update()
{
	t += 0.005;
	addSquare(100, 100);
	drawSprites(game.batch);
}

#ifdef WPL_EMSCRIPTEN
#include <emscripten.h>
void mainloop()
{
	wUpdate(&game.window, &game.state);
	update();
	wRender(&game.window);
}
#endif


//int main(int argc, char** argv)
void GameMain()
{
#ifdef WPL_WIN32_BACKEND
	wWindowDef def = wDefineWindow("TestApp - Win32/NoCRT");
#else
#ifdef WPL_SDL_BACKEND
#ifdef WPL_EMSCRIPTEN
	wWindowDef def = wDefineWindow("TestApp - Emscripten");
#else
	wWindowDef def = wDefineWindow("TestApp - SDL2/CRT");
#endif
#endif
#endif
	wCreateWindow(&def, &game.window);
	wInitState(&game.state, &game.input);

	game.memInfo = wGetMemoryInfo();
	game.arena = wArenaBootstrap(game.memInfo, 0);

	createGraphicsDependencies();
	game.batch = createSpriteBatch(4096, game.arena);
#ifdef WPL_EMSCRIPTEN
	emscripten_set_main_loop(mainloop, 60, 1);
#endif

#ifndef WPL_EMSCRIPTEN
	i32 running = 1;
	while(!game.state.exitEvent) {
		wUpdate(&game.window, &game.state);
		update();
		wRender(&game.window);
	}
#endif

	//return 0;
	wQuit();
}

#ifdef WPL_SDL_BACKEND
int main(int argc, char** argv)
{
	GameMain();
	return 0;
}
#endif
