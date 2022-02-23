//
//  GameViewController.h
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/23/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Board.h"

enum InteractionMode { GAMEMODE_POKE, GAMEMODE_MOVEMAP, GAMEMODE_FLAG };

@interface GameViewController : UIViewController<GameStateListener> {
    IBOutlet UIImageView *playField;
    Board *gameBoard;
    UIImage *covered;
    UIImage *flagged;
    UIImage *tiles[ 11 ];
    CGPoint cameraPosition;
    CGPoint lastTouch;
    int mode;
    IBOutlet UISegmentedControl *modeSelector;
}
- (void) draw;
- (void) goOutcome;
- (IBAction) gameModeChanged:(id)sender;
- (IBAction)backToMenu:(id)sender;
-(IBAction) getLine:(id) sender;
- ( void ) defeat;
- ( void ) victory;
@end
