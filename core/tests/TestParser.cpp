//
// Created by Daniel Monteiro on 2019-07-26.
//

#include <gtest/gtest.h>
#include <stdint.h>

extern "C" {
#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
}

static void myDummyOutputHandler(const char *msg) {}


TEST(TestParser, willNotRunCommandsWithInvalidInput) {

    setLoggerDelegate(myDummyOutputHandler);

    ASSERT_FALSE(parseCommand("", ""));
}
