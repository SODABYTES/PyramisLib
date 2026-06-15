// (C) 2026 Daniel Sosa

#pragma once

#include "PyramisTriangle.h"
#include <stdbool.h>

// This struct contains an individual block's data.
typedef struct PyramisBlock
{
    // The individual triangular slices of the block.
    // triangleA is always the one on top.

    PyramisTriangle triangleA; // The triangle on the top.
    PyramisTriangle triangleB; // The triangle on the bottom.

    // If true, this block should be treated like an empty spot on the grid.
    bool empty;

    // If true, this block is part of a mini pyramid.
    bool miniPyramid;

    // The block's rotation.
    // If false, the line through the block looks like this: [/]
    // If true, the line through the block looks like this: [\]
    bool rotated;

    // If true, this block is treated as a garbage / nuisance block.
    bool garbage;
} PyramisBlock;

extern void PyramisBlockSetDefaultVariables(PyramisBlock* block);
extern bool PyramisBlockQualifiesForPyramid(PyramisBlock* block);