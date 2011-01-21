#include "bulletSizeSet.h"
#include "minorGems/game/gameGraphics.h"

#include "minorGems/graphics/filters/FastBlurFilter.h"

#include "minorGems/util/stringUtils.h"

#include "fixedSpriteBank.h"


int maxBulletSize = 7;


static SpriteHandle spriteBank[ 7 ];

static SpriteHandle shadowSpriteBank[ 7 ];




void initBulletSizeSet() {

    FastBlurFilter filter;
    
    for( int i=0; i<maxBulletSize; i++ ) {
        
        char *fileName = autoSprintf( "bullet%d.tga", i + 1 );

        spriteBank[ i ] = loadSprite( fileName );

        shadowSpriteBank[ i ] = generateShadowSprite( fileName );

        delete [] fileName;
        }
    

    }



void freeBulletSizeSet() {
    
    for( int i=0; i<maxBulletSize; i++ ) {
        freeSprite( spriteBank[ i ] );
        freeSprite( shadowSpriteBank[ i ] );
        }
    }



void drawBulletShadow( float inSize, doublePair inCenter ) {
    int baseSize = (int)inSize;
    
    // scale tiny shadow image up to blur it more
    drawSprite( shadowSpriteBank[ baseSize - 1 ], inCenter, 1.0/4.0 );
    }



void drawBullet( float inSize, doublePair inCenter, float inFade ) {
    
    // tweak to center, since bullets have odd sizes
    // hackish:  half a sprite pixel
    inCenter.x += 0.03125;
    inCenter.y -= 0.03125;


    int baseSize = (int)inSize;
    
    float extra = inSize - baseSize;    
                
    if( extra != 0 ) {
        setDrawFade( extra * inFade );
        drawSprite( spriteBank[ baseSize ], inCenter, 1.0/16 );
   
        if( extra < 0.25 ) {
            setDrawFade( inFade);
            }
        else {
            setDrawFade( inFade * (1 - extra) / 0.75 );
            }
        }
    else {
        setDrawFade( inFade );
        }
    
    drawSprite( spriteBank[ baseSize - 1 ], inCenter, 1.0/16 );    
    }

