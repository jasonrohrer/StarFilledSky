#ifndef FIXED_SPRITE_BANK_INCLUDED
#define FIXED_SPRITE_BANK_INCLUDED


#include "minorGems/game/doublePair.h"
#include "minorGems/graphics/Color.h"

#include "minorGems/game/gameGraphics.h"



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
    F(powerUpSlotLeft), \
    F(powerUpSlotRight), \
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
    F(powerUpCornering), \
    /*  explode is used to mark the end of the power-ups */ \
    F(powerUpExplode), \
    F(enemyBehaviorBorder), \
    F(enemyBehaviorFollow), \
    F(enemyBehaviorDodge), \
    F(enemyBehaviorFast), \
    F(enemyBehaviorRandom), \
    F(enemyBehaviorCircle), \
    F(bracket) \

                



// first, let F simply resolve to the raw name
// thus, the above list expands into the body of the enum
#define F(inName) inName

enum spriteID {
	FIXED_SPRITE_NAMES,
    endSpriteID
    };


// maps each spriteID to a string (except for endSpriteID)
extern const char *spriteIDNames[];


// maps each spriteID to a translation key for the tip associated with
// that sprite (some sprites may not actually have tips defined).

extern const char *spriteIDTipTranslateKeys[];


// reverse of spriteIDNames
spriteID mapNameToID( const char *inName );



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


void drawCrosshairShadow( char inEnteringMouse, doublePair inCenter );

void drawPowerUpShadow( doublePair inCenter );



// utility function
// generates a shadow sprite from any tga file with transparent 
// background color
SpriteHandle generateShadowSprite( const char *inSourceTGAFile );


#endif
