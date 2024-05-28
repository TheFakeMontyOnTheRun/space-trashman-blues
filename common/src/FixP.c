#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>

#endif

#ifndef SMD

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef WIN32

#include <unistd.h>

#endif
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif

#include "Common.h"
#include "FixP.h"


FixP_t lerpFix(const FixP_t v0, const FixP_t v1, const FixP_t dt, const FixP_t total) {
    FixP_t delta = (v1 - v0);
    FixP_t progress = Div(dt, total);
    FixP_t reach = Mul(delta, progress);

    return (v0 + reach);
}

int lerpInt(const int v0, const int v1, const long t, const long total) {
    return fixToInt(lerpFix(intToFix(v0), intToFix(v1), intToFix(t),
                            intToFix(total)));
}
