// (C) 2026 Daniel Sosa

#pragma once

#include "PyramisBlock.h"
#include "PyramisConstants.h"

typedef struct PyramisPyramidResults
{
	unsigned int pyramids;
	PyramisBlock playfield[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y];

} PyramisPyramidResults;