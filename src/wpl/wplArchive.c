
#define TINFL_IMPLEMENTATION
#include "thirdparty/tinfl.h"

wSarArchive* wSarLoad(void* file, wMemoryArena* alloc)
{
	wSarArchive* archive = wArenaPush(alloc, sizeof(wSarArchive));
	archive->base = file;
	archive->header = file;

	if(archive->header->magic != wSar_Magic) {
		printf("S-archive: Wrong magic?\n");
		printf("Expected: %u, Got: %u\n", wSar_Magic, archive->header->magic);
	}

	if(archive->header->version > wSar_Version) {
		printf("S-archive: Wrong version?\n");
		printf("Expected: %u, Got: %u\n", wSar_Version, archive->header->version);
	}
	archive->description = (void*)((usize)file + sizeof(wSarHeader));
	archive->files = (void*)(archive->base + archive->header->fileTableLocation);
	return archive;
}


isize wSarGetFileIndexByHash(wSarArchive* archive, u64 key)
{
	u64 localKey = 0;
	isize min = 0, max = archive->header->fileCount - 1, mid = 0;
	while(min <= max) {
		mid = (min + max) / 2;
		localKey = archive->files[mid].id.hash;
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

wSarFile* wSarGetFile(wSarArchive* archive, string name)
{
	u64 hash = wHashString(name);
	isize index = wSarGetFileIndexByHash(archive, hash);
	if(hash == -1) return NULL;
	return archive->files + index;
}

void* wSarGetFileData(wSarArchive* archive, string name, 
		isize* sizeOut, wMemoryArena* arena)
{
	wSarFile* file = wSarGetFile(archive, name);
	void* input = archive->base + file->location;
	void* output = wArenaPush(arena, file->fullSize + 8);
	wDecompressMemToMem(
			output, file->fullSize + 8, 
			input, file->compressedSize, 
			0);
	if(sizeOut) {
		*sizeOut = file->fullSize;
	}
	return output;
}

