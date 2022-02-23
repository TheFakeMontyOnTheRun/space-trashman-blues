//
//  Board.h
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/24/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "GameStateListener.h"

enum TileType{ TILETYPE_BLANK, TILETYPE_1, TILETYPE_2, TILETYPE_3, TILETYPE_4, TILETYPE_5, TILETYPE_6, TILETYPE_7, TILETYPE_8, TILETYPE_MINE, TILETYPE_TRIGGERED_MINE,TILETYPE_FLAGGED, TILES_TOTAL };
#define BOARD_SIZE 20
#define WIDTH BOARD_SIZE
#define HEIGHT BOARD_SIZE

@protocol DragDropImageViewDelegate;

@interface Board : NSObject {
    NSObject<GameStateListener> *listener;
    int minesPlaced;
    int remainingTilesToClear;
    int tileAt[ BOARD_SIZE ][ BOARD_SIZE ];
    BOOL covered[ BOARD_SIZE ][ BOARD_SIZE ];
    BOOL flagged[ BOARD_SIZE ][ BOARD_SIZE ];
}
- (id) init;
- ( BOOL )isCoveredAtX: (int) x andY: (int) y;
- ( BOOL )isFlaggedAtX: (int) x andY: (int) y;
- (void) pokeAtX: (int) x andY: (int) y;
- (void) flagAtX: (int) x andY: (int) y;
- (int) tileAtX: (int) x andY: (int) y;
- ( void ) floodUncoverAtX:( int ) x andY: ( int ) y;
- ( void ) placeNumbersOnBoard;
- ( void ) victory;
- ( void ) defeat;
- ( void ) uncoverAll;

@property ( retain, nonatomic) NSObject<GameStateListener> * listener;
@end
