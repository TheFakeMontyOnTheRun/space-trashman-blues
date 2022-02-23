//
//  OutcomeViewController.m
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/24/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import "AppDelegate.h"
#import "OutcomeViewController.h"

@interface OutcomeViewController ()

@end

@implementation OutcomeViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

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

- (IBAction) goTitle:(id)sender {
    [ ( ( AppDelegate* )[ [ UIApplication sharedApplication ] delegate ] ) changeScreen: SCREEN_TITLE ];
}
@end
