//
//  Board.m
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/24/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import "AppDelegate.h"
#import "Board.h"

@implementation Board

@synthesize listener;

    - ( int ) tileAtX: (int) x andY: (int) y {
        
        if ( x >= BOARD_SIZE || y >= BOARD_SIZE || x < 0 || y < 0 ) {
            return TILETYPE_BLANK;
        } else {
            return tileAt[ y ][ x ];
        }
    }

    - (void) pokeAtX: (int) x andY: (int) y {
        
        if ( x >= BOARD_SIZE || y >= BOARD_SIZE || x < 0 || y < 0 ) {
            return;
        }
        
        if ( tileAt[ y ][ x ] == TILETYPE_MINE ) {
            
            tileAt[ y ][ x ] = TILETYPE_TRIGGERED_MINE;
            [ self defeat ];           

        } else {

            [ self floodUncoverAtX: x andY: y ];
            
            if ( remainingTilesToClear == minesPlaced ) {
                [ self victory ];
            }
                
        }
    }

    - ( void ) victory {
        
        [ self uncoverAll ];
        
        if ( listener != nil )
            [ listener victory ];
    }

    - ( void ) defeat {

        [ self uncoverAll ];
        
        if ( listener != nil )
            [ listener defeat ];
        
    }

    - (void) uncoverAll {
    
        int c =0;
        int d =0;
        
        for ( c = 0; c < BOARD_SIZE; ++c ) {
            for ( d = 0; d < BOARD_SIZE; ++d ) {
   				covered[ c ][ d ] = NO;
            }
        }
    }

    - ( void ) floodUncoverAtX:( int ) x andY: ( int ) y {
        
        if ( covered[ y ][ x ] && !flagged[ y ][ x ] ) {
			
			if ( tileAt[ y ][ x ] == TILETYPE_BLANK ) {
				
				if ( covered[ y ][ x ] )
					remainingTilesToClear--;
				
				covered[ y ][ x ] = NO;
				
				if ( x > 0)
                    [ self floodUncoverAtX: ( x - 1 ) andY: y ];
				
				if ( ( x + 1 ) < WIDTH )
                    [ self floodUncoverAtX: ( x + 1 ) andY: y ];
				
				if ( y > 0 )
                    [ self floodUncoverAtX: x andY: ( y - 1 ) ];
				
				if ( ( y + 1 ) < HEIGHT )
                    [ self floodUncoverAtX: x andY: ( y + 1 ) ];
				
				if ( x > 0 && y > 0 )
                    [ self floodUncoverAtX: ( x - 1 ) andY: ( y - 1 ) ];
				
				if ( ( x + 1 ) < WIDTH && ( y + 1 ) < HEIGHT )
                    [ self floodUncoverAtX: ( x + 1 ) andY: ( y + 1 ) ];
				
				if ( y > 0 && ( x + 1 ) < WIDTH )
                    [ self floodUncoverAtX: ( x + 1 ) andY: ( y - 1 ) ];
				
				if ( ( y + 1 ) < HEIGHT && x > 0 )
                    [ self floodUncoverAtX: ( x - 1 ) andY: ( y + 1 ) ];
				
				
			} if ( tileAt[ y ][ x ] != TILETYPE_MINE ) {
				
				if ( covered[ y ][ x ] )
					remainingTilesToClear--;
				
				covered[ y ][ x ] = NO;
			}
		}
    }

    - (void) flagAtX:(int)x andY:(int)y  {
        
        if ( x >= BOARD_SIZE || y >= BOARD_SIZE || x < 0 || y < 0 ) {
            return;
        }
        
        flagged[ y ][ x ] = !flagged[ y ][ x ];
        
    }


    - ( BOOL )isCoveredAtX: (int) x andY: (int) y {
        
        if ( x >= BOARD_SIZE || y >= BOARD_SIZE || x < 0 || y < 0 ) {
            return NO;
        }
        
        return covered[ y ][ x ];
    }

    - ( BOOL )isFlaggedAtX: (int) x andY: (int) y {
        
        if ( x >= BOARD_SIZE || y >= BOARD_SIZE || x < 0 || y < 0 ) {
            return NO;
        }
        
        return flagged[ y ][ x ];
    }


    - (id) init {
        
         srand (time(NULL));
        int minesToPlace = 33;
        minesPlaced = minesToPlace;
        remainingTilesToClear = BOARD_SIZE * BOARD_SIZE;
        int c =0;
        int d =0;
        
        for ( c = 0; c < BOARD_SIZE; ++c ) {
            
            
            for ( d = 0; d < BOARD_SIZE; ++d ) {
                
                tileAt[ c ][ d ] = TILETYPE_BLANK;
                covered[ c ][ d ] = YES;
                flagged[ c ][ d ] = NO;
            }
        }
        
        
        while ( minesToPlace > 0 ) {
            
            c = rand() % BOARD_SIZE;
            d = rand() % BOARD_SIZE;
            
            if ( c > 0 && d > 0 && c < ( BOARD_SIZE - 2 ) && d < ( BOARD_SIZE - 2 ) && tileAt[ c ][ d ] != TILETYPE_MINE ) {
            
                tileAt[ c ][ d ] = TILETYPE_MINE;
                --minesToPlace;
            }
        }
       

        [ self placeNumbersOnBoard ];
        return self;
    }


- ( void ) placeNumbersOnBoard {
    
    for ( int x = 1; x < BOARD_SIZE - 1; ++x ) {
        for ( int y = 1; y < BOARD_SIZE - 1; ++y ) {
            if ( tileAt[ y ][ x ] == TILETYPE_MINE ) {
                
                if ( tileAt[ y - 1 ][ x - 1 ] != TILETYPE_MINE)
                    ++tileAt[ y - 1 ][ x - 1 ];
                
                if ( tileAt[ y ][ x - 1 ] != TILETYPE_MINE)
                    ++tileAt[ y ][ x - 1];
                
                if ( tileAt[ y + 1 ][ x - 1 ] != TILETYPE_MINE)
                    ++tileAt[ y + 1 ][ x - 1 ];
                
                if ( tileAt[ y - 1 ][ x ] != TILETYPE_MINE)
                    ++tileAt[ y - 1][ x ];
                
                if ( tileAt[ y + 1 ][ x ] != TILETYPE_MINE)
                    ++tileAt[ y + 1 ][ x ];
                
                if ( tileAt[ y - 1 ][ x + 1 ] != TILETYPE_MINE)
                    ++tileAt[ y - 1 ][ x + 1 ];
                
                if ( tileAt[ y ][ x + 1 ] != TILETYPE_MINE)
                    ++tileAt[ y ][ x + 1 ];
                
                if ( tileAt[ y + 1 ][ x + 1 ] != TILETYPE_MINE)
                    ++tileAt[ y + 1 ][ x + 1 ];
            }
        }
    }		
}
@end
