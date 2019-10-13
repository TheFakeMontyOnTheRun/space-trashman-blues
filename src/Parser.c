//
// Created by Daniel Monteiro on 08/10/2019.
//
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
		moveBy(operand[0] - '0');
	} else {
		return FALSE;
	}

	return TRUE;
}
