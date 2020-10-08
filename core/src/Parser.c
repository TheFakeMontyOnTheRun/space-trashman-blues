//
// Created by Daniel Monteiro on 08/10/2019.
//
#include <stdint.h>

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

    if (operand != NULL ) {
      moveBy(operand[0] - '0');
    } else {
      notifyError("Please specify a valid direction");
    }

  } else if (!strcmp(cmd, "use")) {
    useObjectNamed(operand);
  } else if (!strcmp(cmd, "use-with")) {
    useObjectsTogether(operand);
  } else {
    notifyError("Unrecognized command");
    return FALSE;
  }
  
  return TRUE;
}
