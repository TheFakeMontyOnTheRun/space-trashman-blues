#ifndef MAP_H
#define MAP_H

//ceiling is at height 7, floor at height 0. Camera is at height 2
const struct Pattern patterns[96] = {
		{7, 3,  kCube,       0}, //   32
		{0, 3,  kCube,       0}, //   33
		{0, 3,  kRightNearWall, 0}, //   34
		{7, 3,  kCube,       1}, //   35 #
		{3, 3,  kCube,       0}, //   36
		{7, 3,  kCube,       0}, //   37
		{0, 0,  kLeftNearWall,  0}, //   38
		{0, 3,  kWallNorth,  0}, //   39
		{6, 3,  kCube,       0}, //   40
		{7, 3,  kCube,       0}, //   41
		{0, 0,  kCube,       0}, //   42
		{0, 3,  kCube,       0}, //   43
		{7, 3,  kCube,       0}, //   44 ,
		{0, 15, kWallNorth,  0}, //   45 -
		{7, 3,  kCube,       0}, //   46 .
		{0, 3,  kRightNearWall, 1}, //   47 /
		{7, 3,  kCube,       0}, //   48 0
		{7, 3,  kCube,       0}, //   49 1
		{7, 3,  kCube,       0}, //   50 2
		{7, 3,  kCube,       0}, //   51 3
		{0, 0,  kCube,       0}, //   52 4
		{0, 3,  kCube,       0}, //   53 5
		{0, 3,  kRightNearWall, 0}, //   54 6
		{0, 3,  kLeftNearWall,  0}, //   55 7
		{3, 3,  kCube,       0}, //   56 8
		{7, 3,  kCube,       0}, //   57 9
		{0, 0,  kLeftNearWall,  0}, //   58
		{0, 3,  kWallNorth,  0}, //   59
		{0, 15, kWallCorner,     0}, //   60 <
		{7, 3,  kCube,       0}, //   61
		{0, 0,  kCube,       0}, //   62
		{0, 3,  kCube,       0}, //   63
		{0, 3,  kRightNearWall, 0}, //   64
		{0, 3,  kLeftNearWall,  0}, //   65
		{3, 3,  kCube,       0}, //   66
		{7, 3,  kCube,       0}, //   67
		{0, 0,  kCube,       0}, //   68 D
		{5, 0,  kCube,       0}, //   69 E
		{5, 15, kCube,       0}, //   70 F
		{7, 3,  kCube,       0}, //   71
		{1, 15, kCube,       1}, //   72
		{0, 15, kCube,       1}, //   73 I
		{0, 3,  kRightNearWall, 0}, //   74
		{0, 3,  kLeftNearWall,  0}, //   75
		{4, 0,  kCube,       0}, //   76 L
		{7, 3,  kCube,       0}, //   77
		{0, 0,  kLeftNearWall,  0}, //   78
		{0, 0,  kCube,       0}, //   79
		{7, 3,  kCube,       0}, //   80
		{0, 3,  kCube,       0}, //   81
		{0, 0,  kCube,       0}, //   82
		{0, 3,  kCube,       0}, //   83
		{6, 3,  kCube,       0}, //   84
		{0, 3,  kLeftNearWall,  0}, //   85
		{3, 3,  kCube,       0}, //   86
		{7, 3,  kCube,       0}, //   87
		{7, 3,  kCube,       0}, //   88
		{0, 3,  kWallNorth,  0}, //   89
		{7, 3,  kCube,       0}, //   90
		{0, 3,  kLeftNearWall,  0}, //   91 [
		{0, 3,  kLeftNearWall,  1}, //   92 /*  \ */
		{0, 3,  kRightNearWall, 0}, //   93 ]
		{0, 3,  kRightNearWall, 0}, //   94
		{0, 3,  kLeftNearWall,  0}, //   95 _
		{3, 3,  kCube,       0}, //   96
		{7, 3,  kCube,       0}, //   97
		{0, 0,  kLeftNearWall,  0}, //   98
		{0, 3,  kWallNorth,  0}, //   99
		{6, 3,  kCube,       0}, //  100
		{7, 3,  kCube,       0}, //  101 e
		{0, 0,  kCube,       0}, //  102
		{0, 3,  kCube,       0}, //  103
		{0, 3,  kRightNearWall, 0}, //  104
		{0, 3,  kLeftNearWall,  0}, //  105
		{3, 3,  kCube,       0}, //  106
		{7, 3,  kCube,       0}, //  107
		{0, 0,  kLeftNearWall,  0}, //  108
		{0, 3,  kWallNorth,  0}, //  109
		{7, 3,  kCube,       0}, //  110 n
		{7, 3,  kCube,       0}, //  111
		{0, 0,  kCube,       0}, //  112
		{0, 3,  kCube,       0}, //  113
		{0, 3,  kRightNearWall, 0}, //  114
		{7, 3,  kCube,       0}, //  115 s
		{3, 3,  kCube,       0}, //  116
		{7, 3,  kCube,       0}, //  117
		{0, 0,  kLeftNearWall,  0}, //  118
		{7, 3,  kCube,       0}, //  119 w
		{7, 3,  kCube,       0}, //  120
		{7, 3,  kCube,       0}, //  121
		{0, 0,  kCube,       0}, //  122
		{0, 3,  kCube,       0}, //  123
		{0, 15, kLeftNearWall,  1}, //  124 |
		{0, 3,  kLeftNearWall,  0}, //  125
		{3, 3,  kCube,       0}, //  126
};

int8_t *map[32];
#endif
