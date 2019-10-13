//
// Created by Daniel Monteiro on 08/10/2019.
//
#include "Parser.h"
#include "Derelict.h"

#include <stdio.h>
#include <string.h>

void parseCommand(char *cmd, char *operand) {
  if (!strcmp(cmd, "pick")) {
    pickObjectByName(operand);
  } else if (!strcmp(cmd, "drop")) {
    dropObjectByName(operand);
  } else if (!strcmp(cmd, "move")) {
    printf("direction: %d\n", operand[0] - '0');
    moveBy(operand[0] - '0');
  } else {
    puts("WHAT?!");
  }
}
