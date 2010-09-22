#include "minorGems/game/doublePair.h"


void initSpriteBank();

void freeSpriteBank();


enum spriteID { riseMarker = 0,
                endSpriteID };                


void drawSprite( spriteID inID, doublePair inCenter );
