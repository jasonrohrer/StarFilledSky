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
                powerUpHeart,
                powerUpBulletSize,
                powerUpRapidFire,
                powerUpBulletSpeed,
                powerUpAccuracy,
                powerUpSpread,
                powerUpHeatSeek,
                powerUpBulletDistance,
                powerUpBounce,
                powerUpExplode,
                enemyBehaviorFollow,
                enemyBehaviorDodge,
                enemyBehaviorFast,
                enemyBehaviorRandom,
                enemyBehaviorCircle,
                endSpriteID };                

extern int firstPowerUpID;
extern int lastPowerUpID;

extern int firstBehaviorID;
extern int lastBehaviorID;



void drawSprite( spriteID inID, doublePair inCenter );


#endif
