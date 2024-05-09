//
//  GameView.m
//  Super Mare Imperium - Derelict
//
//  Created by Daniel Monteiro on 09/05/2024.
//

#import "GameViewTouch.h"

CGColorSpaceRef rgb;

@implementation GameViewTouch



// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
    rgb = CGColorSpaceCreateDeviceRGB();
}


- (void)viewDidLoad
{
    rgb = CGColorSpaceCreateDeviceRGB();
}

@end
