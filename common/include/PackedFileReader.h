#ifndef FILEREADER_H
#define FILEREADER_H

#ifndef LEAN_BUILD
void initFileReader(const char *__restrict__ dataFilePath);

size_t sizeOfFile(const char *__restrict__ path);

struct StaticBuffer loadBinaryFileFromPath(const char *__restrict__ path);

FILE *openBinaryFileFromPath(const char *__restrict__ path);

#else

struct StaticBuffer loadBinaryFileFromPath(const uint8_t slot);

#endif

#endif
