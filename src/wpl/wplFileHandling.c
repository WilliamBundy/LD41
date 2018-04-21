/* wHotFile.h
 *
 * This is a debug-only lib for hot-reloading files.
 * It uses malloc which makes it not-really-safe to 
 * use in a shipping version (on windows at least)
 *
 * Usage:
 * 		wHotFile* file = wCreateHotFile(window, "basic.shader");
 * 		if(wUpdateHotFile(file)) {
 * 			//contents discarded, simply recreate shader now
 * 			...
 * 		}
 * 		wDestroyHotFile(file);
 */


wHotFile* wCreateHotFile(wWindow* window, string filename)
{
	wHotFile* file = malloc(sizeof(wHotFile));
	file->zero = '\0';
	file->filenameLength = snprintf(file->filename, 512, "%s%s", 
			window->basePath,
			filename);
	file->handle = wGetFileHandle(file->filename);
	file->lastTime = wGetFileModifiedTime(file->handle);
	file->size = wGetFileSize(file->handle);
	file->data = malloc(file->size + 1);
	wLoadSizedFile(file->filename, file->data, file->size + 1);
	return file;
}

void wDestroyHotFile(wHotFile* file)
{
	free(file->data);
	wCloseFileHandle(file->handle);
	free(file);
}

i32 wCheckHotFile(wHotFile* file)
{
	return wGetFileModifiedTime(file->handle) != file->lastTime;
}

i32 wUpdateHotFile(wHotFile* file)
{
	if(wCheckHotFile(file)) {
		free(file->data);
		file->lastTime = wGetFileModifiedTime(file->handle);
		file->size = wGetFileSize(file->handle);
		file->data = malloc(file->size + 1);
		wLoadSizedFile(file->filename, file->data, file->size + 1);
		return 1;
	}
	return 0;
}
