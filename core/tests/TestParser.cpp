//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gtest/gtest.h>

extern "C" {
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
}



TEST(TestParser, willNotRunCommandsWithInvalidInput) {
  ASSERT_FALSE(parseCommand("", ""));
}