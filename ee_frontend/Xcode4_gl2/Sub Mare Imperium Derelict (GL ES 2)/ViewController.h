//
//  ViewController.h
//  Sub Mare Imperium: Derelict (GL ES)
//
//  Created by Daniel Monteiro on 12/29/23.
//  Copyright (c) 2023 Daniel Monteiro. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface ViewController : GLKViewController {
    
}
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
