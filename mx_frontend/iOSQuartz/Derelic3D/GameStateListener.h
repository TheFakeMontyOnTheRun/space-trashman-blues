//
//  GameStateListener.h
//  Derelict3D
//
//  Created by Daniel Monteiro on 3/24/13.
//  Copyright (c) 2013 Daniel Monteiro. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol GameStateListener <NSObject>
    - ( void ) defeat;
    - ( void ) victory;
@end
