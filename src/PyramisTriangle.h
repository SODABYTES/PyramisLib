// (C) 2026 Daniel Sosa

#pragma once

#include <stdint.h>

typedef struct PyramisTriangle
{
    // The color of this triangle.
    uint_fast8_t color;

    // This triangle's pyramid value. This is determined by the size and quantity of the pyramids
    // that this triangle is a part of.
    unsigned int pyramidValue;

    // The same idea as pyramidValue, except this is specifically for the "drop preview" feature.
    unsigned int pyramidValuePreview;
} PyramisTriangle;

extern void PyramisTriangleSetDefaultVariables(PyramisTriangle* triangle);
extern unsigned int PyramisTriangleGetScoreFromPyramidValue(PyramisTriangle triangle);