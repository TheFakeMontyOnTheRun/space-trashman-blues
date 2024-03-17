//
//  ViewController.h
//  Derelict32-ios
//
//  Created by Daniel Monteiro on 3/11/24.
//  Copyright (c) 2024 Daniel Monteiro. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController {
    IBOutlet UIImageView *playField;
}

- (void) draw;


-(IBAction)btnUpPressed:(id)sender;

-(IBAction)btnDownPressed:(id)sender;

-(IBAction)btnLeftPressed:(id)sender;

-(IBAction)btnRightPressed:(id)sender;

-(IBAction)btnStrafeLeftPressed:(id)sender;

-(IBAction)btnStrafeRightPressed:(id)sender;

-(IBAction)btnFire1Pressed:(id)sender;

-(IBAction)btnFire2Pressed:(id)sender;

-(IBAction)btnFire3Pressed:(id)sender;

-(IBAction)btnFire4Pressed:(id)sender;
@end
