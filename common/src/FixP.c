#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>

#endif

#ifndef SMD

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef WIN32

#include <unistd.h>

#endif
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif

#include "Common.h"


