//
//  GameWindow.m
//  The Mistral Report OSX
//
//  Created by Daniel Monteiro on 6/15/20.
//  Copyright 2020 __MyCompanyName__. All rights reserved.
//

#import "GameWindow.h"
#import "GameView.h"

@implementation GameWindow

-(BOOL)makeFirstResponder:(NSResponder *)aResponder {
    
    /* UGLY HACK */
    if ([self respondsToSelector:@selector(setCollectionBehavior:) ]) {
        [self setCollectionBehavior:
         NSWindowCollectionBehaviorFullScreenPrimary];        
    }
    
    return [super makeFirstResponder: aResponder];
}

/* UGLY HACK */
- (void)zoom:(id)sender {
	[super zoom: sender ];
	
	NSView *view = [[[self contentView] subviews ] objectAtIndex: 0 ];
	
	if ( [ view isKindOfClass: [GameView class] ]) {
		GameView *gv = (GameView*)view;
		[gv viewDidEndLiveResize];
	}
}


@end
