//
// Created by Daniel Monteiro on 08/10/2019.
//
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "Core.h"
#include "Parser.h"
#include "Derelict.h"

#include <stdio.h>
#include <string.h>


int parseCommand(char *cmd, char *operand) {
    if (!strcmp(cmd, "pick")) {
        pickObjectByName(operand);
    } else if (!strcmp(cmd, "drop")) {
        dropObjectByName(operand);
    } else if (!strcmp(cmd, "move")) {

        if (operand != NULL) {
            moveBy(operand[0] - '0');
        } else {
            defaultLogger("Please specify a valid direction");
        }
    } else if (!strcmp(cmd, "use")) {
        useObjectNamed(operand);

    } else if (!strcmp(cmd, "w")) {
        walkBy(0);
    } else if (!strcmp(cmd, "s")) {
        walkBy(2);
    } else if (!strcmp(cmd, "a")) {
        walkBy(3);
    } else if (!strcmp(cmd, "d")) {
        walkBy(1);
    } else if (!strcmp(cmd, "q")) {
        turnLeft();
    } else if (!strcmp(cmd, "e")) {
        turnRight();
    } else if (!strcmp(cmd, "walkTo")) {
        walkTo(operand);
    } else if (!strcmp(cmd, "info")) {
        infoAboutItemNamed(operand);
    } else if (!strcmp(cmd, "use-with")) {
        useObjectsTogether(operand);
    } else {
        defaultLogger("Unrecognized command");
        return FALSE;
    }

    return TRUE;
}
