//
//  main.m
//  The Mistral Report
//
//  Created by Daniel Monteiro on 10/13/18.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "OSXRenderer.h"
#include "GameView.h"

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "KeyboardUI.h"
#include "Menu.h"

int main(int argc, char **argv) {
    return NSApplicationMain(argc, (const char **)argv);
}
