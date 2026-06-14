// (C) 2026 Daniel Sosa

#pragma once

#include <stdint.h>
#include <stdbool.h>

// You're damn right it does!
// ...This struct contains rules and parameters that can tweak gameplay.
typedef struct PyramisRules
{
	// SPEED AND TIMING PARAMETERS
	float dropSpeedGravity; // The speed at which the drop block falls on its own.
	float dropSpeedSoft; // Soft drop speed.
	float dropLockDelay; // How long the drop block can stay on the ground before locking into place.
	float dasDelay; // The amount of time it takes for DAS to kick in.
	float dasRate; // The rate at which DAS shifts the block automatically.
	float lockEntryDelay; // How long the lock state lasts. Players can use IRS or initialize DAS during this time.
	float rotate180Window; // The amount of leniency the player is given when attempting to press both rotation buttons at the same time.
	float crushAnimationLength;
	float topOutShakeLength;

	// SCORING PARAMETERS
	uint8_t speedAndScorePercent; // This influences both the speed curve and the crush score multiplier.
	float dropSoftDistancePerPoint; // If the player soft drops this many units down, they will earn a point.
	float dropHardDistancePerPoint; // If the player hard drops this many units down, they will earn a point.
	uint_fast32_t maxScore; // In some game modes, if the player reaches this score, the game ends.

	// RNG RULES
	uint32_t RNGInitialSeed;
	uint8_t RNGMaxColors;

	// EXPERIMENTAL RULES
	bool pyramidWrapX; // If true, pyramid detection will wrap around the X axis.

	// If true, the player is allowed to hold a block for later.
	// Currently unimplemented. Some of my friends have kept asking me about adding this, but I
	// don't want to, as I want the gameplay to really be focused on improvisation.
	// So, as a sort of joke, I want the functionality to exist in the library, just never actually
	// implemented in any real game,
	bool allowHoldButton;

} PyramisRules;

extern void PyramisRulesSetDefaultVariables(PyramisRules* rules);