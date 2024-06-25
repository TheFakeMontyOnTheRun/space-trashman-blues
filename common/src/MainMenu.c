#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "SoundSystem.h"
#include "Engine.h"

#ifndef MONOCHROME_VECTORS
#include "FixP.h"
#include "Vec.h"
#include "Mesh.h"
#include "CActor.h"
#include "Renderer.h"
#endif

#ifdef EMIT_QUIT_OPTION

#define kMainMenuOptionsCount  4

const char *MainMenu_options[kMainMenuOptionsCount] = {
        "Play game", "Credits", "Help", "Quit"};

const enum EGameMenuState MainMenu_nextStateNavigation[kMainMenuOptionsCount] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

#else

#define kMainMenuOptionsCount 3

const char *MainMenu_options[kMainMenuOptionsCount] = {
        "Play game", "Credits", "Help"};

const enum EGameMenuState MainMenu_nextStateNavigation[kMainMenuOptionsCount] = {
        kPlayGame, kCredits, kHelp};

#endif

#ifndef MONOCHROME_VECTORS
static const uint8_t logoGraphics[] = {
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
2, 11,
0, 11,
0, 9,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
2, 11,
0, 9,
2, 9,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
2, 7,
0, 7,
0, 3,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
2, 7,
0, 3,
2, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 11,
2, 11,
2, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 11,
2, 0,
6, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
9, 11,
6, 11,
6, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
9, 11,
6, 0,
9, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
9, 3,
6, 3,
6, 5,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
9, 3,
6, 5,
9, 5,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 10,
9, 11,
9, 9,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 10,
9, 9,
11, 9,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 10,
11, 10,
11, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 10,
11, 0,
13, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 10,
13, 10,
13, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 10,
13, 0,
15, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 11,
13, 11,
13, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 11,
13, 11,
15, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 10,
15, 10,
15, 3,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 10,
15, 3,
17, 3,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 11,
15, 11,
15, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 11,
15, 11,
17, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 10,
17, 10,
17, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 10,
17, 0,
19, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 11,
17, 11,
17, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 11,
17, 11,
19, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 10,
19, 10,
19, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 10,
19, 0,
21, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 9,
19, 11,
19, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 9,
19, 11,
21, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 9,
21, 10,
21, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 9,
21, 11,
23, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 9,
23, 9,
23, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 9,
23, 11,
29, 11,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 11,
23, 11,
23, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 11,
23, 0,
29, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 9,
29, 9,
29, 0,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 9,
29, 0,
31, 3,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 4,
29, 6,
29, 6,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 4,
29, 6,
31, 8,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
3, 29,
0, 29,
0, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
3, 29,
0, 19,
3, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
4, 29,
3, 29,
3, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
4, 29,
3, 19,
4, 20,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
5, 27,
4, 27,
4, 20,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
5, 27,
4, 20,
5, 21,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 27,
5, 27,
5, 21,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 27,
5, 21,
6, 20,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
7, 29,
6, 29,
6, 20,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
7, 29,
6, 20,
7, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 29,
7, 29,
7, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 29,
7, 19,
11, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 29,
11, 29,
11, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 29,
11, 29,
13, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 29,
13, 29,
13, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 29,
13, 29,
15, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 29,
13, 29,
13, 21,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
15, 29,
13, 21,
15, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 28,
15, 29,
15, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 28,
15, 29,
17, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 29,
15, 29,
15, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 29,
15, 19,
17, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 28,
17, 28,
17, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 28,
17, 19,
19, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 27,
17, 27,
17, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 27,
17, 27,
19, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 28,
19, 28,
19, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 28,
19, 19,
21, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 29,
19, 29,
19, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
21, 29,
19, 27,
21, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 28,
21, 28,
21, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 28,
21, 19,
23, 21,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 29,
21, 29,
21, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 29,
21, 28,
23, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 28,
23, 28,
23, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 28,
23, 28,
25, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 28,
25, 28,
25, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 28,
25, 28,
29, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 29,
25, 29,
25, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 29,
25, 19,
29, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 28,
29, 28,
29, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 28,
29, 19,
31, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 27,
29, 27,
29, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
31, 27,
29, 27,
31, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
33, 28,
31, 28,
31, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
33, 28,
31, 19,
33, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
33, 29,
31, 29,
31, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
33, 29,
31, 27,
33, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 27,
33, 28,
33, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 27,
33, 19,
35, 21,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 23,
33, 25,
33, 25,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 23,
33, 25,
35, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 29,
33, 29,
33, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 29,
33, 27,
35, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 27,
35, 27,
35, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 27,
35, 28,
37, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 27,
37, 27,
37, 28,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 27,
37, 28,
44, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 29,
37, 29,
37, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 29,
37, 19,
44, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 27,
44, 27,
44, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 27,
44, 29,
46, 29,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 29,
44, 29,
44, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 29,
44, 19,
46, 19,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 21,
44, 21,
44, 23,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 21,
44, 23,
46, 23,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 25,
44, 25,
44, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 25,
44, 27,
46, 27,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
4, 47,
0, 47,
0, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
4, 47,
0, 37,
4, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 47,
4, 47,
4, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
6, 47,
4, 37,
6, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
10, 47,
6, 47,
6, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
10, 47,
6, 39,
10, 43,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
10, 47,
6, 47,
6, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
10, 47,
6, 37,
10, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 47,
10, 47,
10, 43,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 47,
10, 43,
11, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 47,
10, 47,
10, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
11, 47,
10, 37,
11, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
12, 47,
11, 47,
11, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
12, 47,
11, 45,
12, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
12, 45,
11, 45,
11, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
12, 45,
11, 38,
12, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 47,
12, 47,
12, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 47,
12, 46,
13, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 45,
12, 45,
12, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
13, 45,
12, 39,
13, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
14, 47,
13, 47,
13, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
14, 47,
13, 38,
14, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 47,
14, 47,
14, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
17, 47,
14, 37,
17, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 47,
17, 47,
17, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
19, 47,
17, 47,
19, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 47,
19, 47,
19, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 47,
19, 47,
23, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 47,
19, 47,
19, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
23, 47,
19, 37,
23, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 47,
23, 47,
23, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 47,
23, 37,
25, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 45,
23, 45,
23, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
25, 45,
23, 45,
25, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
27, 47,
25, 47,
25, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
27, 47,
25, 37,
27, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
27, 43,
25, 45,
25, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
27, 43,
25, 45,
27, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 47,
27, 47,
27, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
29, 47,
27, 46,
29, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 47,
29, 47,
29, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 47,
29, 46,
35, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 47,
29, 47,
29, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
35, 47,
29, 37,
35, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 47,
35, 47,
35, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 47,
35, 47,
37, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 47,
35, 47,
35, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 47,
35, 37,
37, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 39,
35, 39,
35, 41,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 39,
35, 41,
37, 41,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 43,
35, 43,
35, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
37, 43,
35, 45,
37, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
40, 47,
37, 47,
37, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
40, 47,
37, 45,
40, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 47,
40, 47,
40, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 47,
40, 46,
44, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 47,
40, 47,
40, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
44, 47,
40, 37,
44, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 47,
44, 47,
44, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 47,
44, 37,
46, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 45,
44, 45,
44, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
46, 45,
44, 45,
46, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
48, 47,
46, 47,
46, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
48, 47,
46, 37,
48, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
48, 47,
46, 47,
46, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
48, 47,
46, 45,
48, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 47,
48, 47,
48, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 47,
48, 37,
50, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 41,
48, 43,
48, 43,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 41,
48, 43,
50, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 47,
48, 47,
48, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
50, 47,
48, 46,
50, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
52, 47,
50, 47,
50, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
52, 47,
50, 46,
52, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
56, 47,
52, 47,
52, 46,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
56, 47,
52, 46,
56, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
56, 47,
52, 47,
52, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
56, 47,
52, 37,
56, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
58, 47,
56, 47,
56, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
58, 47,
56, 37,
58, 45,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
60, 47,
58, 47,
58, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
60, 47,
58, 37,
60, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
62, 47,
60, 47,
60, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
62, 47,
60, 37,
62, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
62, 47,
60, 47,
60, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
62, 47,
60, 47,
62, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
64, 47,
62, 47,
62, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
64, 47,
62, 39,
64, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
64, 47,
62, 47,
62, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
64, 47,
62, 47,
64, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
66, 47,
64, 47,
64, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
66, 47,
64, 37,
66, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
66, 47,
64, 47,
64, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
66, 47,
64, 47,
66, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
68, 47,
66, 47,
66, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
68, 47,
66, 37,
68, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
68, 45,
66, 47,
66, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
68, 45,
66, 47,
68, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
71, 47,
68, 47,
68, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
71, 47,
68, 47,
71, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
74, 47,
71, 47,
71, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
74, 47,
71, 47,
74, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
74, 47,
71, 47,
71, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
74, 47,
71, 37,
74, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
75, 47,
74, 47,
74, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
75, 47,
74, 47,
75, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
75, 47,
74, 47,
74, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
75, 47,
74, 37,
75, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
76, 47,
75, 47,
75, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
76, 47,
75, 47,
76, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
76, 45,
75, 45,
75, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
76, 45,
75, 38,
76, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
77, 47,
76, 47,
76, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
77, 47,
76, 47,
77, 47,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
77, 45,
76, 45,
76, 39,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
77, 45,
76, 39,
77, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
78, 47,
77, 47,
77, 38,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
78, 47,
77, 38,
78, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
81, 47,
78, 47,
78, 37,
3,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
81, 47,
78, 37,
81, 37,
0};

#else
static const uint8_t logoGraphics[] = {
        33,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        6, 11,
        0, 11,
        0, 9,
        2, 9,
        2, 7,
        0, 7,
        0, 3,
        2, 0,
        9, 0,
        9, 3,
        6, 3,
        6, 5,
        9, 5,
        9, 9,
        11, 9,
        11, 0,
        15, 0,
        15, 3,
        17, 3,
        17, 0,
        21, 0,
        21, 9,
        19, 11,
        13, 11,
        29, 11,
        23, 11,
        23, 0,
        29, 0,
        31, 3,
        31, 4,
        29, 6,
        31, 8,
        31, 9,
        45,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        6, 29,
        6, 27,
        4, 27,
        4, 29,
        0, 29,
        0, 19,
        3, 19,
        5, 21,
        7, 19,
        11, 19,
        11, 29,
        17, 29,
        13, 29,
        13, 21,
        15, 19,
        21, 19,
        23, 21,
        23, 29,
        19, 29,
        19, 27,
        17, 27,
        29, 29,
        25, 29,
        25, 19,
        33, 19,
        35, 21,
        35, 23,
        33, 25,
        35, 27,
        35, 29,
        31, 29,
        31, 27,
        29, 27,
        46, 29,
        37, 29,
        37, 19,
        46, 19,
        46, 21,
        44, 21,
        44, 23,
        46, 23,
        46, 25,
        44, 25,
        44, 27,
        46, 27,
        72,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        4, 47,
        0, 47,
        0, 37,
        4, 37,
        13, 47,
        13, 45,
        11, 45,
        11, 47,
        6, 47,
        6, 37,
        10, 37,
        12, 39,
        14, 37,
        17, 37,
        17, 47,
        23, 47,
        19, 47,
        19, 37,
        25, 37,
        27, 39,
        27, 43,
        25, 45,
        23, 45,
        37, 47,
        29, 47,
        29, 37,
        37, 37,
        37, 39,
        35, 39,
        35, 41,
        37, 41,
        37, 43,
        35, 43,
        35, 45,
        37, 45,
        44, 47,
        40, 47,
        40, 37,
        48, 37,
        50, 39,
        50, 41,
        48, 43,
        50, 45,
        50, 47,
        46, 47,
        46, 45,
        44, 45,
        56, 47,
        52, 47,
        52, 37,
        56, 37,
        58, 45,
        58, 37,
        62, 37,
        62, 39,
        64, 39,
        64, 37,
        68, 37,
        68, 45,
        66, 47,
        60, 47,
        77, 47,
        77, 45,
        75, 45,
        75, 47,
        71, 47,
        71, 37,
        74, 37,
        76, 39,
        78, 37,
        81, 37,
        81, 47,
        0};



#endif



static const uint8_t splashGraphics[] = {
#ifdef MONOCHROME_VECTORS
        5,
#ifndef MONOCHROME_VECTORS
68, 85, 0,
#endif
40, 50,
41, 60,
70, 59,
69, 52,
40, 50,
5,
#ifndef MONOCHROME_VECTORS
80, 68, 22,
#endif
54, 33,
54, 51,
66, 55,
72, 31,
54, 33,
6,
#ifndef MONOCHROME_VECTORS
136, 170, 0,
#endif
54, 57,
81, 58,
81, 124,
32, 123,
42, 57,
42, 57,
4,
#ifndef MONOCHROME_VECTORS
68, 120, 33,
#endif
42, 57,
39, 75,
71, 58,
42, 57,
7,
#ifndef MONOCHROME_VECTORS
85, 34, 0,
#endif
45, 53,
61, 53,
59, 47,
59, 36,
43, 32,
42, 54,
45, 53,
7,
#ifndef MONOCHROME_VECTORS
183, 200, 183,
#endif
46, 35,
63, 37,
81, 38,
87, 35,
87, 21,
49, 18,
46, 35,
5,
#ifndef MONOCHROME_VECTORS
108, 103, 83,
#endif
49, 18,
46, 35,
72, 38,
73, 22,
49, 18,
6,
#ifndef MONOCHROME_VECTORS
138, 145, 111,
#endif
48, 18,
73, 22,
87, 21,
87, 1,
77, 1,
52, 15,
7,
#ifndef MONOCHROME_VECTORS
196, 200, 183,
#endif
27, 98,
27, 108,
25, 106,
1, 108,
1, 1,
68, 1,
43, 15,
6,
#ifndef MONOCHROME_VECTORS
108, 103, 83,
#endif
60, 57,
63, 37,
81, 36,
81, 59,
63, 60,
60, 57,
5,
#ifndef MONOCHROME_VECTORS
145, 124, 111,
#endif
26, 108,
36, 125,
43, 123,
27, 98,
25, 105,
5,
#ifndef MONOCHROME_VECTORS
188, 211, 95,
#endif
27, 98,
43, 98,
56, 124,
42, 123,
27, 98,
4,
#ifndef MONOCHROME_VECTORS
205, 222, 135,
#endif
25, 106,
36, 125,
1, 125,
1, 108,
5,
#ifndef MONOCHROME_VECTORS
85, 68, 0,
#endif
46, 47,
59, 47,
61, 53,
45, 53,
46, 47,
4,
#ifndef MONOCHROME_VECTORS
147, 172, 147,
#endif
59, 39,
87, 41,
87, 36,
59, 39,
8,
#ifndef MONOCHROME_VECTORS
170, 212, 0,
#endif
59, 94,
56, 90,
56, 90,
40, 104,
40, 104,
40, 104,
59, 94,
59, 94,
7,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
27, 98,
36, 98,
49, 18,
77, 1,
68, 1,
43, 15,
27, 98,
5,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
1, 1,
1, 7,
1, 21,
43, 17,
68, 1,
3,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
43, 17,
45, 1,
68, 1,
4,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
49, 18,
43, 17,
68, 1,
78, 1,
4,
#ifndef MONOCHROME_VECTORS
108, 93, 83,
#endif
43, 17,
1, 9,
1, 1,
46, 1,
6,
#ifndef MONOCHROME_VECTORS
85, 0, 0,
#endif
46, 91,
34, 108,
29, 125,
87, 125,
87, 86,
60, 84,
5,
#ifndef MONOCHROME_VECTORS
43, 0, 0,
#endif
50, 103,
74, 107,
87, 107,
87, 125,
46, 125,
7,
#ifndef MONOCHROME_VECTORS
170, 0, 0,
#endif
71, 33,
55, 36,
51, 57,
51, 86,
62, 98,
87, 92,
87, 32,
6,
#ifndef MONOCHROME_VECTORS
180, 0, 0,
#endif
56, 37,
69, 40,
87, 38,
87, 33,
71, 33,
56, 37,
5,
#ifndef MONOCHROME_VECTORS
190, 0, 0,
#endif
52, 56,
59, 67,
60, 47,
56, 37,
52, 56,
5,
#ifndef MONOCHROME_VECTORS
150, 0, 0,
#endif
74, 69,
78, 49,
87, 41,
86, 64,
74, 69,
4,
#ifndef MONOCHROME_VECTORS
170, 0, 0,
#endif
57, 93,
54, 89,
46, 92,
38, 104,
5,
#ifndef MONOCHROME_VECTORS
0, 0, 0,
#endif
54, 106,
54, 106,
52, 125,
52, 125,
87, 125,
5,
#ifndef MONOCHROME_VECTORS
128, 128, 0,
#endif
60, 125,
60, 125,
84, 125,
84, 125,
61, 125,
4,
#ifndef MONOCHROME_VECTORS
0, 0, 255,
#endif
64, 125,
64, 125,
70, 125,
70, 125,
3,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
62, 95,
58, 67,
53, 84,
3,
#ifndef MONOCHROME_VECTORS
0, 0, 111017082,
#endif
53, 84,
58, 67,
53, 59,
3,
#ifndef MONOCHROME_VECTORS
43, 0, 0,
#endif
49, 99,
87, 101,
87, 98,
3,
#ifndef MONOCHROME_VECTORS
92, 92, 0,
#endif
62, 95,
74, 69,
86, 88,
3,
#ifndef MONOCHROME_VECTORS
100, 100, 0,
#endif
58, 67,
62, 96,
74, 69,
3,
#ifndef MONOCHROME_VECTORS
85, 68, 0,
#endif
74, 69,
85, 61,
85, 88,
0};


#else
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        40, 50,
        41, 60,
        41, 51,
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        40, 50,
        41, 51,
        40, 50,
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        41, 60,
        69, 59,
        69, 52,
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        41, 60,
        69, 52,
        41, 51,
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        69, 59,
        70, 59,
        70, 59,
        3,
#ifndef MONOCHROME_VECTORS
        68, 85, 0,
#endif
        69, 59,
        70, 59,
        69, 52,
        3,
#ifndef MONOCHROME_VECTORS
        80, 68, 22,
#endif
        54, 51,
        66, 55,
        66, 32,
        3,
#ifndef MONOCHROME_VECTORS
        80, 68, 22,
#endif
        54, 51,
        66, 32,
        54, 33,
        3,
#ifndef MONOCHROME_VECTORS
        80, 68, 22,
#endif
        66, 55,
        72, 31,
        72, 31,
        3,
#ifndef MONOCHROME_VECTORS
        80, 68, 22,
#endif
        66, 55,
        72, 31,
        66, 32,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 124,
        32, 123,
        32, 123,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 124,
        32, 123,
        42, 57,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 124,
        42, 124,
        42, 57,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 124,
        42, 57,
        42, 57,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 57,
        54, 57,
        54, 124,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        42, 57,
        54, 124,
        42, 124,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        54, 57,
        81, 58,
        81, 124,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        54, 57,
        81, 124,
        54, 124,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        81, 58,
        81, 124,
        81, 124,
        3,
#ifndef MONOCHROME_VECTORS
        136, 170, 0,
#endif
        81, 58,
        81, 124,
        81, 124,
        3,
#ifndef MONOCHROME_VECTORS
        68, 120, 33,
#endif
        42, 57,
        39, 75,
        39, 75,
        3,
#ifndef MONOCHROME_VECTORS
        68, 120, 33,
#endif
        42, 57,
        39, 75,
        42, 73,
        3,
#ifndef MONOCHROME_VECTORS
        68, 120, 33,
#endif
        42, 73,
        71, 58,
        71, 58,
        3,
#ifndef MONOCHROME_VECTORS
        68, 120, 33,
#endif
        42, 73,
        71, 58,
        42, 57,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        43, 32,
        42, 54,
        42, 54,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        43, 32,
        42, 54,
        43, 54,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        45, 32,
        43, 32,
        43, 54,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        45, 32,
        43, 54,
        45, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        45, 53,
        59, 53,
        59, 36,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        45, 53,
        59, 36,
        45, 32,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 53,
        59, 53,
        59, 48,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 53,
        59, 48,
        59, 47,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 47,
        59, 36,
        59, 36,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 47,
        59, 36,
        59, 36,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 53,
        61, 53,
        61, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 34, 0,
#endif
        59, 53,
        61, 53,
        59, 48,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        46, 35,
        46, 35,
        46, 33,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        46, 35,
        46, 33,
        46, 35,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        46, 35,
        49, 35,
        49, 18,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        46, 35,
        49, 18,
        46, 33,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        49, 35,
        63, 37,
        63, 19,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        49, 35,
        63, 19,
        49, 18,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        63, 37,
        81, 38,
        81, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        63, 37,
        81, 21,
        63, 19,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        81, 38,
        87, 35,
        87, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        81, 38,
        87, 21,
        81, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        87, 35,
        87, 35,
        87, 35,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        87, 35,
        87, 35,
        87, 21,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        49, 18,
        46, 35,
        46, 35,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        49, 18,
        46, 35,
        49, 35,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        49, 35,
        72, 38,
        72, 22,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        49, 35,
        72, 22,
        49, 18,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        72, 38,
        73, 22,
        73, 22,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        72, 38,
        73, 22,
        72, 22,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        52, 15,
        48, 18,
        48, 18,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        52, 15,
        48, 18,
        52, 19,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        52, 19,
        73, 22,
        73, 4,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        52, 19,
        73, 4,
        52, 15,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        73, 22,
        77, 22,
        77, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        73, 22,
        77, 1,
        73, 4,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        77, 22,
        87, 21,
        87, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        77, 22,
        87, 1,
        77, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        87, 21,
        87, 1,
        87, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        87, 21,
        87, 1,
        87, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        1, 108,
        1, 108,
        1, 108,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        1, 108,
        1, 108,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        25, 106,
        1, 108,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        25, 106,
        1, 1,
        25, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        27, 108,
        25, 106,
        25, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        27, 108,
        25, 1,
        27, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        27, 98,
        27, 108,
        27, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        27, 98,
        27, 1,
        27, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        43, 15,
        27, 98,
        27, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        43, 15,
        27, 1,
        43, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        43, 1,
        68, 1,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        196, 200, 183,
#endif
        43, 1,
        68, 1,
        43, 15,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        60, 57,
        63, 39,
        63, 60,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        60, 57,
        63, 60,
        60, 57,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        63, 39,
        63, 37,
        63, 60,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        63, 39,
        63, 60,
        63, 60,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        63, 37,
        81, 36,
        81, 59,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        63, 37,
        81, 59,
        63, 60,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        81, 36,
        81, 36,
        81, 36,
        3,
#ifndef MONOCHROME_VECTORS
        108, 103, 83,
#endif
        81, 36,
        81, 36,
        81, 59,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        25, 105,
        26, 108,
        26, 99,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        25, 105,
        26, 99,
        25, 105,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        26, 108,
        27, 109,
        27, 98,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        26, 108,
        27, 98,
        26, 99,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        27, 109,
        36, 125,
        36, 112,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        27, 109,
        36, 112,
        27, 98,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        36, 125,
        43, 123,
        43, 123,
        3,
#ifndef MONOCHROME_VECTORS
        145, 124, 111,
#endif
        36, 125,
        43, 123,
        36, 112,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        27, 98,
        42, 98,
        42, 123,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        27, 98,
        42, 123,
        27, 98,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        42, 98,
        43, 98,
        43, 123,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        42, 98,
        43, 123,
        42, 123,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        43, 98,
        56, 124,
        56, 124,
        3,
#ifndef MONOCHROME_VECTORS
        188, 211, 95,
#endif
        43, 98,
        56, 124,
        43, 123,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        1, 125,
        1, 125,
        1, 125,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        1, 125,
        1, 125,
        1, 108,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        1, 108,
        25, 106,
        25, 125,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        1, 108,
        25, 125,
        1, 125,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        25, 106,
        36, 125,
        36, 125,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        25, 106,
        36, 125,
        25, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        46, 53,
        45, 53,
        45, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        46, 53,
        45, 53,
        46, 47,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        46, 47,
        59, 47,
        59, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        46, 47,
        59, 53,
        46, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        59, 47,
        61, 53,
        61, 53,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        59, 47,
        61, 53,
        59, 53,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        59, 39,
        87, 41,
        87, 36,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        59, 39,
        87, 36,
        59, 39,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        87, 41,
        87, 41,
        87, 41,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        87, 41,
        87, 41,
        87, 36,
        3,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        56, 90,
        40, 104,
        40, 104,
        3,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        56, 90,
        40, 104,
        56, 96,
        3,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        59, 94,
        56, 90,
        56, 96,
        3,
#ifndef MONOCHROME_VECTORS
        170, 212, 0,
#endif
        59, 94,
        56, 96,
        59, 94,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        27, 98,
        36, 98,
        36, 53,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        27, 98,
        36, 53,
        27, 98,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        36, 98,
        43, 55,
        43, 15,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        36, 98,
        43, 15,
        36, 53,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        43, 55,
        49, 18,
        49, 12,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        43, 55,
        49, 12,
        43, 15,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        49, 18,
        68, 7,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        49, 18,
        68, 1,
        49, 12,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        68, 7,
        77, 1,
        77, 1,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        68, 7,
        77, 1,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 7,
        1, 7,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 7,
        1, 11,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 1,
        1, 11,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 11,
        1, 21,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        43, 1,
        1, 1,
        1, 21,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        43, 1,
        1, 21,
        43, 17,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        68, 1,
        43, 1,
        43, 17,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        68, 1,
        43, 17,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        45, 15,
        43, 17,
        43, 17,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        45, 15,
        43, 17,
        45, 1,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        68, 1,
        45, 15,
        45, 1,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        68, 1,
        45, 1,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        49, 18,
        43, 17,
        43, 17,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        49, 18,
        43, 17,
        49, 13,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        68, 7,
        49, 18,
        49, 13,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        68, 7,
        49, 13,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        78, 1,
        68, 7,
        68, 1,
        3,
#ifndef MONOCHROME_VECTORS
        172, 157, 147,
#endif
        78, 1,
        68, 1,
        78, 1,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        1, 9,
        1, 9,
        1, 9,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        1, 9,
        1, 9,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        43, 17,
        1, 9,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        43, 17,
        1, 1,
        43, 1,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        46, 1,
        43, 17,
        43, 1,
        3,
#ifndef MONOCHROME_VECTORS
        108, 93, 83,
#endif
        46, 1,
        43, 1,
        46, 1,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        34, 108,
        29, 125,
        29, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        34, 108,
        29, 125,
        34, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        46, 91,
        34, 108,
        34, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        46, 91,
        34, 125,
        46, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        60, 84,
        46, 91,
        46, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        60, 84,
        46, 125,
        60, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        60, 125,
        87, 125,
        87, 86,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        60, 125,
        87, 86,
        60, 84,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        87, 125,
        87, 125,
        87, 125,
        3,
#ifndef MONOCHROME_VECTORS
        85, 0, 0,
#endif
        87, 125,
        87, 125,
        87, 86,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        46, 125,
        50, 103,
        50, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        46, 125,
        50, 125,
        46, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        50, 103,
        74, 107,
        74, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        50, 103,
        74, 125,
        50, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        74, 107,
        87, 107,
        87, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        74, 107,
        87, 125,
        74, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 107,
        87, 125,
        87, 125,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 107,
        87, 125,
        87, 125,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        51, 57,
        51, 57,
        51, 57,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        51, 57,
        51, 57,
        51, 86,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        55, 36,
        51, 57,
        51, 86,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        55, 36,
        51, 86,
        55, 91,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        62, 35,
        55, 36,
        55, 91,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        62, 35,
        55, 91,
        62, 98,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        71, 33,
        62, 35,
        62, 98,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        71, 33,
        62, 98,
        71, 96,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        87, 32,
        71, 33,
        71, 96,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        87, 32,
        71, 96,
        87, 92,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        87, 92,
        87, 92,
        87, 92,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        87, 92,
        87, 92,
        87, 32,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        56, 37,
        69, 40,
        69, 34,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        56, 37,
        69, 34,
        56, 37,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        69, 40,
        71, 40,
        71, 33,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        69, 40,
        71, 33,
        69, 34,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        71, 40,
        87, 38,
        87, 33,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        71, 40,
        87, 33,
        71, 33,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        87, 38,
        87, 33,
        87, 33,
        3,
#ifndef MONOCHROME_VECTORS
        180, 0, 0,
#endif
        87, 38,
        87, 33,
        87, 33,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        52, 56,
        56, 63,
        56, 37,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        52, 56,
        56, 37,
        52, 56,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        56, 63,
        59, 67,
        59, 43,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        56, 63,
        59, 43,
        56, 37,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        59, 67,
        60, 47,
        60, 47,
        3,
#ifndef MONOCHROME_VECTORS
        190, 0, 0,
#endif
        59, 67,
        60, 47,
        59, 43,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        74, 69,
        78, 49,
        78, 68,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        74, 69,
        78, 68,
        74, 69,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        78, 49,
        86, 41,
        86, 64,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        78, 49,
        86, 64,
        78, 68,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        86, 41,
        87, 41,
        87, 41,
        3,
#ifndef MONOCHROME_VECTORS
        150, 0, 0,
#endif
        86, 41,
        87, 41,
        86, 64,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        38, 104,
        46, 99,
        46, 92,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        38, 104,
        46, 92,
        38, 104,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        46, 99,
        54, 95,
        54, 89,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        46, 99,
        54, 89,
        46, 92,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        54, 95,
        57, 93,
        57, 93,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        54, 95,
        57, 93,
        54, 89,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        54, 106,
        52, 125,
        52, 125,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        54, 106,
        52, 125,
        54, 125,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        87, 125,
        54, 106,
        54, 125,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        87, 125,
        54, 125,
        87, 125,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        61, 125,
        60, 125,
        60, 125,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        61, 125,
        60, 125,
        61, 125,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        61, 125,
        84, 125,
        84, 125,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        61, 125,
        84, 125,
        61, 125,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 255,
#endif
        70, 125,
        64, 125,
        64, 125,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 255,
#endif
        70, 125,
        64, 125,
        70, 125,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        53, 84,
        58, 90,
        58, 67,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        53, 84,
        58, 67,
        53, 84,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        58, 90,
        62, 95,
        62, 95,
        3,
#ifndef MONOCHROME_VECTORS
        128, 128, 0,
#endif
        58, 90,
        62, 95,
        58, 67,
        3,
#ifndef MONOCHROME_VECTORS
        150, 150, 0,
#endif
        53, 59,
        53, 84,
        53, 84,
        3,
#ifndef MONOCHROME_VECTORS
        150, 150, 0,
#endif
        53, 59,
        53, 84,
        53, 84,
        3,
#ifndef MONOCHROME_VECTORS
        150, 150, 0,
#endif
        53, 84,
        58, 67,
        58, 67,
        3,
#ifndef MONOCHROME_VECTORS
        150, 150, 0,
#endif
        53, 84,
        58, 67,
        53, 59,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 98,
        49, 99,
        49, 99,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 98,
        49, 99,
        87, 101,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 101,
        87, 101,
        87, 101,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 101,
        87, 101,
        87, 98,
        3,
#ifndef MONOCHROME_VECTORS
        92, 92, 0,
#endif
        74, 92,
        62, 95,
        62, 95,
        3,
#ifndef MONOCHROME_VECTORS
        92, 92, 0,
#endif
        74, 92,
        62, 95,
        74, 69,
        3,
#ifndef MONOCHROME_VECTORS
        92, 92, 0,
#endif
        86, 88,
        74, 92,
        74, 69,
        3,
#ifndef MONOCHROME_VECTORS
        92, 92, 0,
#endif
        86, 88,
        74, 69,
        86, 88,
        3,
#ifndef MONOCHROME_VECTORS
        100, 100, 0,
#endif
        62, 67,
        58, 67,
        58, 67,
        3,
#ifndef MONOCHROME_VECTORS
        100, 100, 0,
#endif
        62, 67,
        58, 67,
        62, 96,
        3,
#ifndef MONOCHROME_VECTORS
        100, 100, 0,
#endif
        74, 69,
        62, 67,
        62, 96,
        3,
#ifndef MONOCHROME_VECTORS
        100, 100, 0,
#endif
        74, 69,
        62, 96,
        74, 69,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        85, 88,
        74, 69,
        74, 69,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        85, 88,
        74, 69,
        85, 61,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        85, 88,
        85, 88,
        85, 61,
        3,
#ifndef MONOCHROME_VECTORS
        85, 68, 0,
#endif
        85, 88,
        85, 61,
        85, 88,
        0};
#endif

void MainMenu_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
}

void MainMenu_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
#ifndef MONOCHROME_VECTORS
        fillRect(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF00FF00), 0);
#endif
        drawGraphic(0, 0, YRES_FRAMEBUFFER, YRES_FRAMEBUFFER, splashGraphics);
        drawGraphic((XRES_FRAMEBUFFER / 2) + 8, 8, (XRES_FRAMEBUFFER / 2) - 16, (XRES_FRAMEBUFFER / 2) - 16, logoGraphics);
        playSound(MAIN_MENU_THEME);
    }

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) 9 - 4,
            (YRES_FRAMEBUFFER / 8) - 4 - kMainMenuOptionsCount,
            9 + 2,
            kMainMenuOptionsCount + 2,
            "Derelict",
            MainMenu_options,
            kMainMenuOptionsCount,
            cursorPosition);
}

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, void *data) {
    (void)data;
    return handleCursor(&MainMenu_nextStateNavigation[0], kMainMenuOptionsCount, cmd, kResumeCurrentState);
}

void MainMenu_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}
