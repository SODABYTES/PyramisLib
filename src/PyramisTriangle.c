// (C) 2026 Daniel Sosa

#include "PyramisTriangle.h"

void PyramisTriangleSetDefaultVariables(PyramisTriangle* triangle)
{
    triangle->color = 0;
    triangle->pyramidValue = 0;
    triangle->pyramidValuePreview = 0;
}

unsigned int PyramisTriangleGetScoreFromPyramidValue(PyramisTriangle triangle)
{
    return triangle.pyramidValue * 300;

    // OLDER EXPONENTIAL LOGIC THAT I'M KEEPING COMMENTED OUT JUST IN CASE?
    /*
    unsigned int result = 1;
    for (unsigned int i = 0; i < triangle.pyramidValue - 1; i++)
    {
        result = result * 2;
    }
    result = result * 300;
    return result;
    */
}