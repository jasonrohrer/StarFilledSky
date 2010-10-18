#include "bulletSizeSet.h"
#include "minorGems/game/gameGraphics.h"


int maxBulletSize = 7;


static SpriteHandle spriteBank[ 7 ];





void initBulletSizeSet() {
    spriteBank[ 0 ] = loadSprite( "bullet1.tga" );
    spriteBank[ 1 ] = loadSprite( "bullet2.tga" );
    spriteBank[ 2 ] = loadSprite( "bullet3.tga" );
    spriteBank[ 3 ] = loadSprite( "bullet4.tga" );
    spriteBank[ 4 ] = loadSprite( "bullet5.tga" );
    spriteBank[ 5 ] = loadSprite( "bullet6.tga" );
    spriteBank[ 6 ] = loadSprite( "bullet7.tga" );
    }



void freeBulletSizeSet() {
    
    for( int i=0; i<maxBulletSize; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    }



void drawBullet( float inSize, doublePair inCenter ) {
    int baseSize = (int)inSize;
    
    setDrawFade( 1 );
    drawSprite( spriteBank[ baseSize - 1 ], inCenter, 1.0/16 );
    
    
    float extra = inSize - baseSize;
    

    if( extra != 0 ) {
        setDrawFade( extra );
        drawSprite( spriteBank[ baseSize ], inCenter, 1.0/16 );
        }
    
    }

