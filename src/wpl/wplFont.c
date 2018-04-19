wFontInfo* wLoadFontInfo(wWindow* window, char* filename, wMemoryArena* arena)
{
	char buf[2048];
	snprintf(buf, 2048, "%s%s", window->basePath, filename);

	wFontInfo* fi = NULL;
	//wLoadLocalSizedFile(...)
	return fi;
}

