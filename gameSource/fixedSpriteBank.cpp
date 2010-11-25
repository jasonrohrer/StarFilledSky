#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"



// redefine F so that it expands each name into a file-name string
// constant
#undef F
#define F(inName) #inName ".tga"

const char *fixedSpriteFileNames[] = {
	FIXED_SPRITE_NAMES
    };



static SpriteHandle spriteBank[ endSpriteID ];
static Color blurredColors[ endSpriteID ];



int firstPowerUpID = powerUpEmpty;
int lastPowerUpID = powerUpExplode;

int firstBehaviorID = enemyBehaviorFollow;
int lastBehaviorID = enemyBehaviorCircle;


void initSpriteBank() {
    // special case, no transparency
    spriteBank[ riseMarker ] = loadSprite( "riseMarker.tga", false );

    // rest are identical
    for( int i = riseIcon; i < endSpriteID; i++ ) {
        spriteBank[ i ] = loadSprite( fixedSpriteFileNames[i] );
        }

    // compute average colors of each
    for( int i = riseMarker; i < endSpriteID; i++ ) {
    
        Image *spriteImage = readTGAFile( fixedSpriteFileNames[i] );
        
        if( spriteImage != NULL ) {
            int numPixels = spriteImage->getWidth() * 
                spriteImage->getHeight();
            double sums[3] = {0,0,0};
            
            // FIXME:  take sprite mask into account here...
            // how to do this easily?  Corner color?  What about
            // for rise marker?

            for( int c=0; c<3; c++ ) {
                double *channel = spriteImage->getChannel( c );

                for( int p=0; p<numPixels; p++ ) {
                    sums[c] += channel[p];
                    }
                blurredColors[i][c] = sums[c] / numPixels;
                }
            
            delete spriteImage;
            }
        
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


Color getBlurredColor( spriteID inID ) {
    return blurredColors[ inID ];
    }


