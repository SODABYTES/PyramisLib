// (C) 2026 Daniel Sosa

#include "PyramisGame.h"

int PyramisCeilOfFloat(float number)
{
	int result = (int)number;
	if (number > result)
	{
		result += 1;
	}
	return result;
}

PyramisGame* PyramisGameCreate()
{
	PyramisGame* result = malloc(sizeof(PyramisGame));
	if (!result)
	{
		return NULL;
	}

	PyramisGameResetPlayfield(result);
	PyramisGameResetNextQueue(result);

	result->score = 0;
	result->level = 0;

	result->pyramids = 0;

	result->isCrushQueued = false;
	result->crushScoreBase = 0;
	result->crushPyramids = 0;
	result->crushCombo = 0;
	result->crushPyramidsTotal = 0;
	result->wasCrushPressed = false;

	PyramisRulesSetDefaultVariables(&result->rules);
	PyramisInputsSetDefaultVariables(&result->inputs);

	PyramisBlockSetDefaultVariables(&result->dropBlock);
	PyramisGameResetDropBlock(result);
	result->wasDropHardPressed = false;
	result->wasRotateLeftPressed = false;
	result->wasRotateRightPressed = false;
	result->rotate180Window = 0;
	result->rotate180Start = 0;

	result->state = PYRAMIS_GAME_STATE_BLANK;
	result->isNewState = true;
	result->stateDuration = 0;

	PyramisRNGInit(&result->rng, 0, 3);

	return result;
}

bool PyramisGameIsFirstFrameOfState(PyramisGame* game)
{
	return (game->stateDuration == 0);
}

void PyramisGameDecideNextActionAfterLock(PyramisGame* game)
{
	if (game->isCrushQueued)
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH);
	}
	else
	{
		PyramisGameTryToStartDrop(game);
	}
}

void PyramisGameSetState(PyramisGame* game, uint_fast8_t newState)
{
	if (game->state != newState)
	{
		game->stateDuration = 0;
		game->isNewState = true;
		game->state = newState;
	}
}

void PyramisGameResetPlayfield(PyramisGame* game)
{
	if (!game)
	{
		return;
	}

	for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			PyramisBlockSetDefaultVariables(&game->playfield[x][y]);
		}
	}
}

void PyramisGameResetNextQueue(PyramisGame* game)
{
	for (int i = 0; i < PYRAMIS_NEXT_QUEUE_SIZE; i++)
	{
		PyramisBlockSetDefaultVariables(&game->nextQueue[i]);
	}
}

void PyramisGameResetDropBlock(PyramisGame* game)
{
	game->dropBlockPositionX = PYRAMIS_DROP_DEFAULT_X;
	game->dropBlockPositionY = PYRAMIS_GRID_SIZE_Y - PYRAMIS_DROP_DEFAULT_Y;
	game->dropBlockRotation = 0;
	game->dropManualDistanceTraveled = 0;
	game->dropLockDelayTimer = 0;
}

PyramisBlock PyramisGameGetDropBlockWithRotationApplied(PyramisGame* game)
{
	PyramisBlock finalBlock;
	PyramisBlockSetDefaultVariables(&finalBlock);

	finalBlock.empty = false;
	
	switch (game->dropBlockRotation)
	{
	case 0:
		finalBlock.triangleA.color = game->dropBlock.triangleA.color;
		finalBlock.triangleB.color = game->dropBlock.triangleB.color;
		finalBlock.rotated = false;
		break;
	case 1:
		finalBlock.triangleA.color = game->dropBlock.triangleA.color;
		finalBlock.triangleB.color = game->dropBlock.triangleB.color;
		finalBlock.rotated = true;
		break;
	case 2:
		finalBlock.triangleA.color = game->dropBlock.triangleB.color;
		finalBlock.triangleB.color = game->dropBlock.triangleA.color;
		finalBlock.rotated = false;
		break;
	case 3:
		finalBlock.triangleA.color = game->dropBlock.triangleB.color;
		finalBlock.triangleB.color = game->dropBlock.triangleA.color;
		finalBlock.rotated = true;
		break;
	}

	return finalBlock;
}

void PyramisGameApplyDropBlockToPlayfield(PyramisGame* game, PyramisBlock playfield[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y])
{
	playfield[game->dropBlockPositionX][PyramisGameGetDropBlockFurthestDown(game)] = PyramisGameGetDropBlockWithRotationApplied(game);
}

int PyramisGameGetDropBlockFurthestDown(PyramisGame* game)
{
	int result = (int)game->dropBlockPositionY;
	if (result < 0) // The drop block probably shouldn't be this high to begin with but it's better to be safe than sorry
	{
		result = 0;
	}
	bool finished = false;
	while (!finished)
	{
		if (result >= PYRAMIS_GRID_SIZE_Y - 1)
		{
			finished = true;
		}
		else if (game->playfield[game->dropBlockPositionX][result + 1].empty)
		{
			result = result + 1;
		}
		else
		{
			finished = true;
		}
	}
	return result;
}

int PyramisGameGetDropBlockFurthestLeft(PyramisGame* game)
{
	int result = game->dropBlockPositionX;
	int positionY = PyramisCeilOfFloat(game->dropBlockPositionY);
	bool finished = false;
	while (!finished)
	{
		if (result <= 0)
		{
			finished = true;
		}
		else if (game->playfield[result - 1][positionY].empty)
		{
			result = result - 1;
		}
		else
		{
			finished = true;
		}
	}
	return result;
}

int PyramisGameGetDropBlockFurthestRight(PyramisGame* game)
{
	int result = game->dropBlockPositionX;
	int positionY = PyramisCeilOfFloat(game->dropBlockPositionY);
	bool finished = false;
	while (!finished)
	{
		if (result >= PYRAMIS_GRID_SIZE_X - 1)
		{
			finished = true;
		}
		else if (game->playfield[result + 1][positionY].empty)
		{
			result = result + 1;
		}
		else
		{
			finished = true;
		}
	}
	return result;
}

bool PyramisGameIsMoveButtonPressed(PyramisGame* game)
{
	return (game->inputs.moveLeft && !game->inputs.moveRight) || (game->inputs.moveRight && !game->inputs.moveLeft);
}

void PyramisGameMoveDropBlockInDASDirection(PyramisGame* game, int units)
{
	if (game->dasDirection == PYRAMIS_DAS_LEFT)
	{
		game->dropBlockPositionX -= units;
	}
	else
	{
		game->dropBlockPositionX += units;
	}
}

void PyramisGameDropHorizontalMovement(PyramisGame* game)
{
	// Movement in response to the button immediately being pressed
	if (PyramisGameIsMoveButtonPressed(game) && !game->wasMoveButtonPressed)
	{
		PyramisGameMoveDropBlockInDASDirection(game, 1);
	}

	// DAS movement
	if (!game->dasDelayOvercome)
	{
		if (game->dasForce >= game->rules.dasDelay)
		{
			game->dasDelayOvercome = true;
			PyramisGameMoveDropBlockInDASDirection(game, 1);
			game->dasForce -= game->rules.dasDelay;
		}
	}
	if (game->dasDelayOvercome)
	{
		if (game->dasForce >= game->rules.dasRate)
		{
			int unitsToMove = (int)(game->dasForce / game->rules.dasRate);
			PyramisGameMoveDropBlockInDASDirection(game, unitsToMove);
			game->dasForce -= (game->rules.dasRate * unitsToMove);
		}
	}
}

void PyramisGameDropBlockTick(PyramisGame* game, float delta)
{
	// BLOCK ROTATION
	// Fun fact: This is basically the only part of the code that's been copied verbatim from the
	// original Godot prototype because it was already basically perfect. Way to go, past me!

	int originalRotation = game->dropBlockRotation; // If the rotation changes it warrants a recalculation of the drop block preview.

	// Rotate left
	if (game->inputs.rotateLeft && !game->wasRotateLeftPressed)
	{
		if (game->rotate180Window <= 0) // Normal rotation
		{
			game->rotate180Start = game->dropBlockRotation;
			game->dropBlockRotation -= 1;
			game->rotate180Window = game->rules.rotate180Window;
		}
		else // 180 rotation
		{
			game->dropBlockRotation = game->rotate180Start + 2;
			game->rotate180Window = 0;
		}
	}

	// Rotate right
	if (game->inputs.rotateRight && !game->wasRotateRightPressed)
	{
		if (game->rotate180Window <= 0) // Normal rotation
		{
			game->rotate180Start = game->dropBlockRotation;
			game->dropBlockRotation += 1;
			game->rotate180Window = game->rules.rotate180Window;
		}
		else // 180 rotation
		{
			game->dropBlockRotation = game->rotate180Start + 2;
			game->rotate180Window = 0;
		}
	}

	// Limit the value of the new rotation
	while (game->dropBlockRotation < 0)
	{
		game->dropBlockRotation += 4;
	}
	game->dropBlockRotation = game->dropBlockRotation % 4;

	// Precalculate the furthest left and right collisions
	int furthestLeft = PyramisGameGetDropBlockFurthestLeft(game);
	int furthestRight = PyramisGameGetDropBlockFurthestRight(game);

	// Left and right movement
	int originalPositionX = game->dropBlockPositionX; // If the position changes it warrants a recalculation of the drop block preview.
	PyramisGameDropHorizontalMovement(game);

	// Apply the calculated collision detection
	if (game->dropBlockPositionX < furthestLeft)
	{
		game->dropBlockPositionX = furthestLeft;
	}
	else if (game->dropBlockPositionX > furthestRight)
	{
		game->dropBlockPositionX = furthestRight;
	}

	// Precalculate the landing position
	float startingHeight = game->dropBlockPositionY;
	int landingHeight = PyramisGameGetDropBlockFurthestDown(game);
	bool dropSoftScoreQualify = false;

	// Check if the player hard dropped, because if so, none of the rest of this matters
	if (game->inputs.dropHard && !game->wasDropHardPressed)
	{
		// put a function in this line that clears the preview?
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_LOCK);
		return;
	}
	// If the player didn't hard drop, we can continue and apply normal / soft drop gravity...

	// Apply gravity
	if (game->inputs.dropSoft && (game->rules.dropSpeedSoft > game->rules.dropSpeedGravity))
	{
		game->dropBlockPositionY += game->rules.dropSpeedSoft * delta;
		dropSoftScoreQualify = true;
	}
	else
	{
		game->dropBlockPositionY += game->rules.dropSpeedGravity * delta;
	}

	// Apply precalculated landing collision and increment lock delay timer if the block is on the floor
	if (game->dropBlockPositionY >= landingHeight)
	{
		game->dropBlockPositionY = (float)landingHeight;
		game->dropLockDelayTimer += delta;
	}
	else
	{
		game->dropLockDelayTimer = 0;
	}

	// Reset lock delay timer if the block moved down even a little bit ("step reset" in Gamer terms)
	if (game->dropBlockPositionY > startingHeight)
	{
		game->dropLockDelayTimer = 0;
	}

	// Apply score if soft dropping qualifies for it
	if (dropSoftScoreQualify)
	{
		game->dropManualDistanceTraveled += (game->dropBlockPositionY - startingHeight);
		if (game->dropManualDistanceTraveled >= game->rules.dropSoftDistancePerPoint)
		{
			unsigned int reward = (int)(game->dropManualDistanceTraveled / game->rules.dropSoftDistancePerPoint);
			game->score += reward;
			game->dropManualDistanceTraveled -= (game->rules.dropSoftDistancePerPoint * reward);
		}
	}
	else
	{
		game->dropManualDistanceTraveled = 0;
	}

	// If lock delay timer reached (or soft dropping), enter the lock state
	if ((game->dropLockDelayTimer >= game->rules.dropLockDelay || game->inputs.dropSoft) && game->dropBlockPositionY >= landingHeight)
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_LOCK);
	}

	// Get pyramid preview
	if ((originalRotation != game->dropBlockRotation) || (originalPositionX != game->dropBlockPositionX))
	{
		PyramisGamePredictDropPyramids(game);
	}
}

void PyramisGameTryToStartDrop(PyramisGame* game)
{
	if (game->score >= game->rules.maxScore)
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_MAX_OUT_WIN);
		return;
	}

	if (game->playfield[PYRAMIS_DROP_DEFAULT_X][PYRAMIS_GRID_SIZE_Y - (int)PYRAMIS_DROP_DEFAULT_Y].empty)
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_DROP);
	}
	else
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_TOP_OUT_SHAKE);
	}
}

void PyramisGameCrushMainGrid(PyramisGame* game)
{
	for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			bool emptyThisBlock = false;
			if (game->playfield[x][y].triangleA.pyramidValue > 0)
			{
				emptyThisBlock = true;
				game->crushScoreBase += PyramisTriangleGetScoreFromPyramidValue(game->playfield[x][y].triangleA);
			}
			if (game->playfield[x][y].triangleB.pyramidValue > 0)
			{
				emptyThisBlock = true;
				game->crushScoreBase += PyramisTriangleGetScoreFromPyramidValue(game->playfield[x][y].triangleB);
			}
			if (emptyThisBlock)
			{
				game->playfield[x][y].empty = true;
			}
		}
	}
}

void PyramisGameResetCrushScore(PyramisGame* game)
{
	game->crushScoreBase = 0;
	game->crushPyramids = game->crushPyramidsTotal;
	game->crushCombo = 0;
}

uint32_t PyramisGameGetCrushScore(PyramisGame* game)
{
	uint32_t result = 0;

	uint64_t working = game->crushScoreBase * game->crushPyramids * game->crushCombo;
	working = working * game->rules.speedAndScorePercent;
	working = working / 100;

	result = (uint32_t)working;
	return result;
}

void PyramisGameCrushFinish(PyramisGame* game)
{
	game->score += PyramisGameGetCrushScore(game);
	PyramisGameResetCrushScore(game);
	game->isCrushQueued = false;
	game->level += 1;

	PyramisGameTryToStartDrop(game);
}

void PyramisGameDecideNextCrushStep(PyramisGame* game)
{
	if (game->pyramids > 0)
	{
		PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH);
	}
	else
	{
		bool areThereMiniPyramids = false;
		PyramisGameCheckAndApplyPyramidsOnMainPlayfield(game);
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
			{
				if (game->playfield[x][y].miniPyramid)
				{
					areThereMiniPyramids = true;
				}
			}
		}
		if (areThereMiniPyramids)
		{
			PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH_MINI_PYRAMID);
		}
		else
		{
			PyramisGameCrushFinish(game);
		}
	}
}

void PyramisGamePredictDropPyramids(PyramisGame* game)
{
	// First we create a copy of the main playfield.
	PyramisBlock copy[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y];

	for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
		{
			PyramisBlockCopy(&copy[x][y], &game->playfield[x][y]);
		}
	}

	// Next we apply the drop block to our copied grid
	PyramisGameApplyDropBlockToPlayfield(game, copy);

	// Now we can see if that dropped block gave us any pyramids
	PyramisPyramidResults newResults = PyramisGameCheckPyramids(game, copy);

	// Finally, apply the results' pyramid values into the main grid's predict values
	for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
		{
			game->playfield[x][y].triangleA.pyramidValuePreview = newResults.playfield[x][y].triangleA.pyramidValue;
			game->playfield[x][y].triangleB.pyramidValuePreview = newResults.playfield[x][y].triangleB.pyramidValue;
			game->playfield[x][y].rotated = newResults.playfield[x][y].rotated;
		}
	}
}

void PyramisGameRunStateMachineTick(PyramisGame* game, float delta)
{
	// General state machine stuff
	if (game->isNewState)
	{
		game->stateDuration = 0;
		game->isNewState = false;
	}
	else
	{
		game->stateDuration += delta;
	}

	if (game->inputs.crush && (!game->wasCrushPressed) && game->pyramids > 0)
	{
		game->isCrushQueued = true;
	}

	// Individual state behaviors
	switch (game->state)
	{
	case PYRAMIS_GAME_STATE_DROP:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			PyramisGameResetDropBlock(game);

			float dasLimit;
			if (game->dasDelayOvercome)
			{
				dasLimit = game->rules.dasRate;
			}
			else
			{
				dasLimit = game->rules.dasDelay;
			}

			if (game->dasForce > dasLimit)
			{
				game->dasForce = dasLimit;
			}

			// IRS
			if (game->inputs.rotateLeft && game->inputs.rotateRight)
			{
				game->dropBlockRotation = 2;
			}
			else if (game->inputs.rotateLeft)
			{
				game->dropBlockRotation = 3;
			}
			else if (game->inputs.rotateRight)
			{
				game->dropBlockRotation = 1;
			}

			// Get block data from the next queue, and then generate a new block for the next queue
			game->dropBlock = game->nextQueue[0];
			for (int i = 0; i < PYRAMIS_NEXT_QUEUE_SIZE - 1; i++)
			{
				game->nextQueue[i] = game->nextQueue[i + 1];
			}
			game->nextQueue[PYRAMIS_NEXT_QUEUE_SIZE - 1] = PyramisRNGGenerateBlock(&game->rng);

			PyramisGamePredictDropPyramids(game);
		}

		PyramisGameDropBlockTick(game, delta);
		break;

	case PYRAMIS_GAME_STATE_LOCK:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			int landingPosition = PyramisGameGetDropBlockFurthestDown(game);

			// Calculate the score earned if hard dropped by checking the distance the block was at before it locked.
			// If the block was already on the floor when locking then this should earn no points.

			game->dropManualDistanceTraveled = landingPosition - game->dropBlockPositionY;
			if (game->dropManualDistanceTraveled >= game->rules.dropHardDistancePerPoint)
			{
				unsigned int reward = (int)(game->dropManualDistanceTraveled / game->rules.dropHardDistancePerPoint);
				game->score += reward;
				game->dropManualDistanceTraveled = 0;
			}

			// Apply the block to the playfield
			PyramisGameApplyDropBlockToPlayfield(game, game->playfield);

			// Move the drop block position to the final spot. This doesn't actually DO anything but it might be convenient for whatever component is used to display the drop block.
			game->dropBlockPositionY = (float)landingPosition;

			PyramisGameCheckAndApplyPyramidsOnMainPlayfield(game);
		}
		if (game->stateDuration >= game->rules.lockEntryDelay)
		{
			PyramisGameDecideNextActionAfterLock(game);
		}
		break;

	case PYRAMIS_GAME_STATE_CRUSH:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			PyramisGameCrushMainGrid(game);

			game->crushPyramids += game->pyramids;
			game->crushPyramidsTotal += game->pyramids;
			game->crushCombo += 1;
			PyramisGameCheckAndApplyPyramidsOnMainPlayfield(game);
		}
		if (game->stateDuration >= game->rules.crushAnimationLength)
		{
			PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH_GRAVITY);
		}
		break;

	case PYRAMIS_GAME_STATE_CRUSH_GRAVITY:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			bool didAnythingChange = false;

			// Create old grid for comparison
			PyramisBlock oldPlayfield[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y];
			for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
			{
				for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
				{
					PyramisBlockCopy(&oldPlayfield[x][y], &game->playfield[x][y]);
				}
			}

			// Gravity
			for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
			{
				for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
				{
					if (game->playfield[x][y].empty)
					{
						for (int i = 0; i < y; i++)
						{
							game->playfield[x][y - i] = game->playfield[x][y - 1 - i];
							if (y - i == 1)
							{
								game->playfield[x][y - 1 - i].empty = true;
							}
						}
					}
				}
			}

			// See if anything changed
			for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
			{
				for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
				{
					if (game->playfield[x][y].empty != oldPlayfield[x][y].empty)
					{
						didAnythingChange = true;
					}
				}
			}
			PyramisGameCheckAndApplyPyramidsOnMainPlayfield(game);
			if (!didAnythingChange)
			{
				PyramisGameDecideNextCrushStep(game);
			}
		}
		if (game->stateDuration >= game->rules.crushAnimationLength)
		{
			PyramisGameDecideNextCrushStep(game);
		}
		break;

	case PYRAMIS_GAME_STATE_CRUSH_MINI_PYRAMID:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
			{
				for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
				{
					if (game->playfield[x][y].miniPyramid)
					{
						game->playfield[x][y].empty = true;
					}
				}
			}
		}
		if (game->stateDuration >= game->rules.crushAnimationLength)
		{
			PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH_GRAVITY);
		}
		break;

	case PYRAMIS_GAME_STATE_TOP_OUT_SHAKE:
		if (PyramisGameIsFirstFrameOfState(game))
		{
			if (game->pyramids > 0)
			{
				game->isCrushQueued = true;
			}
		}
		if (game->stateDuration >= game->rules.topOutShakeLength)
		{
			if (game->isCrushQueued)
			{
				PyramisGameSetState(game, PYRAMIS_GAME_STATE_CRUSH);
			}
			else
			{
				PyramisGameSetState(game, PYRAMIS_GAME_STATE_TOP_OUT_LOSE);
			}
		}
	}

	game->wasMoveButtonPressed = PyramisGameIsMoveButtonPressed(game);
	game->wasDropHardPressed = game->inputs.dropHard;
	game->wasRotateLeftPressed = game->inputs.rotateLeft;
	game->wasRotateRightPressed = game->inputs.rotateRight;
	game->wasCrushPressed = game->inputs.crush;

	game->rotate180Window -= delta;
}

void PyramisGameResetDAS(PyramisGame* game)
{
	game->wasMoveButtonPressed = false; // This allows the block to move if switching directions in a single frame. a little white lie never hurt anybody...
	game->dasDelayOvercome = false;
	game->dasForce = 0;
}

void PyramisGameUpdateDAS(PyramisGame* game, float delta)
{
	bool moving = false;

	if (game->inputs.moveLeft && !game->inputs.moveRight) // Is left being pressed?
	{
		moving = true;
		if (game->dasDirection == PYRAMIS_DAS_RIGHT)
		{
			PyramisGameResetDAS(game);
		}
		game->dasDirection = PYRAMIS_DAS_LEFT;
	}
	else if (game->inputs.moveRight && !game->inputs.moveLeft) // Is right being pressed?
	{
		moving = true;
		if (game->dasDirection == PYRAMIS_DAS_LEFT)
		{
			PyramisGameResetDAS(game);
		}
		game->dasDirection = PYRAMIS_DAS_RIGHT;
	}
	else
	{
		PyramisGameResetDAS(game);
	}

	if (moving)
	{
		game->dasForce += delta;
	}
}

int PyramisGameConvertX(PyramisGame* game, int original)
{
	// If X wraparound isn't enabled then just return the original value.
	if (!game->rules.pyramidWrapX)
	{
		return original;
	}

	// Assuming X wraparound IS enabled then we just continue with the conversion.
	int result = original;

	while (result < 0)
	{
		result += PYRAMIS_GRID_SIZE_X;
	}
	result = result % PYRAMIS_GRID_SIZE_X;

	return result;
}

int PyramisGameConvertY(PyramisGame* game, int original)
{
	// There's no Y wraparound rule, but it's better to anticipate for the possibility by having a function, right?
	return original;
}

bool PyramisIsCoordinateValid(int x, int y)
{
	// We assume that the coordinate is valid until we find a problem with it.
	bool result = true;

	if (x < 0 || x >= PYRAMIS_GRID_SIZE_X)
	{
		result = false;
	}

	if (y < 0 || y >= PYRAMIS_GRID_SIZE_Y)
	{
		result = false;
	}

	return result;
}

PyramisPyramidResults PyramisGameCheckPyramids(PyramisGame* game, PyramisBlock gridToCheck[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y])
{
	// Here we go. I'm writing the pyramid detection code again. Hopefully for the last time

	PyramisPyramidResults result;

	// Copy the grid that was supplied to us.
	result.pyramids = 0;
	for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			PyramisBlockCopy(&result.playfield[x][y], &gridToCheck[x][y]);
			result.playfield[x][y].triangleA.pyramidValue = 0;
			result.playfield[x][y].triangleB.pyramidValue = 0;
			result.playfield[x][y].miniPyramid = false;
		}
	}

	for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			// Use the current coordinate as a base for the pyramid detection.
			if (!PyramisBlockQualifiesForPyramid(&result.playfield[x][y]))
			{
				continue;
			}

			/*
			* Standard pyramid detection. Only unrotated blocks should be checked for pyramids.
			* After all, a pyramid cap is always formed by 1 rotated block and 1 unrotated block.
			* We don't need to check both.
			* (Also, this doesn't "continue" in case we want to add more methods of pyramid
			* detection eventually, such as Shiloh's idea.)
			*/

			if (!result.playfield[x][y].rotated)
			{
				unsigned int color;
				int capCoordinate;

				// Check up.
				color = result.playfield[x][y].triangleA.color;
				capCoordinate = PyramisGameConvertY(game, y - 1);
				if (PyramisIsCoordinateValid(x, capCoordinate))
				{
					// Check for mini pyramid.
					bool rotationCheck = result.playfield[x][capCoordinate].rotated;
					bool colorCheck = result.playfield[x][capCoordinate].triangleB.color == color;
					bool qualifyCheck = PyramisBlockQualifiesForPyramid(&result.playfield[x][capCoordinate]);
					
					if (rotationCheck && colorCheck && qualifyCheck)
					{
						result.playfield[x][y].miniPyramid = true;
						result.playfield[x][capCoordinate].miniPyramid = true;

						bool longer = true; // If true, we extend the check further to see if the pyramid is any bigger.
						int pyramidSize = 0;

						while (longer)
						{
							longer = false;
							int convertedX = PyramisGameConvertX(game, x - (pyramidSize + 1));
							int convertedYUp = PyramisGameConvertY(game, y - (pyramidSize + 2));
							int convertedYDown = PyramisGameConvertY(game, y + (pyramidSize + 1));

							if (PyramisIsCoordinateValid(convertedX, convertedYUp) && PyramisIsCoordinateValid(convertedX, convertedYDown))
							{
								PyramisBlock blockUp = result.playfield[convertedX][convertedYUp];
								PyramisBlock blockDown = result.playfield[convertedX][convertedYDown];
								
								bool checkUp = (blockUp.rotated && blockUp.triangleB.color == color && PyramisBlockQualifiesForPyramid(&blockUp));
								bool checkDown = (!blockDown.rotated && blockDown.triangleA.color == color && PyramisBlockQualifiesForPyramid(&blockDown));

								if (checkUp && checkDown)
								{
									longer = true;
									pyramidSize += 1;
								}
							}
						}
						for (int i = 0; i < pyramidSize + 1; i++)
						{
							int convertedX = PyramisGameConvertX(game, x - i);
							int convertedYUp = PyramisGameConvertY(game, y - (i + 1));
							int convertedYDown = PyramisGameConvertY(game, y + i);
							result.playfield[convertedX][convertedYUp].triangleB.pyramidValue += pyramidSize;
							result.playfield[convertedX][convertedYDown].triangleA.pyramidValue += pyramidSize;
						}
						if (pyramidSize > 0)
						{
							result.pyramids += 1;
						}
					}
				}

				// Check down.
				color = result.playfield[x][y].triangleB.color;
				capCoordinate = PyramisGameConvertY(game, y + 1);
				if (PyramisIsCoordinateValid(x, capCoordinate))
				{
					// Check for mini pyramid.
					bool rotationCheck = result.playfield[x][capCoordinate].rotated;
					bool colorCheck = result.playfield[x][capCoordinate].triangleA.color == color;
					bool qualifyCheck = PyramisBlockQualifiesForPyramid(&result.playfield[x][capCoordinate]);

					if (rotationCheck && colorCheck && qualifyCheck)
					{
						//lala
						result.playfield[x][y].miniPyramid = true;
						result.playfield[x][capCoordinate].miniPyramid = true;

						bool longer = true;
						int pyramidSize = 0;

						while (longer)
						{
							longer = false;
							int convertedX = PyramisGameConvertX(game, x + (pyramidSize + 1));
							int convertedYUp = PyramisGameConvertY(game, y - (pyramidSize + 1));
							int convertedYDown = PyramisGameConvertY(game, y + (pyramidSize + 2));

							if (PyramisIsCoordinateValid(convertedX, convertedYUp) && PyramisIsCoordinateValid(convertedX, convertedYDown))
							{
								PyramisBlock blockUp = result.playfield[convertedX][convertedYUp];
								PyramisBlock blockDown = result.playfield[convertedX][convertedYDown];
								
								bool checkUp = (!blockUp.rotated && blockUp.triangleB.color == color && PyramisBlockQualifiesForPyramid(&blockUp));
								bool checkDown = (blockDown.rotated && blockDown.triangleA.color == color && PyramisBlockQualifiesForPyramid(&blockDown));

								if (checkUp && checkDown)
								{
									longer = true;
									pyramidSize += 1;
								}
							}
						}
						for (int i = 0; i < pyramidSize + 1; i++)
						{
							int convertedX = PyramisGameConvertX(game, x + i);
							int convertedYUp = PyramisGameConvertY(game, y - i);
							int convertedYDown = PyramisGameConvertY(game, y + (i + 1));
							result.playfield[convertedX][convertedYUp].triangleB.pyramidValue += pyramidSize;
							result.playfield[convertedX][convertedYDown].triangleA.pyramidValue += pyramidSize;
						}
						if (pyramidSize > 0)
						{
							result.pyramids += 1;
						}
					}
				}

				// Check left.
				color = result.playfield[x][y].triangleA.color;
				capCoordinate = PyramisGameConvertX(game, x - 1);
				if (PyramisIsCoordinateValid(capCoordinate, y))
				{
					// Check mini pyramid.
					bool rotationCheck = result.playfield[capCoordinate][y].rotated;
					bool colorCheck = result.playfield[capCoordinate][y].triangleA.color == color;
					bool qualifyCheck = PyramisBlockQualifiesForPyramid(&result.playfield[capCoordinate][y]);
					if (rotationCheck && colorCheck && qualifyCheck)
					{
						result.playfield[x][y].miniPyramid = true;
						result.playfield[capCoordinate][y].miniPyramid = true;

						bool longer = true;
						int pyramidSize = 0;

						while (longer)
						{
							longer = false;
							int convertedY = PyramisGameConvertY(game, y - (pyramidSize + 1));
							int convertedXLeft = PyramisGameConvertX(game, x - (pyramidSize + 2));
							int convertedXRight = PyramisGameConvertX(game, x + (pyramidSize + 1));

							if (PyramisIsCoordinateValid(convertedXLeft, convertedY) && PyramisIsCoordinateValid(convertedXRight, convertedY))
							{
								PyramisBlock blockLeft = result.playfield[convertedXLeft][convertedY];
								PyramisBlock blockRight = result.playfield[convertedXRight][convertedY];

								bool checkLeft = (blockLeft.rotated && blockLeft.triangleA.color == color && PyramisBlockQualifiesForPyramid(&blockLeft));
								bool checkRight = (!blockRight.rotated && blockRight.triangleA.color == color && PyramisBlockQualifiesForPyramid(&blockRight));
								
								if (checkLeft && checkRight)
								{
									longer = true;
									pyramidSize += 1;
								}
							}
						}
						for (int i = 0; i < pyramidSize + 1; i++)
						{
							int convertedY = PyramisGameConvertY(game, y - i);
							int convertedXLeft = PyramisGameConvertX(game, x - (i + 1));
							int convertedXRight = PyramisGameConvertX(game, x + i);
							result.playfield[convertedXLeft][convertedY].triangleA.pyramidValue += pyramidSize;
							result.playfield[convertedXRight][convertedY].triangleA.pyramidValue += pyramidSize;
						}
						if (pyramidSize > 0)
						{
							result.pyramids += 1;
						}
					}
				}

				// Check right.
				color = result.playfield[x][y].triangleB.color;
				capCoordinate = PyramisGameConvertX(game, x + 1);
				if (PyramisIsCoordinateValid(capCoordinate, y))
				{
					//Check mini pyramid.
					bool rotationCheck = result.playfield[capCoordinate][y].rotated;
					bool colorCheck = result.playfield[capCoordinate][y].triangleB.color == color;
					bool qualifyCheck = PyramisBlockQualifiesForPyramid(&result.playfield[capCoordinate][y]);

					if (rotationCheck && colorCheck && qualifyCheck)
					{
						result.playfield[x][y].miniPyramid = true;
						result.playfield[capCoordinate][y].miniPyramid = true;

						bool longer = true;
						int pyramidSize = 0;

						while (longer)
						{
							longer = false;
							int convertedY = PyramisGameConvertY(game, y + (pyramidSize + 1));
							int convertedXLeft = PyramisGameConvertX(game, x - (pyramidSize + 1));
							int convertedXRight = PyramisGameConvertX(game, x + (pyramidSize + 2));

							if (PyramisIsCoordinateValid(convertedXLeft, convertedY) && PyramisIsCoordinateValid(convertedXRight, convertedY))
							{
								PyramisBlock blockLeft = result.playfield[convertedXLeft][convertedY];
								PyramisBlock blockRight = result.playfield[convertedXRight][convertedY];

								bool checkLeft = (!blockLeft.rotated && blockLeft.triangleB.color == color && PyramisBlockQualifiesForPyramid(&blockLeft));
								bool checkRight = (blockRight.rotated && blockRight.triangleB.color == color && PyramisBlockQualifiesForPyramid(&blockRight));

								if (checkLeft && checkRight)
								{
									longer = true;
									pyramidSize += 1;
								}
							}
						}
						for (int i = 0; i < pyramidSize + 1; i++)
						{
							int convertedY = PyramisGameConvertY(game, y + i);
							int convertedXLeft = PyramisGameConvertX(game, x - i);
							int convertedXRight = PyramisGameConvertX(game, x + (i + 1));
							result.playfield[convertedXLeft][convertedY].triangleB.pyramidValue += pyramidSize;
							result.playfield[convertedXRight][convertedY].triangleB.pyramidValue += pyramidSize;
						}
						if (pyramidSize > 0)
						{
							result.pyramids += 1;
						}
					}
				}
			}
		}
	}

	return result;
}

void PyramisGameCheckAndApplyPyramidsOnMainPlayfield(PyramisGame* game)
{
	PyramisPyramidResults newResults = PyramisGameCheckPyramids(game, game->playfield);

	for (int y = 0; y < PYRAMIS_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < PYRAMIS_GRID_SIZE_X; x++)
		{
			game->playfield[x][y].miniPyramid = newResults.playfield[x][y].miniPyramid;
			game->playfield[x][y].triangleA = newResults.playfield[x][y].triangleA;
			game->playfield[x][y].triangleB = newResults.playfield[x][y].triangleB;
		}
	}
	game->pyramids = newResults.pyramids;
}

void PyramisGameSetVariablesForNewGame(PyramisGame* game)
{
	// Scoring
	game->score = 0;
	game->level = 0;
	game->crushPyramidsTotal = 0;

	// Playfield
	PyramisGameResetPlayfield(game);

	// Next queue and RNG
	PyramisRNGInit(&game->rng, game->rules.RNGInitialSeed, game->rules.RNGMaxColors);
	PyramisGameResetNextQueue(game);
	for (int i = 0; i < PYRAMIS_NEXT_QUEUE_SIZE; i++)
	{
		game->nextQueue[i] = PyramisRNGGenerateBlock(&game->rng);
	}
}

void PyramisGameUpdate(PyramisGame* game, float delta)
{
	if (!game)
	{
		return;
	}

	PyramisGameNormalModeUpdateRules(game);

	PyramisGameUpdateDAS(game, delta);

	/// STATE MACHINE
	
	PyramisGameRunStateMachineTick(game, delta);
	unsigned int failsafe = 0;
	while (game->isNewState)
	{
		PyramisGameRunStateMachineTick(game, 0);
		failsafe++;
		if (failsafe >= PYRAMIS_FAILSAFE_MAX_TRIES)
		{
			game->stateDuration = 0;
			game->isNewState = false;
		}
	}
}

void PyramisGameNormalModeUpdateRules(PyramisGame* game)
{
	int gravityThreshold = 60;

	float newGravity = (game->level * (game->level / 10.0f) * 1.25f) + 0.25f;

	float speedMultiplier = game->rules.speedAndScorePercent / (float)100;

	newGravity *= speedMultiplier;

	game->rules.dropSpeedGravity = newGravity;

	float newDelayTime = 0.5f;
	if (newGravity > gravityThreshold)
	{
		newDelayTime = 0.5f - ((float)(newGravity - gravityThreshold) * 0.001f);
	}

	game->rules.dropLockDelay = newDelayTime;
	game->rules.crushAnimationLength = newDelayTime;

	float newEntryDelay = 0.5f;
	if (newDelayTime < newEntryDelay)
	{
		newEntryDelay = newDelayTime;
	}
	game->rules.lockEntryDelay = newEntryDelay;
}

void PyramisGameFree(PyramisGame* game)
{
	if (!game)
	{
		return;
	}

	free(game);
}