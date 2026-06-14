// (C) 2026 Daniel Sosa

#include "PyramisRules.h"

void PyramisRulesSetDefaultVariables(PyramisRules* rules)
{
	rules->dropSpeedGravity = 0.25f;
	rules->dropSpeedSoft = 30;
	rules->dropLockDelay = 0.5f;
	rules->dasDelay = 0.166f;
	rules->dasRate = 0.016f;
	rules->lockEntryDelay = 0.5f;
	rules->rotate180Window = 0.05f;
	rules->crushAnimationLength = 0.5f;
	rules->topOutShakeLength = 1;

	rules->speedAndScorePercent = 100;
	rules->dropSoftDistancePerPoint = 0.5f;
	rules->dropHardDistancePerPoint = 0.25f;
	rules->maxScore = 99999999;

	rules->RNGInitialSeed = 0;
	rules->RNGMaxColors = 3;

	rules->pyramidWrapX = false;
	rules->allowHoldButton = false;
}