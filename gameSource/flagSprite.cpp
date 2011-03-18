#include "flagSprite.h"


static unsigned char flagColorMap[16][3] = 
{ 
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
    { 255, 255, 255 },

    { 0, 0, 0 },
    { 255, 255, 0 },
    { 0, 255, 255 },
    { 255, 0, 255 },

    { 255, 128, 0 },
    { 128, 0, 255 },
    { 0, 255, 128 },
    { 128, 128, 128 },

    { 128, 64, 0 },
    { 0, 128, 0 },
    { 128, 0, 0 },
    { 255, 128, 255 } 
};



static int hexTo16( char inHexChar ) {
    
    // numerals
    if( inHexChar >= 48 && inHexChar <= 57 ) {
        return inHexChar - 48;
        }
    
    // A - F
    if( inHexChar >= 65 && inHexChar <= 70 ) {
        return inHexChar - 65 + 10;
        }
    
    // default, bad encoding
    return 0;
    }

        


SpriteHandle generateFlagSprite( const char *inFlagString ) {

    int w = 16;
    int h = 16;
    

    unsigned char *rgba = new unsigned char[ w * h * 4 ];
    
    // start all transparent
    memset( rgba, 0, w * h * 4 );
    

    // flag is 3x3, blown up x4 to 12x12 and centered on the 16x16, 
    // with a 2 pixel border

    for( int y=2; y<h-2; y++ ) {
        int flagY = y - 2;
        int smallFlagY = flagY / 4;
        
        for( int x=2; x<w-2; x++ ) {
            int flagX = x - 2;
            int smallFlagX = flagX / 4;

            char flagChar = inFlagString[ smallFlagY * 3 + smallFlagX ];
            
            int colorIndex = hexTo16( flagChar );
            

            int pixelIndex = y * w * 4 + x * 4;
            
            for( int i=0; i<3; i++ ) {
                
                rgba[ pixelIndex + i ] = 
                    flagColorMap[ colorIndex ][i];
                }
            // solid where flag pixels are
            rgba[ pixelIndex + 3 ] = 255;
            }
        }
    


    SpriteHandle handle = fillSprite( rgba, w, h );

    delete [] rgba;
    
    return handle;
    }

