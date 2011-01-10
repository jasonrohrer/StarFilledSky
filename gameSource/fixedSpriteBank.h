#ifndef FIXED_SPRITE_BANK_INCLUDED
#define FIXED_SPRITE_BANK_INCLUDED


#include "minorGems/game/doublePair.h"
#include "minorGems/graphics/Color.h"


void initSpriteBank();

void freeSpriteBank();




// macro trick that allows us to define this list only once and use
// it in both the enum and to create the string array.
// Ensures consistency of both lists.

// found here:  
//    http://www.gamedev.net/community/forums/topic.asp?topic_id=260159

#ifdef F
  #error Macro "F" already defined
#endif

#define FIXED_SPRITE_NAMES \
    F(riseMarker), \
    F(riseIcon), \
    F(eye), \
    F(eyeLeft), \
    F(eyeSquint), \
    F(eyeLeftSquint), \
    F(eyesTogether), \
    F(eyesTogetherSquint), \
    F(crosshair), \
    F(enterCrosshair), \
    F(powerUpSlot), \
    F(powerUpBorder), \
    F(powerUpEmpty), \
    F(powerUpHeart), \
    F(powerUpBulletSize), \
    F(powerUpRapidFire), \
    F(powerUpBulletSpeed), \
    F(powerUpSpread), \
    F(powerUpHeatSeek), \
    F(powerUpBulletDistance), \
    F(powerUpBounce), \
    F(powerUpExplode), \
    F(enemyBehaviorBorder), \
    F(enemyBehaviorFollow), \
    F(enemyBehaviorDodge), \
    F(enemyBehaviorFast), \
    F(enemyBehaviorRandom), \
    F(enemyBehaviorCircle)
                



// first, let F simply resolve to the raw name
// thus, the above list expands into the body of the enum
#define F(inName) inName

enum spriteID {
	FIXED_SPRITE_NAMES,
    endSpriteID
    };


// maps each spriteID to a string (except for endSpriteID)
extern const char *spriteIDNames[];




extern int firstPowerUpID;
extern int lastPowerUpID;

extern int firstBehaviorID;
extern int lastBehaviorID;



//
// example call:
//  drawSprite( enemyBehaviorCircle, position );
//
void drawSprite( spriteID inID, doublePair inCenter );


Color getBlurredColor( spriteID inID );


#endif
