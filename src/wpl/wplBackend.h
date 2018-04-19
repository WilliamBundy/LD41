
i64 wCreateWindow(wWindowDef* def, wWindow* window);
void wShowWindow(wWindow* window);
i64 wUpdate(wWindow* window, wState* state);
i64 wRender(wWindow* window);
void wQuit();

u8* wLoadFile(string filename, isize* sizeOut, wMemoryArena* alloc);
isize wLoadSizedFile(string filename, u8* buffer, isize bufferSize);
u8* wLoadLocalFile(wWindow* window, string filename, isize* sizeOut, wMemoryArena* arena);
isize wLoadLocalSizedFile(
		wWindow* window, string filename,
		u8* buffer, isize bufferSize);

typedef void* wFileHandle;
wFileHandle wGetFileHandle(string filename);
wFileHandle wGetLocalFileHandle(wWindow* window, string filename);;
isize wGetFileSize(wFileHandle file);
isize wGetFileModifiedTime(wFileHandle file);

