#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include <stdio.h>
#include <MacWindows.h>
#include <Dialogs.h>
#include <Menus.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Files.h>
#include <Processes.h>
#include <Errors.h>
#include <Script.h>	
#include <path2fss.h>
	
#include "Common.h"

#define kDataPath_MaxLength 256

char mDataPath[kDataPath_MaxLength];

FILE *OpenData(char* filename);
uint8_t* readFully(char *filename);

FILE *OpenData(char* filename) {
	ProcessSerialNumber psn = {0, kCurrentProcess };
	ProcessInfoRec info = {0};
	FSSpec spec;
	Str255 name;
	OSErr err;
	short 	refNum;
	static unsigned char kDataName[] = "base.pfs";
	Handle handle;
		
	info.processInfoLength = sizeof(info);
	info.processAppSpec = &spec;
	info.processName = name;
	

	err =	GetProcessInformation(&psn, &info);

	
	if (!err) {
	
			
		short oldVol ;
		short aVol ;
		long aDir , aProc ;

		FILE * ret = NULL ;
		
	    if ( GetVol ( NULL , & oldVol ) ) {
		    puts("Failed during GetVol");
    	    return NULL ;
	    }
	    
    	if ( GetWDInfo ( oldVol , & aVol , & aDir , & aProc ) ) {
		    puts("Failed during GetWDInfo");    	
        	return NULL  ;
	    }
    
    	if ( HSetVol ( NULL , spec.vRefNum , spec.parID ) ) {
			puts("Failed during HSetVol");
        	return NULL ;
	    }

    	ret = fopen ( filename , "rb" ) ;
   
	    if ( HSetVol ( NULL, aVol , aDir ) ) {
	       puts("an error we can't currently handle");
    	}

	    if ( SetVol ( NULL, oldVol ) ) {
			puts("an error we can't currently handle");
	    }
	    

    	return ret ;		

	}
		
	return NULL;
}

uint8_t* readFully(char *filename) {
	FILE *filePtr = OpenData(filename);
	uint8_t *resultBuffer;
	size_t size;
	
	assert(filePtr);
	
	fseek( filePtr, 0, SEEK_END);
	size = ftell(filePtr);
	
	assert(size);
	
	resultBuffer = (uint8_t*)calloc(1, size);
	assert(resultBuffer);
	
	rewind( filePtr );
	
	assert(fread( resultBuffer, size, 1, filePtr ));
	
	fclose( filePtr );
	
	
	return resultBuffer; 
}


void initFileReader(const char * __restrict__ dataFilePath) {
	sprintf (mDataPath, "%s", dataFilePath);
}

size_t sizeOfFile(const char * __restrict__ path) {

    FILE *mDataPack = OpenData(mDataPath);

	char buffer[85];
	int c;
	uint32_t size = 0;
	uint32_t offset = 0;
	uint16_t entries = 0;
	assert (fread(&entries, 2, 1, mDataPack));

	for (c = 0; c < entries; ++c) {
		uint8_t stringSize = 0;

		assert (fread(&offset, 4, 1, mDataPack));
		offset = toNativeEndianess(offset);
		assert (fread(&stringSize, 1, 1, mDataPack));
		assert (fread(&buffer, stringSize + 1, 1, mDataPack));

		if (!strcmp(buffer, path)) {
			goto found;
		}
	}

	found:
	if (offset == 0) {
		printf("failed to load %s\n", path);
		exit(-1);
	}

	fseek(mDataPack, offset, SEEK_SET);
	assert (fread(&size, 4, 1, mDataPack));
	size = toNativeEndianess(size);
	fclose(mDataPack);

	return size;
}

struct StaticBuffer loadBinaryFileFromPath(const char * __restrict__ path) {

    FILE *mDataPack = OpenData(mDataPath);

    size_t offset = 0;
	uint16_t entries = 0;
	char buffer[85];
	int c;
	uint32_t size = 0;
    struct StaticBuffer toReturn;
	
    if (!mDataPack) {
    	puts("Failed to read file");
    	puts(path);
    	exit(0);
    }	

	assert (fread(&entries, 2, 1, mDataPack));

	for (c = 0; c < entries; ++c) {
		uint8_t stringSize = 0;

		assert (fread(&offset, 4, 1, mDataPack));
		offset = toNativeEndianess(offset);
		assert (fread(&stringSize, 1, 1, mDataPack));
		assert (fread(&buffer, stringSize + 1, 1, mDataPack));

		if (!strcmp(buffer, path)) {
			goto found;
		}
	}


    found:

    if (offset == 0) {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    fseek(mDataPack, offset, SEEK_SET);

    assert (fread(&size, 4, 1, mDataPack));
    size = toNativeEndianess(size);
    toReturn.size = size;
    toReturn.data = (uint8_t *) malloc(size);

    assert (fread(toReturn.data, sizeof(uint8_t), size, mDataPack));
    fclose(mDataPack);

    return toReturn;
}

FILE *openBinaryFileFromPath(const char * __restrict__ path) {

    FILE *mDataPack = OpenData(mDataPath);

	uint32_t offset = 0;
	uint16_t entries = 0;
	char buffer[85];
	int c;
	uint32_t size = 0;
	
	if (!mDataPack) {
    	puts("Failed to read file");
    	exit(0);
    }
	

	assert (fread(&entries, 2, 1, mDataPack));

	for (c = 0; c < entries; ++c) {
		uint8_t stringSize = 0;

		assert (fread(&offset, 4, 1, mDataPack));
		offset = toNativeEndianess(offset);

		assert (fread(&stringSize, 1, 1, mDataPack));
		assert (fread(&buffer, stringSize + 1, 1, mDataPack));

		if (!strcmp(buffer, path)) {
			goto found;
		}
	}

	return NULL;

	found:

	if (offset == 0) {
		printf("failed to load %s\n", path);
		exit(-1);
	}

	fseek(mDataPack, offset, SEEK_SET);
	assert (fread(&size, 4, 1, mDataPack));
	size = toNativeEndianess(size);

	return mDataPack;
}
