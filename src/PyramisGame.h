// (C) 2026 Daniel Sosa

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <malloc.h>
#include <stdio.h>

#include "PyramisBlock.h"
#include "PyramisConstants.h"
#include "PyramisPyramidResults.h"
#include "PyramisRNG.h"
#include "PyramisRules.h"
#include "PyramisInputs.h"

#define PYRAMIS_FAILSAFE_MAX_TRIES 64

#define PYRAMIS_DROP_DEFAULT_X 3
#define PYRAMIS_DROP_DEFAULT_Y 10.25f // From the bottom of the grid.

// The size of the next queue. (This isn't necessarily how many blocks the player can see.)
#define PYRAMIS_NEXT_QUEUE_SIZE 8

enum PyramisGameStates
{
    PYRAMIS_GAME_STATE_BLANK = 0, // Recommended for implementing custom behaviors in your game.
    PYRAMIS_GAME_STATE_DROP = 1, // In this state, the player is actively moving a block to place it in the grid.
    PYRAMIS_GAME_STATE_LOCK = 2, // In this state, the block that the player just dropped is currently locking into place.
    PYRAMIS_GAME_STATE_CRUSH = 3,
    PYRAMIS_GAME_STATE_CRUSH_GRAVITY = 4,
    PYRAMIS_GAME_STATE_CRUSH_MINI_PYRAMID = 5,
    PYRAMIS_GAME_STATE_TOP_OUT_SHAKE = 6,
    PYRAMIS_GAME_STATE_TOP_OUT_LOSE = 7,
    PYRAMIS_GAME_STATE_MAX_OUT_WIN = 8,
};

// This is effectively a bool, but we're using an enum for readability.
enum PyramisDASDirections
{
    PYRAMIS_DAS_LEFT = 0,
    PYRAMIS_DAS_RIGHT = 1,
};

// This struct represents an individual game of Pyramis.
typedef struct PyramisGame
{
    // The main grid of blocks that is played on.
    PyramisBlock playfield[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y];
    
    // The block in slot 0 is the one the player will receive next.
    PyramisBlock nextQueue[PYRAMIS_NEXT_QUEUE_SIZE];
    uint_fast32_t score;
    uint_fast32_t level; // Normally, gameplay gets faster as this increases.

    /// CRUSH STUFF

    uint_fast8_t pyramids;
    bool isCrushQueued; // If true, pyramids will be crushed once blocks are finished dropping.
    unsigned int crushScoreBase;
    unsigned int crushPyramids;
    unsigned int crushCombo;
    unsigned int crushPyramidsTotal;
    bool wasCrushPressed;

    PyramisRules rules;
    PyramisInputs inputs; // Your program should change this data before calling the Update function.

    /// DROP BLOCK

    PyramisBlock dropBlock;
    int dropBlockPositionX;
    float dropBlockPositionY;
    int dropBlockRotation; // 0 = default rotation, each number afterwards is another clockwise 90 degree rotation. Max is 3.
    float dropManualDistanceTraveled; // Used to determine score for soft and hard drop.
    float dropLockDelayTimer; // If this reaches the dropLockDelay specified in the PyramisRules, the block will lock into place.
    bool wasDropHardPressed;
    bool wasRotateLeftPressed;
    bool wasRotateRightPressed;
    float rotate180Window; // Amount of time player is given to press a second rotation button to rotate a block 180 degrees.
    int rotate180Start; // The starting rotation of an attempted 180 degree rotation.

    /// DAS (DELAYED AUTO SHIFT)

    bool wasMoveButtonPressed;
    bool dasDelayOvercome;
    bool dasDirection;
    float dasForce;

    /// GAME STATE

    uint_fast8_t state;
    bool isNewState;
    float stateDuration; // How long the current state has lasted, in seconds.

    /// RNG

    PyramisRNG rng;

} PyramisGame;

// This returns NULL if it somehow fails.
extern PyramisGame* PyramisGameCreate();

extern void PyramisGameSetState(PyramisGame* game, uint_fast8_t newState);

extern void PyramisGameResetPlayfield(PyramisGame* game);
extern void PyramisGameResetNextQueue(PyramisGame* game);

extern void PyramisGameResetDropBlock(PyramisGame* game);
extern PyramisBlock PyramisGameGetDropBlockWithRotationApplied(PyramisGame* game);
extern void PyramisGameApplyDropBlockToPlayfield(PyramisGame* game, PyramisBlock playfield[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y]);
extern int PyramisGameGetDropBlockFurthestDown(PyramisGame* game);
extern int PyramisGameGetDropBlockFurthestLeft(PyramisGame* game);
extern int PyramisGameGetDropBlockFurthestRight(PyramisGame* game);
extern bool PyramisGameIsMoveButtonPressed(PyramisGame* game);
extern void PyramisGameMoveDropBlockInDASDirection(PyramisGame* game, int units);
extern void PyramisGameDropHorizontalMovement(PyramisGame* game);
extern void PyramisGameDropBlockTick(PyramisGame* game, float delta);
extern void PyramisGameTryToStartDrop(PyramisGame* game);

extern void PyramisGameResetDAS(PyramisGame* game);
extern void PyramisGameUpdateDAS(PyramisGame* game, float delta);

extern int PyramisGameConvertY(PyramisGame* game, int original);
extern PyramisPyramidResults PyramisGameCheckPyramids(PyramisGame* game, PyramisBlock gridToCheck[PYRAMIS_GRID_SIZE_X][PYRAMIS_GRID_SIZE_Y]);

extern uint32_t PyramisGameGetCrushScore(PyramisGame* game);
extern void PyramisGameCrushFinish(PyramisGame* game);
extern void PyramisGameDecideNextCrushStep(PyramisGame* game);

extern void PyramisGamePredictDropPyramids(PyramisGame* game);

extern void PyramisGameRunStateMachineTick(PyramisGame* game, float delta);

extern void PyramisGameCheckAndApplyPyramidsOnMainPlayfield(PyramisGame* game);

extern void PyramisGameSetVariablesForNewGame(PyramisGame* game);

extern void PyramisGameUpdate(PyramisGame* game, float delta);

extern void PyramisGameNormalModeUpdateRules(PyramisGame* game);

extern void PyramisGameFree(PyramisGame* game);

#ifdef __cplusplus
}
#endif