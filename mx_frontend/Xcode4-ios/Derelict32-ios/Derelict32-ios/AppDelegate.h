//
//  AppDelegate.h
//  Derelict32-ios
//
//  Created by Daniel Monteiro on 3/11/24.
//  Copyright (c) 2024 Daniel Monteiro. All rights reserved.
//

#import <UIKit/UIKit.h>

enum GAME_SCREENS{ SCREEN_TITLE, SCREEN_GAME, SCREEN_OUTCOME };

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>

- (void) changeScreen: (int) screen;

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) UIViewController *viewController;



@end
