//
//  TitleScreen.m
//  Derelict128
//
//  Created by Daniel Monteiro on 1/4/24.
//  Copyright (c) 2024 Daniel Monteiro. All rights reserved.
//
#import "AppDelegate.h"
#import "TitleScreen.h"

@interface TitleScreen ()

@end

@implementation TitleScreen
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}
*/
- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
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
