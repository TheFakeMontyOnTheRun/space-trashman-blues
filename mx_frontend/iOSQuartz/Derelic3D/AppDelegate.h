//
//  AppDelegate.h
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/23/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import <UIKit/UIKit.h>



enum GAME_SCREENS{ SCREEN_TITLE, SCREEN_GAME, SCREEN_OUTCOME };


@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>

- (void) changeScreen: (int) screen;

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) UIViewController *viewController;

@end
