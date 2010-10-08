#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"


static SpriteHandle spriteBank[ 100 ];



void initSpriteBank() {
    spriteBank[ riseMarker ] = loadSprite( "riseMarker.tga", true );
    spriteBank[ crosshair ] = loadSprite( "crosshair.tga" );
    spriteBank[ enterCrosshair ] = loadSprite( "enterCrosshair.tga" );
    }



void freeSpriteBank() {
    
    for( int i=0; i<endSpriteID; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    }



void drawSprite( spriteID inID, doublePair inCenter ) {    
    drawSprite( spriteBank[ inID ], inCenter, 1.0/16 );
    
    }

