/* Sar, the sane archive
 * usage:
 * sar <archive.sar> x|c <files>
 */ 

#include <Windows.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <intrin.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MZ_ASSERT(x)
#include "miniz.h"
#include "miniz.c"
#include "..\wpl\wpl.h"

#include "tinydir.h"

#define WB_ALLOC_IMPLEMENTATION
#define WB_ALLOC_CUSTOM_INTEGER_TYPES
#define WB_ALLOC_BACKEND_API static
#include "..\wpl\thirdparty\wb_alloc.h"

typedef uint32_t u32;
typedef uint64_t u64;

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
	u64 hash = wSarHashString(name);
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
	tinfl_decompress_mem_to_mem(
			output, file->fullSize + 8, 
			input, file->compressedSize, 
			0);
	if(sizeOut) {
		*sizeOut = file->fullSize;
	}
	return output;
}

#define wSar_Editing_Implementation
#ifdef wSar_Editing_Implementation
struct wSarEditingArchive
{
	wMemoryArena *finalAlloc, *tableAlloc, *dataAlloc;


	wSarHeader* header;
	char* description;
	
	u64 fileCount;
	//While in the editingarchive, file->location refers
	//to the offset within the dataAlloc thing
	wSarFile* fileTable;
	void* fileData;

};

wSarEditingArchive* wSarCreateEditingArchive(wSarArchive* existing)
{
	wSarEditingArchive local = {0}, *e; 
	wMemoryInfo memInfo = wGetMemoryInfo();
	local.finalAlloc = wArenaBootstrap(memInfo, 0);
	local.tableAlloc = wArenaBootstrap(memInfo, 0);
	local.dataAlloc = wArenaBootstrap(memInfo, 0);
	e = wArenaPush(local.finalAlloc, sizeof(wSarEditingArchive));
	*e = local;

	e->header = wArenaPush(e->finalAlloc, sizeof(wSarHeader));
	e->fileCount = 0;
	e->fileTable = e->tableAlloc->head;
	e->fileData = e->dataAlloc->head;

	if(existing) {
		wSarHeader* exhead = existing->header;
		*e->header = *exhead;
		e->header->magic = wSar_Magic;
		e->header->version = wSar_Version;
		if(exhead->descriptionLength > 0) {
			e->description = wArenaPush(e->finalAlloc, exhead->descriptionLength);
		}

		// copy file table and data over
		if(exhead->fileCount > 0) {
			usize ftsize = sizeof(wSarFile) * exhead->fileCount;
			wArenaPush(e->tableAlloc, ftsize);
			memcpy(e->fileTable, existing->files, ftsize);
			e->fileCount = exhead->fileCount;
			e->header->fileCount = e->fileCount;
			for(usize i = 0; i < e->fileCount; ++i) {
				wSarFile* f = e->fileTable + i;
				void* data = wArenaPush(e->dataAlloc, f->compressedSize);
				memcpy(data, existing->base + f->location, f->compressedSize);
				f->location = (usize)data - (usize)e->fileData;
			}

		}

	}
	return e;
}

/*		void *tdefl_compress_mem_to_heap(
 * 			void *src, 
 * 			size_t src_len, 
 * 			size_t *out_len, 
 * 			int flags);
 **/
void wSarAddFile(wSarEditingArchive* e, string name, void* data, isize size)
{
	e->fileCount++;
	e->header->fileCount++;
	wSarFile* file = wArenaPush(e->tableAlloc, sizeof(wSarFile));
	isize namelen = strlen(name);
	memcpy(file->id.name, name, namelen <= wSar_NameLen ? namelen : wSar_NameLen);
	file->id.hash = wSarHashString(file->id.name);
	file->fullSize = size;
	//TODO(will) compress and copy data
	usize compressedSize = 0;
	void* compressed = tdefl_compress_mem_to_heap(data, size, &compressedSize, 0);
	if(compressed) {
		void* finalBuf = wArenaPush(e->dataAlloc, compressedSize);
		memcpy(finalBuf, compressed, compressedSize);
		file->compressedSize = compressedSize;
		file->location = (usize)finalBuf - (usize)e->fileData;
	} else {
		//we've got an error
		//... but no way to handle it
	}
	free(compressed);
	//You're also free to free the input data too
}

void wSarSortFiles(wSarFile* array, isize count)
{
	for(isize i = 1; i < count; ++i) {
		isize j = i - 1;
		if(array[j].id.hash > array[i].id.hash) {
			wSarFile temp = array[i];
			while((j >= 0) && (array[j].id.hash > temp.id.hash)) {
				array[j + 1] = array[j];
				j--;
			}
			array[j + 1] = temp;
		}
	}
}

void* wSarFinalizeArchive(wSarEditingArchive* e, isize* outSize)
{
	//at this point:
	// 		all files we want are in the archive
	// 		all the data we want is in the data array
	// 		the header is set up correctly
	//here's what we need to do:
	//		sort all files by hash
	//		contiguify the three arenas into one
	//		relocate file->location based on new base position 
	//			right now file->location points to the start of the file
	//			from e->fileData.
	//			We need it to point to the start of the file from the
	//			archive's base (from 0)
	//		....write it to file??? (not here)
	wSarSortFiles(e->fileTable, e->fileCount);

	usize fileTableSize = sizeof(wSarFile) * e->fileCount;
	wSarFile* newFileTable = wArenaPush(e->finalAlloc, fileTableSize);
	memcpy(newFileTable, e->fileTable, fileTableSize);
	//A reminder that (usize)e->header represents the start of the file
	e->header->fileTableLocation = (usize)newFileTable - (usize)e->header;

	usize fileDataSize = (usize)e->dataAlloc->head - (usize)e->fileData;
	void* newDataBuf = wArenaPush(e->finalAlloc, fileDataSize);
	memcpy(newDataBuf, e->fileData, fileDataSize);

	for(usize i = 0; i < e->fileCount; ++i) {
		wSarFile* f = newFileTable + i;
		f->location += (usize)newDataBuf - (usize)e->header;
	}

	isize size = (isize)e->finalAlloc->head - (isize)e->header;
	if(outSize) *outSize = size;

	e->header->magic = wSar_Magic;
	e->header->version = wSar_Version;
	return e->header;
}
#endif

u8* loadFile(char* filename, isize* size_out)
{
	char* str = NULL;
	FILE* fp = fopen(filename, "rb");
	if(fp != NULL) {
		fseek(fp, 0L, SEEK_END);
		isize size = ftell(fp);
		rewind(fp);
		str = malloc(size + 1);
		fread(str, sizeof(char), size, fp);
		str[size] = '\0';
		fclose(fp);
		if(size_out != NULL) {
			*size_out = size;
		}
	} else {
		fprintf(stderr, ">>> Could not open file %s\n", filename);
	}
	return str;
}

void recursivelyAddToArchive(wSarEditingArchive* e, string path)
{
	tinydir_dir dir;
	printf("Folder %s\n", path);
	if(tinydir_open(&dir, path) == -1) {
		fprintf(stderr, "Error: couldn't open directory %s. Skipping...\n", path);
		return;
	}

	while(dir.has_next) {
		tinydir_file file;
		if(tinydir_readfile(&dir, &file)) {
			fprintf(stderr, "Error: couldn't open file %s. Skipping...\n", file.path);
		}

		if(file.is_dir) {
			if(file.name[0] != '.') {
				recursivelyAddToArchive(e, file.path);
			}
			goto iterContinue;
		}
		else if(!file.is_reg) goto iterContinue;

		printf("| Adding %s\n", file.name);
		isize size = 0;
		u8* fileData = loadFile(file.path, &size);
		wSarAddFile(e, file.name, fileData, size);
		free(fileData);

iterContinue:
		if(tinydir_next(&dir) == -1) {
			return;
		}
	}
}



int main(int argc, char** argv)
{
	wMemoryInfo meminfo = wGetMemoryInfo();
	wMemoryArena* arena = wArenaBootstrap(meminfo, 0);
	printf("s-archive tool, for wpl\n");

	if(argc < 3) {
		printf("Warning: archive and command not specified\n"
				"Usage: sar archive.sar [extract|x, compress|add|c|a, print|p] ...files...\n");
		return 0;
	}

	i32 mode = 0;
	char c = argv[2][0];
	if(c == 'x' || c == 'e') {
		mode = 1;
	} else if(c == 'c' || c == 'a') {
		mode = 2;
	} else if(c == 'p') {
		mode = 3;
	}

	if(mode == 0) {
		printf("Error: unknown command\n");
		return 0;
	}
	

	if(mode == 1) {
		printf("Extracting %s...\n", argv[1]);
		tinydir_file file;
		wSarArchive* archive = NULL;

		if(tinydir_file_open(&file, argv[1]) != -1) {
			if(!file.is_dir && file.is_reg) {
				isize size = 0;
				void* data = loadFile(file.path, &size);
				printf("Loaded %d bytes\n", size);
				archive = wSarLoad(data, arena);
			} else {
				fprintf(stderr, "Error: Cannot open archive %s\n", argv[1]);
				return 0;
			}
		} else {
			fprintf(stderr, "Error: Cannot open archive %s\n", argv[1]);
			return 0;
		}

		string dirpath = ".";
		if(argc >= 4 && tinydir_file_open(&file, argv[3]) != -1) {
			//if we provide dir for the 3rd arg, extract into that.
			if(file.is_dir) dirpath = argv[3];
		}

		wSarFile* fileTable = (void*)((usize)
				archive->header + archive->header->fileTableLocation);
		for(usize i = 0; i < archive->header->fileCount; ++i) {
			wSarFile* file = fileTable + i;
			void* compressedData = (void*)((usize)archive->header + file->location);
			usize outputSize = 0;
			void* data = tinfl_decompress_mem_to_heap(
					compressedData, file->compressedSize, 
					&outputSize, 0);
			if(outputSize != file->fullSize) {
				fprintf(stderr, "Warning: %s uncompressed size discrepancy:\n"
						"Got: %zu | Expected %zu\n", 
						file->id.name, outputSize, file->fullSize);
			}

			char filename[1024];
			snprintf(filename, 1024, "%s/%s", dirpath, file->id.name);
			FILE* output = fopen(filename, "wb");
			if(output) {
				usize bytesWritten = fwrite(data, 1, outputSize, output);
				if(bytesWritten < outputSize) {
					fprintf(stderr, "Error: %s file writing failed!\n"
							"Incomplete file written to disk\n",
							filename);
				}
				fclose(output);
				
			} else {
				fprintf(stderr, "Error: couldn't open %s for writing\n", filename);
			}

		}

	}

	if(mode == 2) {
		tinydir_file file;
		wSarArchive* archive = NULL;

		if(tinydir_file_open(&file, argv[1]) != -1) {
			if(!file.is_dir && file.is_reg) {
				isize size = 0;
				void* data = loadFile(file.path, &size);
				printf("Loaded %d bytes\n", size);
				archive = wSarLoad(data, arena);
			} else {
				fprintf(stderr, "Error: Cannot open archive %s\n", argv[1]);
				return 0;
			}
		} else {
			printf("Creating archive %s...\n", argv[1]);
		}

		wSarEditingArchive* e = wSarCreateEditingArchive(archive);
		for(isize i = 3; i < argc; ++i) {
			if(tinydir_file_open(&file, argv[i]) == -1) {
				fprintf(stderr, "Error: couldn't open file %s. Skipping...\n", argv[i]);
				continue;
			}

			if(file.is_dir) {
				recursivelyAddToArchive(e, file.path);
				continue;
			}
			if(!file.is_reg) continue;

			printf("| Adding %s\n", file.name);
			isize size = 0;
			u8* fileData = loadFile(file.path, &size);
			wSarAddFile(e, file.name, fileData, size);
			free(fileData);
		}
		{
			isize  size = 0;
			void* data = wSarFinalizeArchive(e, &size);
			
			FILE* output = fopen(argv[1], "wb");
			if(output) {
				isize bytesWritten = fwrite(data, 1, size, output);
				if(bytesWritten < size) {
					fprintf(stderr, "Error: archive writing failed!\n"
							"Incomplete archive written to disk\n");
				}
				fclose(output);
			} else {
				fprintf(stderr, 
						"Error: Can't open final archive %s for writing\n",
						argv[1]);
			}
			printf("%d|%dk bytes written\n", size, size >> 10);
		}


		return;
	}


	printf("Archive: %s\n", argv[1]);
	printf("Mode: %s\n", mode == 1 ? "Extract" : "Compress");
	for(isize i = 0; i < (argc-3); ++i) {
		printf("\tFile: %s\n", argv[i+3]);
	}

	return 0;
}


