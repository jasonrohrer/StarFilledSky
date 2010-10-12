#include "numerals.h"


#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/stringUtils.h"


static SpriteHandle spriteMap[10];


void initNumerals( const char *inTGAFileName ) {

    Image *spriteImage = readTGAFile( inTGAFileName );


    int subW = spriteImage->getWidth();

    // extra pixel spaces between numerals
    int subH = ( spriteImage->getHeight() + 1 ) / 10;
    
    for( int i=0; i<10; i++ ) {
        
        Image *numImage = spriteImage->getSubImage( 0, i * subH,
                                                    subW, subH - 1 );
        spriteMap[i] = fillSprite( numImage );
        
        delete numImage;
        }
    delete spriteImage;
    }



void freeNumerals() {
    for( int i=0; i<10; i++ ) {
        freeSprite( spriteMap[i] );
        }
    }



static double scaleFactor = 1 / 16.0;


static void drawNumeral( char inNumeral, doublePair inPosition ) {
    
    drawSprite( spriteMap[ (int)inNumeral - 48 ], inPosition,
                scaleFactor );
    }




void drawNumber( unsigned int inNumber, doublePair inPosition, 
                 TextAlignment inAlign ) {
    

    char *numberString = autoSprintf( "%d", inNumber );
    
    int numNumerals = strlen( numberString );
    
    

    // Assume right alignment only for now (yagni)

    for( int i=numNumerals-1; i>=0; i-- ) {
        drawNumeral( numberString[i], inPosition );
        
        inPosition.x -= .25;
        }
    
    }

