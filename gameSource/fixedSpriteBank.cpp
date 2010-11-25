#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"



// redefine F so that it expands each name into a file-name string
// constant
#undef F
#define F(inName) #inName ".tga"

const char *fixedSpriteFileNames[] = {
	FIXED_SPRITE_NAMES
    };



static SpriteHandle spriteBank[ 100 ];


int firstPowerUpID = powerUpEmpty;
int lastPowerUpID = powerUpExplode;

int firstBehaviorID = enemyBehaviorFollow;
int lastBehaviorID = enemyBehaviorCircle;


void initSpriteBank() {
    spriteBank[ riseMarker ] = loadSprite( "riseMarker.tga", false );

    for( int i = riseIcon; i < endSpriteID; i++ ) {
        spriteBank[ i ] = loadSprite( fixedSpriteFileNames[ i ] );
        }
    }



void freeSpriteBank() {
    
    for( int i=0; i<endSpriteID; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    }



void drawSprite( spriteID inID, doublePair inCenter ) {    
    drawSprite( spriteBank[ inID ], inCenter, 1.0/16 );
    
    }

