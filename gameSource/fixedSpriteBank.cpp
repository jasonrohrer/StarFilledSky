#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"



// redefine F so that it expands each name into a string constant
#undef F
#define F(inName) #inName

const char *spriteIDNames[] = {
	FIXED_SPRITE_NAMES
    };



// redefine F so that it expands each name into a file-name string
// constant
#undef F
#define F(inName) #inName ".tga"

const char *fixedSpriteFileNames[] = {
	FIXED_SPRITE_NAMES
    };



static SpriteHandle spriteBank[ endSpriteID ];
static Color blurredColors[ endSpriteID ];

static char transparentLowerLeftCorner[ endSpriteID ];


int firstPowerUpID = powerUpEmpty;
int lastPowerUpID = powerUpExplode;

int firstBehaviorID = enemyBehaviorFollow;
int lastBehaviorID = enemyBehaviorCircle;


void initSpriteBank() {
    for( int i = riseMarker; i < endSpriteID; i++ ) {
        transparentLowerLeftCorner[ i ] =  true;
        }
    // special case, no transparency
    transparentLowerLeftCorner[ riseMarker ] = false;


    for( int i = riseMarker; i < endSpriteID; i++ ) {
        spriteBank[ i ] = loadSprite( fixedSpriteFileNames[i],
                                      transparentLowerLeftCorner[i] );
        }

    // compute average colors of each
    for( int i = riseMarker; i < endSpriteID; i++ ) {
    
        Image *spriteImage = readTGAFile( fixedSpriteFileNames[i] );
        
        if( spriteImage != NULL ) {
            int w = spriteImage->getWidth();
            int h = spriteImage->getHeight();
            
            int numPixels = w * h;
            
            double sums[3] = {0,0,0};
            
            int cornerIndex = (h-1) * w;
            
            double *channels[3] = { spriteImage->getChannel(0),
                                    spriteImage->getChannel(1),
                                    spriteImage->getChannel(2) };
            
            double cornerColor[3] = { channels[0][cornerIndex],
                                      channels[1][cornerIndex],
                                      channels[2][cornerIndex] };
            

            int numFullPixels = 0;

            for( int p=0; p<numPixels; p++ ) {
            
                if( ! transparentLowerLeftCorner[i] ||
                    channels[0][p] != cornerColor[0] ||
                    channels[1][p] != cornerColor[1] ||
                    channels[2][p] != cornerColor[2] ) {
                    
                    for( int c=0; c<3; c++ ) {
                        sums[c] += channels[c][p];
                        }
                    numFullPixels ++;
                    }
                }
            
            for( int c=0; c<3; c++ ) {
                blurredColors[i][c] = sums[c] / numFullPixels;
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


