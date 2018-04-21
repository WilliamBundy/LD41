

#define FNV64_Basis 14695981039346656037UL
#define FNV64_Prime 1099511628211UL
u64 hashBuffer(const char* buf, isize length)
{
	u64 hash = FNV64_Basis;
	for(isize i = 0; i < length; ++i) {
		hash *= FNV64_Prime;
		hash ^= buf[i];
	}
	return hash;
}

u64 hashString(string s)
{
	u64 hash = FNV64_Basis;
	while(*s != '\0') {
		hash *= FNV64_Prime;
		hash ^= *s++;
	}
	return hash;
}

void segmentSort(TextureSegment* array, isize count)
{
	for(isize i = 1; i < count; ++i) {
		isize j = i - 1;
		if(array[j].hash > array[i].hash) {
			TextureSegment temp = array[i];
			while((j >= 0) && (array[j].hash > temp.hash)) {
				array[j + 1] = array[j];
				j--;
			}
			array[j + 1] = temp;
		}
	}
}

typedef u32 Pixel;

#ifndef WirmphtEnabled
struct TextureSegment
{
	char name[16];
	u64 hash;
	Rect2i region;
	Vec2 size;
	u8* data;
};

struct TextureAtlas
{
	Texture texture;
	TextureSegment* segments;
	i32 segmentCapacity;
	i32 segmentCount;
};

#endif

void atlasInit(TextureAtlas* atlas, 
		i16 w, i16 h, 
		i32 segmentCapacity,
		MemoryArena* arena)
{
	atlas->texture = (Texture){0, v2i(w, h), NULL};
	atlas->segmentCapacity = segmentCapacity;
	atlas->segmentCount = 0;
	atlas->texture.pixels = arenaPush(arena, sizeof(Pixel) * w * h);
	atlas->segments = arenaPush(arena, sizeof(TextureSegment) * segmentCapacity);
}

//Use a temporary arena
void atlasAdd(TextureAtlas* atlas, 
		Texture* texture, string spriteSheet, 
		MemoryArena* arena)
{
	if(!texture->pixels)fprintf(stderr, "Error: NULL pixels on a texture\n");
	i32 count = countSpriteSheet(spriteSheet) + 1;
	TextureSegment* segments = arenaPush(arena, sizeof(TextureSegment) * count);
	count = parseSpriteSheet(spriteSheet, texture, segments, count + 1);
	for(isize i = 0; i < count; ++i) {
		TextureSegment* s = segments + i;
		s->data = arenaPush(arena, s->region.w * s->region.h * sizeof(Pixel));
		for(isize y = 0; y < s->region.h; ++y) {
			memcpy(s->data + y * s->region.w * sizeof(Pixel),
				texture->pixels + ((s->region.y + y) * 
					texture->size.x + s->region.x) * sizeof(Pixel),
				s->region.w * sizeof(Pixel));
		}
		atlas->segments[atlas->segmentCount++] = *s;
	}
}


void atlasAddFile(TextureAtlas* atlas, string textureName, string spriteSheet, MemoryArena* arena)
{
	Texture* texture = textureFileLoad(textureName, arena);
	atlasAdd(atlas, texture, spriteSheet, arena);
}

void atlasWriteSegment(TextureAtlas* atlas, TextureSegment* segment, i32 border)
{
	Rect2i area = segment->region;
	Texture* texture = &atlas->texture;
	u8* pixels = texture->pixels;
	for(isize y = 0; y < area.h; ++y) {
		memcpy(pixels + ((area.y + y) * texture->size.x + area.x) * sizeof(Pixel),
			segment->data + y * area.w * sizeof(Pixel),
			area.w * sizeof(Pixel));
	}

	if(border) {
		//top and bottom rows
		isize y = -1;
		memcpy(pixels + ((area.y + y) * texture->size.x + area.x) * sizeof(Pixel),
			segment->data + area.w * sizeof(Pixel),
			area.w * sizeof(Pixel));
		y = area.h;
		memcpy(pixels + ((area.y + y) * texture->size.x + area.x) * sizeof(Pixel),
			segment->data + (y-1) * area.w * sizeof(Pixel),
			area.w * sizeof(Pixel));
		//left and right side
		//why we didn't do this in the first place? dunno
		Pixel* data = (void*)pixels;
		for(isize y = 0; y < area.h; ++y) {
			isize index = (area.y + y) * 
				texture->size.x + area.x - 1 * sizeof(Pixel);
			data[index] = data[index + 1];
			data[index + area.w] = data[index + area.w - 1];
		}
	}
}


//Use a temporary arena
//Clears segment->data to null for all packed rects

//In the case packing fails, it returns without doing anything.
//You should know whether or not packing succeeds for any given combination
//of textures, so just fix it
i32 atlasFinalize(TextureAtlas* atlas, MemoryArena* arena)
{
	stbrp_rect* rects = arenaPush(arena, sizeof(stbrp_rect) * atlas->segmentCount);
	for(isize i = 0; i < atlas->segmentCount; ++i) {
		TextureSegment* s = atlas->segments + i;
		stbrp_rect* r = rects + i;
		r->id = i;
		r->w = s->region.w + 2;
		r->h = s->region.h + 2;
	}

	stbrp_context ctx = {0};
	Vec2i size = atlas->texture.size;
	stbrp_node* nodes = arenaPush(arena, sizeof(stbrp_node) * size.x + 1);
	stbrp_init_target(&ctx, size.x, size.y, nodes, size.x + 1);
	i32 ret = stbrp_pack_rects(&ctx, rects, atlas->segmentCount);
	if(!ret) return 0;

	for(isize i = 0; i < atlas->segmentCount; ++i) {
		stbrp_rect* r = rects + i;
		TextureSegment* s = atlas->segments + r->id;
		s->region.x = r->x + 1;
		s->region.y = r->y + 1;
		s->size = v2(s->region.w, s->region.h);
		atlasWriteSegment(atlas, s, 0);
		s->data = NULL;
		s->hash = hashString(s->name);
	}

	segmentSort(atlas->segments, atlas->segmentCount);
	return 1;
}

isize atlasGetIndex(TextureAtlas* atlas, string name)
{
	u64 key = hashString(name);
	u64 localKey = 0;
	isize min = 0, max = atlas->segmentCount - 1, mid = 0;
	while(min <= max) {
		mid = (min + max) / 2;
		localKey = atlas->segments[mid].hash;
		if(localKey == key) {
			return mid;
		} else if(localKey < key) {
			min = mid + 1;
		} else {
			max = mid - 1;
		}
	}
	return -1;
}

TextureSegment* atlasGetSegment(TextureAtlas* atlas, string name)
{
	isize index = atlasGetIndex(atlas, name);
	if(index == -1) {
		return NULL;
	}

	return atlas->segments + index;
}

Vec2i atlasGetSize(TextureAtlas* atlas, string name)
{
	isize index = atlasGetIndex(atlas, name);
	if(index == -1) {
		return v2i(-1, -1);
	}

	TextureSegment* s = atlas->segments + index;
	return v2i(s->region.w, s->region.h);
}

Vec2i atlasGetXY(TextureAtlas* atlas, string name)
{
	isize index = atlasGetIndex(atlas, name);
	if(index == -1) {
		return v2i(-1, -1);
	}

	TextureSegment* s = atlas->segments + index;
	return v2i(s->region.x, s->region.y);
}

Rect2i atlasGet(TextureAtlas* atlas, string name)
{
	isize index = atlasGetIndex(atlas, name);
	if(index == -1) {
		return r2i(-1, -1, 0, 0);
	}

	return atlas->segments[index].region;
}

i32 isWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\r';
}

i32 isNumber(char c)
{
	return c >= '0' && c <= '9';
}

i32 stringToDecimal(string s, isize len)
{
	i32 result = 0;
	for(isize i = 0; i < len; ++i) {
		result *= 10;
		result += s[i] - '0';
	}
	return result;
}

i32 matchstring(string text, string a, i32 len)
{
	isize i = 0;
	while(i < len) {
		if(text[i] != a[i]) return 0;
		if(text[i] == '\0' || a[i] == '\0') return 0;
		++i;	
	}
	return 1;
}

//spritesheet:
// @ssv1 is the title/version line
// # comment
// identifier x y w h
// or: a\0 (just "a" normally) for the entire thing, named after the first
// 16 chars
i32 parseSpriteSheet(string text, Texture* texture, TextureSegment* segments, isize capacity) 
{
	if(text[0] == 'a' && text[1] == '\0')  {
		TextureSegment* seg = segments;
		memset(seg->name, 0, 16);
		i32 nameLen = strlen(texture->name);
		string localName = texture->name;
		for(isize i = 0; i < nameLen; ++i) {
			if(texture->name[i] == '/') {
				localName += i + 1;
			}
		}
		nameLen = strlen(localName);
		memcpy(seg->name, localName, nameLen < 16 ? nameLen : 16);
		seg->name[15] = 0;
		seg->region = r2i(0, 0, texture->size.x, texture->size.y);
		return 1;
	}

	i32 error = 0;
	//advance to the next non-whitespace or non-newline character
	while(*text && (isWhitespace(*text) || *text == '\n')) text++;

	//check version line
	if(!matchstring(text, "@ssv1", 5)) {
		error = 1;
	} else {
		text += sizeof("@ssv1") - 1; 
	}

	i32 count = 0;
	while(*text) {
		while(*text && (isWhitespace(*text) || *text == '\n')) text++;
		if(count >= capacity) {
			return capacity;
		}
		TextureSegment* seg = segments + count++;
		seg->data = NULL;
		
		while(*text && (*text == '#' || isWhitespace(*text) || *text == '\n')) {
			while(*text && (*text != '\n')) text++;
			text++; //start next line
		}
		if(!*text) break;

		string nameStart = text;
		while(*text && !isWhitespace(*text)) text++;
		isize nameLen = text - nameStart;
		memset(seg->name, 0, 16);
		memcpy(seg->name, nameStart, nameLen < 16 ? nameLen : 16);
		seg->name[15] = 0;

#define extractNumber(name) \
		while(*text && isWhitespace(*text)) text++; \
		string name ## Start = text; \
		while(*text && isNumber(*text)) text++; \
		isize name ## Len = text - name ## Start; \
		i32 name ## Val = stringToDecimal(name ## Start, name ## Len); \
		//printf("%d\n", name ## Val);

		extractNumber(x);
		extractNumber(y);
		extractNumber(w);
		extractNumber(h);

#undef extractNumber

		seg->region = r2i(xVal, yVal, wVal, hVal);
		while(*text && (isWhitespace(*text) || *text == '\n')) text++;

	}
	return count;
}

isize countSpriteSheet(string text) 
{
	if(text[0] == 'a' && text[1] == '\0') return 1;
	i32 error = 0;
	//advance to the next non-whitespace or non-newline character
	while(*text && (isWhitespace(*text) || *text == '\n')) text++;

	//check version line
	if(!matchstring(text, "@ssv1", 5)) {
		error = 1;
	} else {
		text += sizeof("@ssv1") - 1; 
	}


	i32 count = 0;
	while(*text) {
		while(*text && (isWhitespace(*text) || *text == '\n')) text++;
		while(*text && (*text == '#' || isWhitespace(*text) || *text == '\n')) {
			while(*text && (*text != '\n')) text++;
			text++; //start next line
		}

		if(!*text) break;


		count++;
		string nameStart = text;
		while(*text && !isWhitespace(*text)) text++;
		isize nameLen = text - nameStart;

#define extractNumber(name) \
		while(*text && isWhitespace(*text)) text++; \
		string name ## Start = text; \
		while(*text && isNumber(*text)) text++; \
		isize name ## Len = text - name ## Start; \
		//printf("%d\n", name ## Val);

		extractNumber(x);
		extractNumber(y);
		extractNumber(w);
		extractNumber(h);

#undef extractNumber
	}
	return count;
}
