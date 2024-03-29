//
//  GameView.h
//  The Mistral Report
//
//  Created by Daniel Monteiro on 10/13/18.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface GameView : NSView {
    NSTimer* timer;
    int bufferedInput;
}
-(void) initTimer;
-(void) stopTimer;
- (void)drawRect:(NSRect)rect;
- (int) getInput;

-(void)keyUp:(NSEvent*)event;
-(void)keyDown:(NSEvent*)event;
- (BOOL)acceptsFirstResponder;
@end
