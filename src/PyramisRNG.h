// (C) 2026 Daniel Sosa

#pragma once

#include "PyramisBlock.h"

#define PYRAMIS_RNG_BAG_ARRAY_SIZE 64

typedef struct PyramisRNG
{
	uint32_t seed;
	uint8_t maxColors;

	PyramisBlock bag[PYRAMIS_RNG_BAG_ARRAY_SIZE];
	uint8_t bagPopulation;

} PyramisRNG;

extern void PyramisRNGInit(PyramisRNG* rng, uint32_t seed, uint8_t maxColors);
extern PyramisBlock PyramisRNGGenerateBlock(PyramisRNG* rng);