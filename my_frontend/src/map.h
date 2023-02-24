#ifndef MAP_H
#define MAP_H

#define BASE_CEILING_HEIGHT 12

//ceiling is at height 12, floor at height 0. Camera is at height 2

const struct Pattern patterns[96] = {
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   32
		{0,                   3,  CUBE,       0}, //   33
		{0,                   3,  RIGHT_NEAR, 0}, //   34
		{BASE_CEILING_HEIGHT, 3,  CUBE,       1}, //   35 #
		{3,                   3,  CUBE,       0}, //   36
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   37
		{0,                   0,  LEFT_WALL,  0}, //   38
		{0,                   3,  BACK_WALL,  0}, //   39
		{6,                   3,  CUBE,       0}, //   40
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   41
		{0,                   0,  CUBE,       0}, //   42
		{0,                   3,  CUBE,       0}, //   43
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   44 ,
		{0,                   15, BACK_WALL,  0}, //   45 -
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   46 .
		{0,                   3,  RIGHT_NEAR, 1}, //   47 /
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   48 0
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   49 1
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   50 2
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   51 3
		{0,                   0,  CUBE,       0}, //   52 4
		{0,                   3,  CUBE,       0}, //   53 5
		{0,                   3,  RIGHT_NEAR, 0}, //   54 6
		{0,                   3,  LEFT_NEAR,  0}, //   55 7
		{3,                   3,  CUBE,       0}, //   56 8
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   57 9
		{0,                   13, BACK_WALL,  0}, //  58 :
		{0,                   13, LEFT_WALL,  0}, //   59 ;
		{0,                   15, CORNER,     0}, //   60 <
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   61
		{0,                   0,  CUBE,       0}, //   62
		{0,                   3,  CUBE,       0}, //   63
		{0,                   3,  RIGHT_NEAR, 0}, //   64
		{0,                   3,  LEFT_NEAR,  0}, //   65
		{3,                   3,  CUBE,       0}, //   66
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   67
		{0,                   0,  CUBE,       0}, //   68 D
		{5,                   0,  CUBE,       0}, //   69 E
		{5,                   15, CUBE,       0}, //   70 F
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   71
		{1,                   15, CUBE,       1}, //   72
		{0,                   15, CUBE,       1}, //   73 I
		{0,                   3,  RIGHT_NEAR, 0}, //   74
		{0,                   3,  LEFT_NEAR,  0}, //   75
		{4,                   0,  CUBE,       0}, //   76 L
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   77
		{0,                   0,  LEFT_WALL,  0}, //   78
		{0,                   0,  CUBE,       0}, //   79
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   80
		{0,                   3,  CUBE,       0}, //   81
		{0,                   0,  CUBE,       0}, //   82
		{0,                   3,  CUBE,       0}, //   83
		{6,                   3,  CUBE,       0}, //   84
		{0,                   3,  LEFT_NEAR,  0}, //   85
		{3,                   3,  CUBE,       0}, //   86
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   87
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   88
		{0,                   3,  BACK_WALL,  0}, //   89
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   90
		{0,                   3,  LEFT_NEAR,  0}, //   91 [
		{0,                   3,  LEFT_NEAR,  1}, //   92 /*  \ */
		{0,                   3,  RIGHT_NEAR, 0}, //   93 ]
		{0,                   3,  RIGHT_NEAR, 0}, //   94
		{BASE_CEILING_HEIGHT, 3,  CUBE,       1}, //   95 _
		{3,                   3,  CUBE,       0}, //   96
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //   97
		{0,                   0,  LEFT_WALL,  0}, //   98
		{0,                   3,  BACK_WALL,  0}, //   99
		{6,                   3,  CUBE,       0}, //  100
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  101 e
		{0,                   0,  CUBE,       0}, //  102
		{0,                   3,  CUBE,       0}, //  103
		{0,                   3,  RIGHT_NEAR, 0}, //  104
		{0,                   3,  LEFT_NEAR,  0}, //  105
		{3,                   3,  CUBE,       0}, //  106
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  107
		{0,                   0,  LEFT_WALL,  0}, //  108
		{0,                   3,  BACK_WALL,  0}, //  109
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  110 n
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  111
		{0,                   0,  CUBE,       0}, //  112
		{0,                   3,  CUBE,       0}, //  113
		{0,                   3,  RIGHT_NEAR, 0}, //  114
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  115 s
		{3,                   3,  CUBE,       0}, //  116
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  117
		{0,                   0,  LEFT_WALL,  0}, //  118
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  119 w
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  120
		{BASE_CEILING_HEIGHT, 3,  CUBE,       0}, //  121
		{0,                   0,  CUBE,       0}, //  122
		{0,                   3,  CUBE,       0}, //  123
		{0,                   15, LEFT_WALL,  1}, //  124 |
		{0,                   3,  LEFT_NEAR,  0}, //  125
		{3,                   3,  CUBE,       0}, //  126
};

int8_t map[32][32];


#include "maps.h"

#endif
