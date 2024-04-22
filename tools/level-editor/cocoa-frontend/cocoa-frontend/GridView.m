//
//  GridView.m
//  cocoa-frontend
//
//  Created by Daniel Monteiro on 4/22/24.
//  Copyright (c) 2024 Daniel Monteiro. All rights reserved.
//

#import "GridView.h"

@implementation GridView

CGDataProviderRef provider;
CGImageRef ref;


- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{

    CGRect bounds;

    bounds.origin.x = 5;
    bounds.origin.y = 10;
    bounds.size.width = 20;
    bounds.size.height = 30;

    /*  MAC_OS_X_VERSION_10_10*/
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101000
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
#else
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
    
    CGContextSaveGState(context);
    
    CGContextSetRGBFillColor(context, 1.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(context, bounds);


    
    
    
    CGContextRestoreGState(context);
}

@end
