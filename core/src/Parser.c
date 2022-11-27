/*
Created by Daniel Monteiro on 08/10/2019.
*/

#ifndef DONT_INCLUDE
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include "Core.h"
#include "Parser.h"
#include <string.h>
#endif

int parseCommand(const char *cmd, const char *operand) {
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
#ifdef INCLUDE_ITEM_DESCRIPTIONS
    } else if (!strcmp(cmd, "info")) {
        infoAboutItemNamed(operand);
#endif
    } else if (!strcmp(cmd, "use-with")) {
        useObjectsTogether(operand);
    } else {
        defaultLogger("Unrecognized command");
        return FALSE;
    }

    return TRUE;
}
