// (C) 2026 Daniel Sosa

#include "PyramisRNG.h"

void PyramisRNGInit(PyramisRNG* rng, uint32_t seed, uint8_t maxColors)
{
	rng->seed = seed;
	rng->maxColors = maxColors;

	for (int i = 0; i < PYRAMIS_RNG_BAG_ARRAY_SIZE; i++)
	{
		PyramisBlockSetDefaultVariables(&rng->bag[i]);
	}
	rng->bagPopulation = 0;
}

void PyramisRNGFillBag(PyramisRNG* rng)
{
	rng->bagPopulation = 0;
	for (int a = 0; a < rng->maxColors; a++)
	{
		for (int b = 0; b < rng->maxColors; b++)
		{
			PyramisBlock* targetBlock = &rng->bag[rng->bagPopulation];
			PyramisBlockSetDefaultVariables(targetBlock);
			targetBlock->empty = false;
			targetBlock->rotated = false;
			targetBlock->triangleA.color = a;
			targetBlock->triangleB.color = b;

			rng->bagPopulation += 1;
		}
	}
}

uint32_t PyramisRNGGetRandomNumber(PyramisRNG* rng, uint32_t max)
{
	// Knuthmaxxing
	rng->seed = rng->seed * 1664525 + 1013904223;

	// Limit the result
	uint32_t result = (rng->seed / (UINT32_MAX / max)) % max;

	return result;
}

PyramisBlock PyramisRNGGenerateBlock(PyramisRNG* rng)
{
	PyramisBlock newBlock;
	PyramisBlockSetDefaultVariables(&newBlock);

	// Populate the bag if we need to
	if (rng->bagPopulation <= 0)
	{
		PyramisRNGFillBag(rng);
	}

	// Select one of the blocks from the bag
	uint32_t selection = PyramisRNGGetRandomNumber(rng, rng->bagPopulation);

	newBlock = rng->bag[selection];

	// Remove the block from the bag
	for (int i = selection; i < PYRAMIS_RNG_BAG_ARRAY_SIZE - 1; i++)
	{
		rng->bag[i] = rng->bag[i + 1];
	}
	rng->bagPopulation -= 1;
	
	return newBlock;
}