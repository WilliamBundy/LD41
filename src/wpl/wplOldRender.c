/*
struct wSprite
{
	i32 flags;
	u32 color;
	f32 x, y, w, h, cx, cy;
	i16 tx, ty, tw, th;
	f32 angle;
	f32 sdf;
};

struct wVertex
{
	f32 kind;
	f32 x, y;
	f32 u, v;
	f32 sx, sy;
	u32 color;
};

struct wRenderGroup
{
	wTexture* texture;
	wShader* shader;
	u32 vao, vbo;

	i32 blank;
	i32 clearOnDraw;

	f32 dpi;
	f32 scale;
	f32 offsetX, offsetY;
	u32 tint;

	float sdfPxRange;
	float sdfDx, sdfDy;

	wSprite* sprites;
	wVertex* verts;
	u16* indices;
	i64 count, capacity, lastFilled;
};

enum SpriteFlags
{
	Anchor_Center = 0,
	Anchor_TopLeft = 1,
	Anchor_TopCenter = 2,
	Anchor_TopRight = 3,
	Anchor_RightCenter = 4,
	Anchor_BottomRight = 5,
	Anchor_BottomCenter = 6,
	Anchor_BottomLeft = 7,
	Anchor_LeftCenter = 8,
	Sprite_Hidden = 1<<4,
	Sprite_NoTexture = 1<<5,
	Sprite_RotateCW = 1<<6,
	Sprite_RotateCCW = 1<<7,
	Sprite_FlipHoriz = 1<<8,
	Sprite_FlipVert = 1<<9,
	Sprite_Circle = 1<<10,
	Sprite_NoAA = 1<<13,
	Sprite_MSDF = 1<<14,
};
*/


/*
void wInitSprite(wSprite* s);
wSprite* wGroupAddRaw(wRenderGroup* group, 
		i32 flags, 
		u32 color,
		f32 x, f32 y, 
		f32 w, f32 h,
		i16 tx, i16 ty, i16 tw, i16 th, 
		f32 angle);

wSpriteList wDrawText(
		wRenderGroup* group, wFontInfo* info,
		f32 x, f32 y,
		string text, isize count,
		f32 pointSize, f32 maxWidthPixels, 
		u32 color, f32 sdfSharpness);
void wApplyWaveEffect(wRenderGroup* group, wSpriteList* l, f32 t, f32 amplitude, f32 frequency);

void wGroupAdd(wRenderGroup* group, wSprite* sprite);
wSprite* wGetSprite(wRenderGroup* group);
void wGroupInit(wWindow* window, wRenderGroup* group, i64 cap, wShader* shader, wTexture* texture, wMemoryArena* arena);
void wGroupDrawBasic(wState* state, wRenderGroup* group);
void wGroupDraw(wState* state, wRenderGroup* group);
*/

static
void initDefaultShader(wplWindow* window, wplShader* shader)
{
	u32 vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &window->vertShader, NULL);
	glCompileShader(vert);

	{
		i32 success = 1;
		glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
		if(!success) {
			char log[4096];
			i32 logSize = 0;
			glGetShaderInfoLog(vert, 4096, &logSize, log);
			fprintf(stderr, "\nVertex Shader Compile Log\n%s\n\n", log);
		}
	}
	
	u32 frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &window->fragShader, NULL);
	glCompileShader(frag);

	{
		i32 success = 1;
		glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
		if(!success) {
			char log[4096];
			i32 logSize = 0;
			glGetShaderInfoLog(frag, 4096, &logSize, log);
			fprintf(stderr, "\nFragment Shader Compile Log\n%s\n\n", log);
		}
	}

	shader->vert = vert;
	shader->frag = frag;
	shader->program = glCreateProgram();

	glAttachShader(shader->program, vert);
	glAttachShader(shader->program, frag);
	glLinkProgram(shader->program);

	{
		i32 success = 1;
		glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
		if(!success) {
			char log[4096];
			i32 logSize = 0;
			glGetProgramInfoLog(shader->program, 4096, &logSize, log);
			fprintf(stderr, "\nShader Program Link Log\n%s\n\n", log);

		}
	}

	glUseProgram(shader->program);
	/*
	shader->uInvTextureSize = glGetUniformLocation(
			shader->program, "uInvTextureSize");
	shader->uTint = glGetUniformLocation(shader->program, "uTint");
	shader->uPxRange = glGetUniformLocation(shader->program, "uPxRange");
	shader->uScale = glGetUniformLocation(shader->program, "uScale");
	shader->uViewport = glGetUniformLocation(shader->program, "uViewport");
	shader->uOffset = glGetUniformLocation(shader->program, "uOffset");
	*/
}

wplSprite* wplGroupAddRaw(
		wplRenderGroup* group,
		i32 flags,
		u32 color,
		f32 x, f32 y,
		f32 w, f32 h,
		i16 tx, i16 ty, i16 tw, i16 th, 
		f32 angle)
{
	wplSprite s;
	wplInitSprite(&s);
	s.flags = flags;
	s.color = color;
	s.x = x;
	s.y = y;
	s.w = w;
	s.h = h;
	s.cx = 0;
	s.cy = 0;
	s.tx = tx;
	s.ty = ty;
	s.tw = tw;
	s.th = th;
	s.angle = angle;
	group->sprites[group->count++] = s;
	return group->sprites + group->count - 1;
}

void wplInitSprite(wplSprite* s)
{
	s->flags = 0;
	s->color = 0xFFFFFFFF;
	s->x = 0;
	s->y = 0;
	s->w = 0;
	s->h = 0;
	s->tx = 0;
	s->ty = 0;
	s->tw = 0;
	s->th = 0;
	s->angle = 0;
	s->sdf = 1.0f;
}

wplSprite* wplGetSprite(wplRenderGroup* group)
{
	wplSprite* s = group->sprites + group->count++;
	wplInitSprite(s);
	return s;
}

void wplGroupAdd(wplRenderGroup* group, wplSprite* sprite)
{
	wplSprite* s = wplGetSprite(group);
	*s = *sprite;
}

void wplGroupInit(
		wplWindow* window,
		wplRenderGroup* group, 
		i64 cap, 
		wplShader* shader, wplTexture* texture, 
		MemoryArena* arena)
{
	group->dpi = 72.0f;
	group->scale = 1;
	group->clearOnDraw = 1;
	group->offsetX = 0;
	group->offsetY = 0;
	group->tint = 0xFFFFFFFF;

	group->sdfPxRange = 8.0f;
	group->texture = texture;
	group->shader = shader;

	if(!texture->glIndex) {
		wplUploadTexture(texture);
	}

	if(!shader->vert || !shader->frag || !shader->program) {
		initDefaultShader(window, shader);
	}

	group->capacity = cap;
	group->sprites = arenaPush(arena, sizeof(wplSprite) * group->capacity);
	group->verts = NULL; //arenaPush(arena, sizeof(wplVertex) * 4 * group->capacity);
	group->indices = NULL; //arenaPush(arena, sizeof(u16) * 6 * group->capacity);
	group->count = 0;

	glGenVertexArrays(1, &group->vao);
	glBindVertexArray(group->vao);
	glGenBuffers(1, &group->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, group->vbo);

	u32 attribTypes[] = {
		GL_FLOAT, GL_DOUBLE, 
		GL_INT, GL_SHORT, GL_UNSIGNED_BYTE,
		GL_INT, GL_SHORT, GL_UNSIGNED_BYTE,
		GL_INT, GL_SHORT, GL_UNSIGNED_BYTE,
		0, 0, 0
	};

	for(isize i = 0; i < shader->attribCount; ++i) {
		wplShaderComponent* c = shader->attribs + i;
		i32 isNormalized = 0;
		u32 type = attribTypes[c->type - wplShader_Float];
		glEnableVertexAttribArray(c->loc);
		if(glVertexAttribDivisor) glVertexAttribDivisor(c->loc, c->divisor);
		switch(c->type) {
			case wplShader_NormalizedInt:
			case wplShader_NormalizedShort:
			case wplShader_NormalizedByte:
				isNormalized = 1;
			case wplShader_Float:
			case wplShader_Double:
			case wplShader_FloatInt:
			case wplShader_FloatShort:
			case wplShader_FloatByte:
				glVertexAttribPointer(
						c->loc,
						c->count,
						type,
						isNormalized,
						shader->stride,
						(void*)c->ptr);
				break;
			case wplShader_Int:
			case wplShader_Short:
			case wplShader_Byte:
				glVertexAttribIPointer(
						c->loc,
						c->count,
						c->type,
						shader->stride,
						(void*)c->ptr);
				break;
			default:
				break;
		}
	}

	glBindVertexArray(0);
}

const float SoffsetX[] = {0.0, 0.5, 0.0, -0.5, -0.5, -0.5,  0.0,  0.5, 0.5};
const float SoffsetY[] = {0.0, 0.5, 0.5,  0.5,  0.0, -0.5, -0.5, -0.5, 0.0};

static
void groupProcessSprites(wplState* state, wplRenderGroup* group)
{
	vf128 groupScale = _mm_set_ps1(group->scale);
	vf128 offsetXs = _mm_set_ps1(group->offsetX);
	vf128 offsetYs = _mm_set_ps1(group->offsetY);
	vf128 viewportXs = _mm_set_ps1(1.0f/(f32)state->width);
	vf128 viewportYs = _mm_set_ps1(1.0f/(f32)state->height);
	for(isize i = 0; i < group->count; ++i) {
		wplSprite* s = group->sprites + i;
		isize i4 = i * 4;
		wplVertex* p = group->verts + i4;
		u16* indexes = group->indices + (i*6);
		indexes[0] = i4 + 0;
		indexes[1] = i4 + 1;
		indexes[2] = i4 + 2;
		indexes[3] = i4 + 1;
		indexes[4] = i4 + 2;
		indexes[5] = i4 + 3;

		f32 uvrect[4];
		uvrect[0] = (f32)s->tx;
		uvrect[1] = (f32)s->ty;
		uvrect[2] = (f32)(s->tx + s->tw); 
		uvrect[3] = (f32)(s->ty + s->th);

		int f = s->flags & 0xf;
		vf128 xs = _mm_add_ps(_mm_set_ps(-0.5, -0.5, 0.5, 0.5),
				_mm_set1_ps(SoffsetX[f]));
		vf128 ys = _mm_add_ps(_mm_set_ps(0.5, -0.5, 0.5, -0.5),
				_mm_set1_ps(SoffsetY[f]));
		vf128 uvxs = _mm_set_ps(uvrect[0], uvrect[0], uvrect[2], uvrect[2]);
		vf128 uvys = _mm_set_ps(uvrect[3], uvrect[1], uvrect[3], uvrect[1]);


		f32 scaleX = s->w;
		f32 scaleY = s->h;
		if(s->flags & Sprite_RotateCW) {
			uvxs = vfShuffle(uvxs, 3, 1, 2, 0);
			uvys = vfShuffle(uvys, 3, 1, 2, 0);
			scaleX = s->h;
			scaleY = s->w;
		}

		if(s->flags & Sprite_RotateCCW) {
			uvxs = vfShuffle(uvxs, 2, 0, 3, 1);
			uvys = vfShuffle(uvys, 2, 0, 3, 1);
			scaleX = s->h;
			scaleY = s->w;
		}

		{
			vf128 scaleXs = _mm_set_ps1(scaleX * group->scale);
			vf128 scaleYs = _mm_set_ps1(scaleY * group->scale);
			xs = _mm_mul_ps(xs, scaleXs);
			ys = _mm_mul_ps(ys, scaleYs);
		}

		/* Rotations 
		 * -angle yields the correct rotation for inverted Y, matching
		 *  the shader version.
		 * */
		if(s->angle != 0) {
			vf128 lsin, lcos;
			wb_sincos_ps(_mm_set_ps1(-s->angle), &lsin, &lcos);

			vf128 centerX = _mm_set_ps1(s->cx);
			vf128 centerY = _mm_set_ps1(s->cy);

			xs = _mm_sub_ps(xs, centerX);
			ys = _mm_sub_ps(ys, centerY);
			
			vf128 cxs = _mm_mul_ps(lcos, xs);
			vf128 sxs = _mm_mul_ps(lsin, xs);

			vf128 cys = _mm_mul_ps(lcos, ys);
			vf128 sys = _mm_mul_ps(lsin, ys);

			xs = _mm_add_ps(cxs, sys);
			ys = _mm_sub_ps(cys, sxs);

			xs = _mm_add_ps(xs, centerX);
			ys = _mm_add_ps(ys, centerY);
		}

		{
			vf128 pxs = _mm_set_ps1(s->x);
			pxs = _mm_mul_ps(pxs, groupScale);
			xs = _mm_add_ps(xs, pxs);
			xs = _mm_sub_ps(xs, offsetXs);

			vf128 pys = _mm_set_ps1(s->y);
			pys = _mm_mul_ps(pys, groupScale);
			ys = _mm_add_ps(ys, pys);
			ys = _mm_sub_ps(ys, offsetYs);
		}

		/* Normalize the position to -1, 1 based on the window size
		 * Essentially the orthographic matrix transform, flattened
		 * pos * vec2(2, -2) / viewportWH - vec2(1, -1)
		 */
		{
			vf128 number = _mm_set_ps1(2);
			xs = _mm_mul_ps(xs, number);
			xs = _mm_mul_ps(xs, viewportXs);
			number = _mm_set_ps1(1);
			xs = _mm_sub_ps(xs, number);

			number = _mm_set_ps1(-2);
			ys = _mm_mul_ps(ys, number);
			ys = _mm_mul_ps(ys, viewportYs);
			number = _mm_set_ps1(-1);
			ys = _mm_sub_ps(ys, number);
		}

		vf32x4 x = {xs}, y = {ys}, uvx = {uvxs}, uvy = {uvys};
		for(isize j = 0; j < 4; ++j) {
			p[j].x = x.f[j];
			p[j].y = y.f[j];
			p[j].u = uvx.f[j];
			p[j].v = uvy.f[j];
			p[j].color = s->color; 
			p[j].sx = 1;
			p[j].sy = 1;
			if(s->flags & Sprite_MSDF) {
				//TODO(will): Implement SDF effects (outlining and whatnot)
				//	by setting the kind value to N + SDF; 
				p[j].kind = s->sdf;
			} else if(s->flags & Sprite_NoTexture) {
				p[j].kind = 40.0;
			} else if(s->flags & Sprite_NoAA) {
				p[j].kind = 11.0;
			} else {
				p[j].kind = 16.0;
			}
		}
	}
}

void wplGroupDraw(wplState* state, wplRenderGroup* group)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	wplShader* shader = group->shader;
	glUseProgram(shader->program);

	/*
	glUniform2f(shader->uInvTextureSize,
			1.0f / (f32)group->texture->w, 
			1.0f / (f32)group->texture->h);
	glUniform4f(shader->uTint, 
			(f32)(group->tint & 0xFF) / 255.0f, 
			(f32)((group->tint >> 8) & 0xFF) / 255.0f,
			(f32)((group->tint >> 16) & 0xFF) / 255.0f,
			(f32)((group->tint >> 24) & 0xFF) / 255.0f);
	glBindTexture(GL_TEXTURE_2D, group->texture->glIndex);
	glUniform1f(shader->uScale, group->scale);
	glUniform2f(shader->uOffset, group->offsetX, group->offsetY);
	glUniform2f(shader->uViewport, state->width, state->height);
	*/

	glBindVertexArray(group->vao);
	glBindBuffer(GL_ARRAY_BUFFER, group->vbo);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(wplSprite) * group->count,
			group->sprites,
			GL_STREAM_DRAW);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, group->count);

	glBindVertexArray(0);

	if(group->clearOnDraw) {
		group->count = 0;
	}
}
void wplGroupDrawOld(wplState* state, wplRenderGroup* group)
{
	if(group->count == 0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	wplShader* shader = group->shader;
	glUseProgram(shader->program);

	/*
	glUniform2f(shader->uInvTextureSize,
			1.0f / (f32)group->texture->w, 
			1.0f / (f32)group->texture->h);
	glUniform4f(shader->uTint, 
			(f32)(group->tint & 0xFF) / 255.0f, 
			(f32)((group->tint >> 8) & 0xFF) / 255.0f,
			(f32)((group->tint >> 16) & 0xFF) / 255.0f,
			(f32)((group->tint >> 24) & 0xFF) / 255.0f);
	glUniform1f(shader->uPxRange, group->sdfPxRange);
	*/

	glBindTexture(GL_TEXTURE_2D, group->texture->glIndex);

	groupProcessSprites(state, group);
	glBindBuffer(GL_ARRAY_BUFFER, group->vbo);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(wplVertex) * 4 * group->count,
			group->verts,
			GL_STREAM_DRAW);

	glDrawElements(GL_TRIANGLES,
			group->count * 6,
			GL_UNSIGNED_SHORT,
			group->indices);
		
	if(group->clearOnDraw) {
		group->count = 0;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static inline
f32 glyphGetKerning(wplFontInfo* info, char c, char last)
{
	if(last <= 32 || last >= 127) return 0.0f;
	return info->kerning[last-32][c-32];
}

wplSpriteList wplDrawText(
		wplRenderGroup* group, wplFontInfo* info,
		f32 x, f32 y,
		string text, isize count,
		f32 pointSize, f32 maxWidthPixels, 
		u32 color, f32 sdfSharpness)
{

	if(count == -1) count = strlen(text);
	else if(count <= -1000) {
		isize c = strlen(text);
		count *= -1;
		count -= 1000;
		if(count > c) count = c;
	}
	f32 ox = 0.0f, oy = 0.0f;
	char last = 0;

	f32 pixelSize = (pointSize * group->dpi) / 72.0f;
	f32 padding = (f32)info->pxRange;
	f32 fontScale = info->scale;

	wplGlyphImage* a = NULL;
	wplGlyph* g = info->glyphs + ('A'-32);
	f32 glyphHeight = fabsf(g->t - g->b);
	f32 scaledHeight = glyphHeight * fontScale;
	f32 scaledRatio = pixelSize / scaledHeight;
	f32 heightRatio = pixelSize / glyphHeight;
	wplSpriteList l;
	l.start = group->count;

	f32 maxX = 0;
	f32 widthP = 0;

	for(isize i = 0; i < count; ++i) {
		char c = text[i];
		switch(c) {
			case '\r':
				continue;

			case '\n':
				if(ox > maxX) maxX = ox;
				ox = 0;
				oy += info->lineSpacing * heightRatio;
				continue;

			case '\t':
				ox += info->glyphs[0].advance * heightRatio * 8;
				continue;

			case ' ':
				ox += info->glyphs[0].advance * heightRatio;
				continue;
		}
		if(c <= 32 || c >= 127) continue;
		a = info->images + (c-32);
		g = info->glyphs + (c-32);

		ox += glyphGetKerning(info, c, last) * pixelSize * fontScale * 0.5f;
		f32 gx = (a->bbx - padding) * scaledRatio;
		if(i == 0) {
			widthP += gx;
			ox -= gx * 1.25;
		}
		wplSprite* s = wplGroupAddRaw(group, Anchor_TopLeft | Sprite_MSDF, color,
				x + ox + gx, y + oy,
				a->w * scaledRatio, a->h * scaledRatio,
				a->x + info->atlasX, a->y + info->atlasY,
				a->w, a->h, 
				0);
		s->sdf = sdfSharpness;
		ox += g->advance * heightRatio;
		if(ox > maxX) maxX = ox;
		last = c;
	}
	g = info->glyphs + ('A'-32);
	a = info->images + ('A'-32);
	l.count = group->count - l.start;
	l.l = x;
	l.t = y + a->bby * scaledRatio;
	l.r = x + maxX + widthP + padding * scaledRatio * 0.5f;
	l.b = y + oy + a->h * scaledRatio;
	return l;
}

void wplApplyWaveEffect(wplRenderGroup* group, wplSpriteList* l, 
		f32 t, f32 amplitude, f32 frequency)
{
	for(isize i = 0; i < l->count; ++i) {
		wplSprite* s = group->sprites + l->start + i;
		s->y += sinf(t - (f32)i / frequency) * amplitude;
	}
}
