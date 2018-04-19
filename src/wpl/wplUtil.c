#define wHash_FNV64_Basis 14695981039346656037UL
#define wHash_FNV64_Prime 1099511628211UL
u64 wHashBuffer(const char* buf, isize length)
{
	u64 hash = wHash_FNV64_Basis;
	for(isize i = 0; i < length; ++i) {
		hash *= wHash_FNV64_Prime;
		hash ^= buf[i];
	}
	return hash;
}

u64 wHashString(string s)
{
	u64 hash = wHash_FNV64_Basis;
	while(*s != '\0') {
		hash *= wHash_FNV64_Prime;
		hash ^= *s++;
	}
	return hash;
}

void wCopyMemoryBlock(void* dest, const void* source, 
		i32 sx, i32 sy, i32 sw, i32 sh,
		i32 dx, i32 dy, i32 dw, i32 dh,
		i32 size, i32 border)
{
	u8* dst = dest;
	const u8* src = source;
	for(isize i = 0; i < sh; ++i) {
		memcpy(
			dst + ((i+dy) * dw + dx) * size, 
			src + ((i+sy) * sw + sx) * size,
			sw * size);
	}

	if(border) {
		for(isize i = 0; i < sh; ++i) {
			memcpy(
					dst + ((i+dy) * dw + (dx-1)) * size, 
					dst + ((i+sy) * dw + sx) * size,
					1 * size);

			memcpy(
					dst + ((i+dy) * dw + (dx+sw)) * size, 
					src + ((i+sy) * dw + (sx+sw-1)) * size,
					1 * size);
		}

		memcpy(
			dst + ((dy-1) * (dw) + (dx-1)) * size, 
			dst + ((dy) * (dw) + (dx-1)) * size,
			(sw+2) * size);

		memcpy(
			dst + ((dy+sh) * (dw) + (dx-1)) * size, 
			dst + ((dy+sh-1) * (dw) + (dx-1)) * size,
			(sw+2) * size);
	}
}

void wLogError(i32 errorClass, string fmt, VariadicArgs)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
}
