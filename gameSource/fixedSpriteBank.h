#ifndef FIXED_SPRITE_BANK_INCLUDED
#define FIXED_SPRITE_BANK_INCLUDED


#include "minorGems/game/doublePair.h"


void initSpriteBank();

void freeSpriteBank();


enum spriteID { riseMarker = 0,
                riseIcon,
                crosshair,
                enterCrosshair,
                powerUpSlot,
                powerUpBorder,
                powerUpEmpty,
                endSpriteID };                


void drawSprite( spriteID inID, doublePair inCenter );


#endif
