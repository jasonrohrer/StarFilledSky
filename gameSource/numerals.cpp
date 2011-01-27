#include "numerals.h"


#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/stringUtils.h"


// 11th is plus sign
static SpriteHandle spriteMap[11];


void initNumerals( const char *inTGAFileName ) {

    Image *spriteImage = readTGAFile( inTGAFileName );


    int subW = spriteImage->getWidth();

    // extra pixel spaces between numerals
    int subH = ( spriteImage->getHeight() + 1 ) / 11;
    
    for( int i=0; i<11; i++ ) {
        
        Image *numImage = spriteImage->getSubImage( 0, i * subH,
                                                    subW, subH - 1 );
        spriteMap[i] = fillSprite( numImage );
        
        delete numImage;
        }
    delete spriteImage;
    }



void freeNumerals() {
    for( int i=0; i<11; i++ ) {
        freeSprite( spriteMap[i] );
        }
    }



static double scaleFactor = 1 / 16.0;


static void drawNumeral( char inNumeral, doublePair inPosition ) {
    
    if( inNumeral == '+' ) {
        drawSprite( spriteMap[ 10 ], inPosition,
                    scaleFactor );
        }
    else {
        drawSprite( spriteMap[ (int)inNumeral - 48 ], inPosition,
                    scaleFactor );
        }
    
    }




void drawNumber( unsigned int inNumber, doublePair inPosition, 
                 TextAlignment inAlign, char inPrependPlus ) {
    

    char *numberString;
    
    if( inPrependPlus ) {
        numberString = autoSprintf( "+%d", inNumber );
        }
    else {
        numberString = autoSprintf( "%d", inNumber );
        }
    
    int numNumerals = strlen( numberString );
    
    

    // Assume right alignment only for now (yagni)

    for( int i=numNumerals-1; i>=0; i-- ) {
        drawNumeral( numberString[i], inPosition );
        
        inPosition.x -= .25;
        }

    delete [] numberString;    
    }

