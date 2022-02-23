//
//  ViewController.m
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/23/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import "AppDelegate.h"
#import "SplashViewController.h"

@interface SplashViewController ()

@end

@implementation SplashViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)playGame:(id)sender {
    [ ( ( AppDelegate* )[ [ UIApplication sharedApplication ] delegate ] ) changeScreen: SCREEN_GAME ];
}
@end
