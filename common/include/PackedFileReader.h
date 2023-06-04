#ifndef FILEREADER_H
#define FILEREADER_H

#ifndef LEAN_BUILD
void initFileReader(const char * dataFilePath);
struct StaticBuffer loadBinaryFileFromPath(const char * path);
#else
struct StaticBuffer loadBinaryFileFromPath(const uint8_t slot);
#endif

void disposeDiskBuffer(struct StaticBuffer buffer);
#endif
