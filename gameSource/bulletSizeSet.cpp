#include "bulletSizeSet.h"
#include "minorGems/game/gameGraphics.h"

#include "minorGems/graphics/filters/FastBlurFilter.h"

#include "minorGems/util/stringUtils.h"



int maxBulletSize = 7;


static SpriteHandle spriteBank[ 7 ];

static SpriteHandle shadowSpriteBank[ 7 ];




void initBulletSizeSet() {

    FastBlurFilter filter;
    
    for( int i=0; i<maxBulletSize; i++ ) {
        
        char *fileName = autoSprintf( "bullet%d.tga", i + 1 );
        

        spriteBank[ i ] = loadSprite( fileName );

        Image *spriteImage = readTGAFile( fileName );
        
        delete [] fileName;

        int w = spriteImage->getWidth();
        int h = spriteImage->getHeight();

        // lower left corner
        Color transColor = spriteImage->getColor( (h-1) * w );


        Image shadowImage( 16, 16, 4, true );
        
        double *shadowAlpha = shadowImage.getChannel( 3 );
        

        int halfW = w/2;
        int halfH = h/2;
        

        // shadow image is centered at 1/4 size to be blown up by texture
        // scaling later
        for( int y=0; y<h; y++ ) {
            for( int x=0; x<w; x++ ) {
                Color pixelColor = spriteImage->getColor( y*w + x );
                
                if( !pixelColor.equals( &transColor ) ) {
                    
                    // fill in shadow
                    shadowAlpha[ ((y-halfH) / 4 + halfH) * w + 
                                 ((x-halfW) / 4 + halfW) ] = 1;
                    }
                }  
            }


        shadowImage.filter( &filter, 3 );
        shadowImage.filter( &filter, 3 );
        shadowImage.filter( &filter, 3 );


        delete spriteImage;
        
        shadowSpriteBank[ i ] = fillSprite( &shadowImage, false );
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

