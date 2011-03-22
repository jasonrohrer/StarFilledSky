#include "flagSprite.h"


unsigned char flagColorMap[16][3] = 
{ 
    { 223, 45, 0 },
    { 49, 221, 9 },
    { 56, 56, 187 },
    { 255, 245, 218 },

    { 54, 54, 54 },
    { 255, 221, 0 },
    { 76, 208, 208 },
    { 212, 0, 211 },

    { 230, 135, 0 },
    { 123, 25, 177 },
    { 90, 255, 139 },
    { 128, 128, 128 },

    { 127, 79, 0 },
    { 31, 115, 1 },
    { 113, 0, 27 },
    { 255, 159, 218 } 
};



int hexTo16( char inHexChar ) {
    
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


char sixteenToHex( int inNumber ) {
    if( inNumber < 10 ) {
        return (char)( 48 + inNumber );
        }
    else if( inNumber < 16 ) {
        return (char)( 65 + (inNumber - 10 ) );
        }
    else {
        // default, out of range
        return 0;
        }        
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

