#include "minorGems/game/doublePair.h"
#include "PowerUpSet.h"

void initSetTipDisplay();


void freeSetTipDisplay();



void drawSetTipDisplay( doublePair inScreenCenter );



// inSet can be destroyed by caller immediately after this call returns
void triggerSetTip( PowerUpSet *inSet, char inUnderMiddleBracket=false,
                    char inFadeIn=false );



// this call implimented by game.cpp file
void triggerCurrentPlayerSetTip();
