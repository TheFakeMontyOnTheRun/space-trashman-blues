//
//  GameView.h
//  Madame Y
//
//  Created by Daniel Monteiro on 27/05/2020.
//  Copyright © 2020 Daniel Monteiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

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

NS_ASSUME_NONNULL_END
