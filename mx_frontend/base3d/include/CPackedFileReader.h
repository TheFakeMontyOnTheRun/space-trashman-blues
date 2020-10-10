#ifndef FILEREADER_H
#define FILEREADER_H

void initFileReader(const char *__restrict__ dataFilePath);

size_t sizeOfFile(const char *__restrict__ path);

uint8_t *loadBinaryFileFromPath(const char *__restrict__ path);

FILE *openBinaryFileFromPath(const char *__restrict__ path);

#endif
