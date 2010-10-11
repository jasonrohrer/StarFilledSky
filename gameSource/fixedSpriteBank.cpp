#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"


static SpriteHandle spriteBank[ 100 ];



void initSpriteBank() {
    spriteBank[ riseMarker ] = loadSprite( "riseMarker.tga", false );
    spriteBank[ riseIcon ] = loadSprite( "riseIcon.tga" );
    spriteBank[ crosshair ] = loadSprite( "crosshair.tga" );
    spriteBank[ enterCrosshair ] = loadSprite( "enterCrosshair.tga" );
    spriteBank[ powerUpSlot ] = loadSprite( "powerUpSlot.tga" );
    spriteBank[ powerUpBorder ] = loadSprite( "powerUpBorder.tga" );
    spriteBank[ powerUpEmpty ] = loadSprite( "powerUpEmpty.tga" );
    }



void freeSpriteBank() {
    
    for( int i=0; i<endSpriteID; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    }



void drawSprite( spriteID inID, doublePair inCenter ) {    
    drawSprite( spriteBank[ inID ], inCenter, 1.0/16 );
    
    }

