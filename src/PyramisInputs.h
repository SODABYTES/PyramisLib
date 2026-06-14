// (C) 2026 Daniel Sosa
//
// This struct contains data representing the player's inputs on their controller.

#pragma once

#include <stdbool.h>

typedef struct PyramisInputs
{
	bool moveLeft;
	bool moveRight;
	bool dropSoft;
	bool dropHard;
	bool rotateLeft; // More specifically, counter-clockwise.
	bool rotateRight; // More specifically, clockwise.
	bool crush;
	bool hold; // You never know.

} PyramisInputs;

extern void PyramisInputsSetDefaultVariables(PyramisInputs* inputs);