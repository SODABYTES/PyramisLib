// (C) 2026 Daniel Sosa

#include "PyramisBlock.h"

void PyramisBlockSetDefaultVariables(PyramisBlock* block)
{
	PyramisTriangleSetDefaultVariables(&block->triangleA);
	PyramisTriangleSetDefaultVariables(&block->triangleB);
	block->empty = true;
	block->miniPyramid = false;
	block->rotated = false;
	block->garbage = false;
}

bool PyramisBlockQualifiesForPyramid(PyramisBlock* block)
{
	bool result = true;
	if (block->empty || block->garbage)
	{
		result = false;
	}
	return result;
}

void PyramisBlockCopy(PyramisBlock* to, PyramisBlock* from)
{
	to->triangleA = from->triangleA;
	to->triangleB = from->triangleB;
	to->empty = from->empty;
	to->miniPyramid = from->miniPyramid;
	to->rotated = from->rotated;
	to->garbage = from->garbage;
}