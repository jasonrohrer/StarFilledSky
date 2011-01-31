#include "minorGems/game/doublePair.h"
#include "fixedSpriteBank.h"


void initTipDisplay();


void freeTipDisplay();



void drawTipDisplay( doublePair inScreenCenter );



// can start tip in a partially-advanced state (to prevent it from lingering
// after it stops being re-triggered)
// 0.5 is the fade-out point 
void triggerTip( spriteID inPowerType, float inStartingProgress = 0 );
