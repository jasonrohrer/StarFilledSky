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
    
    // tweak to center, since bullets have odd sizes
    // hackish:  half a sprite pixel
    inCenter.x += 0.03125;
    inCenter.y -= 0.03125;


    int baseSize = (int)inSize;
    
    float extra = inSize - baseSize;
    

    if( extra != 0 ) {
        setDrawFade( extra );
        drawSprite( spriteBank[ baseSize ], inCenter, 1.0/16 );
   
        if( extra < 0.25 ) {
            setDrawFade( 1 );
            }
        else {
            setDrawFade( (1 - extra) / 0.75 );
            }
        }
    else {
        setDrawFade( 1 );
        }
    
    drawSprite( spriteBank[ baseSize - 1 ], inCenter, 1.0/16 );    
    }

