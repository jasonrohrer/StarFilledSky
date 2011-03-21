#include "fixedSpriteBank.h"




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



// redefine F so that it expands each name into a _tip translation key
// constant
#undef F
#define F(inName) #inName "_tip"

const char *spriteIDTipTranslateKeys[] = {
	FIXED_SPRITE_NAMES
    };




static SpriteHandle spriteBank[ endSpriteID ];
static Color blurredColors[ endSpriteID ];

static SpriteHandle crosshairShadow;
static SpriteHandle enterCrosshairShadow;
static SpriteHandle powerUpShadow;


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
    transparentLowerLeftCorner[ flagEditCell ] = false;


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
    
    crosshairShadow = 
        generateShadowSprite( fixedSpriteFileNames[ crosshair ] );
    enterCrosshairShadow = 
        generateShadowSprite( fixedSpriteFileNames[ enterCrosshair ] );

    powerUpShadow = 
        generateShadowSprite( "powerUpFullMask.tga" );

    }



void freeSpriteBank() {
    
    for( int i=0; i<endSpriteID; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    freeSprite( crosshairShadow );
    freeSprite( enterCrosshairShadow );
    freeSprite( powerUpShadow );
    }



void drawSprite( spriteID inID, doublePair inCenter ) {    
    drawSprite( spriteBank[ inID ], inCenter, 1.0/16 );
    
    }


Color getBlurredColor( spriteID inID ) {
    return blurredColors[ inID ];
    }


void drawCrosshairShadow( char inEnteringMouse, doublePair inCenter ) {    

    // scale up with linear filter
    if( inEnteringMouse ) {
        drawSprite( enterCrosshairShadow, inCenter, 1.0/4 );
        }
    else {
        drawSprite( crosshairShadow, inCenter, 1.0/4 );
        }
    }



void drawPowerUpShadow( doublePair inCenter ) {
    // scale up with linear filter

    drawSprite( powerUpShadow, inCenter, 1.0/4 );
    }



#include "minorGems/graphics/filters/FastBlurFilter.h"


SpriteHandle generateShadowSprite( const char *inSourceTGAFile ) {
    Image *spriteImage = readTGAFile( inSourceTGAFile );

    int w = spriteImage->getWidth();
    int h = spriteImage->getHeight();

    // lower left corner
    Color transColor = spriteImage->getColor( (h-1) * w );


    Image shadowImage( w, h, 4, true );
        
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
                int fx = x / 4 + halfW - halfW / 4;
                int fy = y / 4 + halfH - halfH / 4;
                shadowAlpha[ fy * w + fx ] = 1;
                }
            }  
        }

    FastBlurFilter filter;

    shadowImage.filter( &filter, 3 );
    shadowImage.filter( &filter, 3 );
    shadowImage.filter( &filter, 3 );


    delete spriteImage;
        
    return fillSprite( &shadowImage, false );
    }



spriteID mapNameToID( const char *inName ) {
    for( int i = riseMarker; i < endSpriteID; i++ ) {
        
        if( strcmp( inName, spriteIDNames[i] ) == 0 ) {
            return (spriteID)i;
            }    
        }

    return endSpriteID;
    }




