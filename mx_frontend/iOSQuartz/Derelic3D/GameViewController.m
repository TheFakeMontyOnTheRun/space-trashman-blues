//
//  GameViewController.m
//  Derelict3D
//
//  Created by Daniel Monteiro on 2/23/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import "AppDelegate.h"
#import "Board.h"
#import "GameViewController.h"

@interface GameViewController ()

@end

@implementation GameViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        
        gameBoard = [[ Board alloc ] init ];
        
        
        tiles[ 0 ] =    [UIImage imageNamed:@"images/blanksvg.png"];
        tiles[ 1 ] =    [UIImage imageNamed:@"images/n1svg.png"];
        tiles[ 2 ] =    [UIImage imageNamed:@"images/n2svg.png"];
        tiles[ 3 ] =    [UIImage imageNamed:@"images/n3svg.png"];
        tiles[ 4 ] =    [UIImage imageNamed:@"images/n4svg.png"];
        tiles[ 5 ] =    [UIImage imageNamed:@"images/n5svg.png"];
        tiles[ 6 ] =    [UIImage imageNamed:@"images/n6svg.png"];
        tiles[ 7 ] =    [UIImage imageNamed:@"images/n7svg.png"];
        tiles[ 8 ] =    [UIImage imageNamed:@"images/n8svg.png"];
        tiles[ 9 ] =    [UIImage imageNamed:@"images/minesvg.png"];
        tiles[ 10 ] =    [UIImage imageNamed:@"images/minespokedsvg.png"];
        
        flagged =    [UIImage imageNamed:@"images/flagged.png"];
        
        covered =    [UIImage imageNamed:@"images/coveredsvg.png"];
        
        mode = GAMEMODE_POKE;
        
        
        
        gameBoard.listener = self;
    }
    
    
    
    return self;
}

- (IBAction) gameModeChanged:(id)sender {
    
    mode = [ modeSelector selectedSegmentIndex ];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    [ self draw ];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {

    UITouch *touch = [touches anyObject];
    
    CGPoint touchInRect = [ touch locationInView: playField ];
    int relativeX;
    int relativeY;
    
    relativeX = ( touchInRect.x + cameraPosition.x ) / [ tiles[ 0 ] size ].width;
    relativeY = ( touchInRect.y + cameraPosition.y ) / [ tiles[ 0 ] size ].height;
    
    
    if ( CGRectContainsPoint( [ playField frame ], touchInRect ) ) {
        
        switch ( mode ) {
            case GAMEMODE_POKE:
                [ gameBoard pokeAtX: relativeX andY: relativeY ];
                break;
            case GAMEMODE_MOVEMAP:

                cameraPosition.x += (lastTouch.x - touchInRect.x );
                cameraPosition.y += (lastTouch.y - touchInRect.y );
                break;
        }
        
        lastTouch.x = touchInRect.x;
        lastTouch.y = touchInRect.y;
        [ self draw ];

    }
}


- ( void ) defeat {
    [ self performSelector: @selector( goOutcome ) withObject: self afterDelay: 2 ];
}

- ( void ) victory {
    [ self performSelector: @selector( goOutcome ) withObject: self afterDelay: 2 ];
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch *touch = [touches anyObject];
    
    if ([touch view] == playField )
    {
        
            
        CGPoint touchLocation = [touch locationInView: playField ];
        int relativeX;
        int relativeY;
        
        relativeX = ( touchLocation.x + cameraPosition.x ) / [ tiles[ 0 ] size ].width;
        relativeY = ( touchLocation.y + cameraPosition.y ) / [ tiles[ 0 ] size ].height;
        
        
        switch ( mode ) {
                
            case GAMEMODE_POKE:
                [ gameBoard pokeAtX: relativeX andY: relativeY ];
                break;
                
            case GAMEMODE_FLAG:
                
                [ gameBoard flagAtX: relativeX andY: relativeY ];
                break;
		}       
        
        lastTouch.x = touchLocation.x;
        lastTouch.y = touchLocation.y;
        [ self draw ];
    }
    
}




- ( void ) draw {
    
    UIGraphicsBeginImageContext( playField.frame.size);
    
    UIImage *img;
    CGRect rect = CGRectMake( 20, 20, 64, 64 );
    
    int startX = cameraPosition.x / rect.size.width;
    int startY = cameraPosition.y / rect.size.height;
    
    for ( int y = startY; y < BOARD_SIZE; ++y ) {
        
        for ( int x = startX; x < BOARD_SIZE; ++x ) {
            
            if ( [ gameBoard isCoveredAtX: x andY: y ] ) {

                if ( [ gameBoard isFlaggedAtX: x andY: y ] )
                    
                    img = flagged;
                else
                    img = covered;
                
            } else
                img = tiles[ [ gameBoard tileAtX: x andY: y ]  ];
            
            rect.origin.y = ( y * img.size.height ) - cameraPosition.y;
            rect.origin.x = ( x * img.size.width  ) - cameraPosition.x;
            rect.size.width = img.size.width + 2;
            rect.size.height = img.size.width + 2;
            [img drawInRect: rect ];
        }
    }
    
    
    playField.image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
}

-(IBAction) getLine:(id) sender{
    
    [ self draw ];
    [ self performSelector: @selector( goOutcome ) withObject: self afterDelay: 2 ];
}

-(void) goOutcome {
    [ ( ( AppDelegate* )[ [ UIApplication sharedApplication ] delegate ] ) changeScreen: SCREEN_OUTCOME ];    
}

- (IBAction)backToMenu:(id)sender {
    [ ( ( AppDelegate* )[ [ UIApplication sharedApplication ] delegate ] ) changeScreen: SCREEN_TITLE ];
}
@end
