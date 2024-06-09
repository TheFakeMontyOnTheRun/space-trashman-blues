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
const char *MainMenu_options[4] = {
        "Play game", "Credits", "Help", "Quit"};

const enum EGameMenuState MainMenu_nextStateNavigation[4] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

const uint8_t kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

const enum EGameMenuState MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const uint8_t kMainMenuOptionsCount = 3;
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
0, 0, 0,
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
88, 35,
88, 21,
49, 18,
46, 35,
5,
#ifndef MONOCHROME_VECTORS
0, 0, 0,
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
88, 21,
88, 1,
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
5,
#ifndef MONOCHROME_VECTORS
205, 222, 135,
#endif
1, 108,
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
8,
#ifndef MONOCHROME_VECTORS
147, 172, 147,
#endif
59, 39,
59, 39,
88, 42,
88, 42,
88, 36,
88, 36,
59, 39,
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
0, 0, 218639482,
#endif
27, 98,
36, 98,
49, 18,
78, 1,
68, 1,
43, 15,
27, 98,
5,
#ifndef MONOCHROME_VECTORS
0, 0, 218639482,
#endif
1, 1,
1, 7,
1, 21,
43, 17,
68, 1,
3,
#ifndef MONOCHROME_VECTORS
0, 0, 218639482,
#endif
43, 17,
46, 1,
68, 1,
4,
#ifndef MONOCHROME_VECTORS
0, 0, 218639482,
#endif
49, 18,
43, 17,
68, 1,
78, 1,
4,
#ifndef MONOCHROME_VECTORS
36, 34, 28,
#endif
43, 17,
1, 9,
0, 1,
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
8,
#ifndef MONOCHROME_VECTORS
170, 0, 0,
#endif
87, 32,
71, 33,
55, 36,
51, 57,
51, 86,
62, 98,
88, 92,
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
0, 0, 218639482,
#endif
62, 95,
58, 67,
53, 84,
3,
#ifndef MONOCHROME_VECTORS
0, 0, 218639482,
#endif
53, 84,
58, 67,
53, 59,
5,
#ifndef MONOCHROME_VECTORS
43, 0, 0,
#endif
49, 99,
49, 99,
87, 101,
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
        0, 0, 0,
#endif
        42, 57,
        39, 75,
        39, 75,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        42, 57,
        39, 75,
        42, 73,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        42, 73,
        71, 58,
        71, 58,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
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
        88, 35,
        88, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        81, 38,
        88, 21,
        81, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        88, 35,
        88, 21,
        88, 21,
        3,
#ifndef MONOCHROME_VECTORS
        183, 200, 183,
#endif
        88, 35,
        88, 21,
        88, 21,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        49, 18,
        46, 35,
        46, 35,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        49, 18,
        46, 35,
        49, 35,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        49, 35,
        72, 38,
        72, 22,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        49, 35,
        72, 22,
        49, 18,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
#endif
        72, 38,
        73, 22,
        73, 22,
        3,
#ifndef MONOCHROME_VECTORS
        0, 0, 0,
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
        88, 21,
        88, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        77, 22,
        88, 1,
        77, 1,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        88, 21,
        88, 21,
        88, 21,
        3,
#ifndef MONOCHROME_VECTORS
        138, 145, 111,
#endif
        88, 21,
        88, 21,
        88, 1,
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
        1, 108,
        1, 125,
        3,
#ifndef MONOCHROME_VECTORS
        205, 222, 135,
#endif
        1, 108,
        1, 125,
        1, 125,
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
        88, 42,
        88, 36,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        59, 39,
        88, 36,
        59, 39,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        88, 42,
        88, 36,
        88, 36,
        3,
#ifndef MONOCHROME_VECTORS
        147, 172, 147,
#endif
        88, 42,
        88, 36,
        88, 36,
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
        78, 1,
        78, 1,
        3,
#ifndef MONOCHROME_VECTORS
        227, 226, 219,
#endif
        68, 7,
        78, 1,
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
        1, 21,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 1,
        1, 21,
        3,
#ifndef MONOCHROME_VECTORS
        145, 138, 111,
#endif
        1, 1,
        1, 21,
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
        46, 15,
        43, 17,
        43, 17,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        46, 15,
        43, 17,
        46, 1,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        68, 1,
        46, 15,
        46, 1,
        3,
#ifndef MONOCHROME_VECTORS
        72, 62, 55,
#endif
        68, 1,
        46, 1,
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
        36, 34, 28,
#endif
        1, 9,
        0, 1,
        0, 1,
        3,
#ifndef MONOCHROME_VECTORS
        36, 34, 28,
#endif
        1, 9,
        0, 1,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        36, 34, 28,
#endif
        43, 17,
        1, 9,
        1, 1,
        3,
#ifndef MONOCHROME_VECTORS
        36, 34, 28,
#endif
        43, 17,
        1, 1,
        43, 1,
        3,
#ifndef MONOCHROME_VECTORS
        36, 34, 28,
#endif
        46, 1,
        43, 17,
        43, 1,
        3,
#ifndef MONOCHROME_VECTORS
        36, 34, 28,
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
        88, 92,
        88, 92,
        3,
#ifndef MONOCHROME_VECTORS
        170, 0, 0,
#endif
        87, 92,
        88, 92,
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
        87, 98,
        87, 98,
        87, 101,
        3,
#ifndef MONOCHROME_VECTORS
        43, 0, 0,
#endif
        87, 98,
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
    cursorPosition = 0;
}

void MainMenu_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
#ifndef MONOCHROME_VECTORS
        fillRect(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF00FF00), 0);
#endif
        drawGraphic(splashGraphics);
        playSound(MAIN_MENU_THEME);
    }

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) 9 - 4,
            (YRES_FRAMEBUFFER / 8) - 4 - kMainMenuOptionsCount,
            9 + 2,
            kMainMenuOptionsCount + 2,
            "Episode 0",
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
