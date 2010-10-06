#include "Font.h"

#include "minorGems/graphics/RGBAImage.h"

#include <string.h>


typedef union rgbaColor {
        struct comp { 
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;
            } comp;
        
        // access those bytes as an array
        unsigned char bytes[4];
        
        // reinterpret those bytes as an unsigned int
        unsigned int rgbaInt; 
    } rgbaColor;



Font::Font( const char *inFileName, int inCharSpacing, int inSpaceWidth,
            char inFixedWidth )
        : mCharSpacing( inCharSpacing ), mSpaceWidth( inSpaceWidth ),
          mFixedWidth( inFixedWidth ) {


    Image *spriteImage = readTGAFile( inFileName );
    
    if( spriteImage != NULL ) {
        
        int width = spriteImage->getWidth();
        
        int height = spriteImage->getHeight();
        
        int numPixels = width * height;
        
        rgbaColor *spriteRGBA = new rgbaColor[ numPixels ];
        
        
        unsigned char *spriteBytes = 
            RGBAImage::getRGBABytes( spriteImage );
        
        delete spriteImage;

        for( int i=0; i<numPixels; i++ ) {
            
            for( int b=0; b<4; b++ ) {
                
                spriteRGBA[i].bytes[b] = spriteBytes[ i*4 + b ];
                }
            }
        
        delete [] spriteBytes;
        
        

        // use corner color as transparency
        // copy red channel into other channels
        // (ignore other channels, even for transparency)
        
        spriteRGBA[0].comp.a = 0;
        unsigned char tr;
        tr = spriteRGBA[0].comp.r;

        for( int i=0; i<numPixels; i++ ) {
            unsigned char r = spriteRGBA[i].comp.r;
            
            if( r == tr ) {
                // matches corner r
                spriteRGBA[i].comp.a = 0;
                }
                
            spriteRGBA[i].comp.g = r;
            spriteRGBA[i].comp.b = r;
            }
            
                        
                
        mSpriteWidth = width / 16;
        mSpriteHeight = height / 16;
        
        int pixelsPerChar = mSpriteWidth * mSpriteHeight;
            
        for( int i=0; i<256; i++ ) {
            int yOffset = ( i / 16 ) * mSpriteHeight;
            int xOffset = ( i % 16 ) * mSpriteWidth;
            
            rgbaColor *charRGBA = new rgbaColor[ pixelsPerChar ];
            
            for( int y=0; y<mSpriteHeight; y++ ) {
                for( int x=0; x<mSpriteWidth; x++ ) {
                    
                    int imageIndex = (y + yOffset) * width
                        + x + xOffset;
                    int charIndex = y * mSpriteWidth + x;
                    
                    charRGBA[ charIndex ] = spriteRGBA[ imageIndex ];
                    }
                }
                
            // don't bother consuming texture ram for blank sprites
            char allTransparent = true;
            
            for( int p=0; p<pixelsPerChar && allTransparent; p++ ) {
                if( charRGBA[ p ].comp.a != 0 ) {
                    allTransparent = false;
                    }
                }
                
            if( !allTransparent ) {
                
                // convert into an image
                Image *charImage = new Image( mSpriteWidth, mSpriteHeight,
                                              4, false );
                
                for( int c=0; c<4; c++ ) {
                    double *chan = charImage->getChannel(c);
                    
                    for( int p=0; p<pixelsPerChar; p++ ) {
                        
                        chan[p] = charRGBA[p].bytes[c] / 255.0;
                        }
                    }
                

                mSpriteMap[i] = 
                    fillSprite( charImage );
                delete charImage;
                }
            else {
                mSpriteMap[i] = NULL;
                }
            

            if( mFixedWidth ) {
                mCharLeftEdgeOffset[i] = 0;
                mCharWidth[i] = mSpriteWidth;
                }
            else if( allTransparent ) {
                mCharLeftEdgeOffset[i] = 0;
                mCharWidth[i] = mSpriteWidth;
                }
            else {
                // implement pseudo-kerning
                
                int farthestLeft = mSpriteWidth;
                int farthestRight = 0;
                
                char someInk = false;
                
                for( int y=0; y<mSpriteHeight; y++ ) {
                    for( int x=0; x<mSpriteWidth; x++ ) {
                        
                        unsigned char r = 
                            charRGBA[ y * mSpriteWidth + x ].comp.r;
                        
                        if( r > 0 ) {
                            someInk = true;
                            
                            if( x < farthestLeft ) {
                                farthestLeft = x;
                                }
                            if( x > farthestRight ) {
                                farthestRight = x;
                                }
                            }
                        }
                    }
                
                if( ! someInk  ) {
                    mCharLeftEdgeOffset[i] = 0;
                    mCharWidth[i] = mSpriteWidth;
                    }
                else {
                    mCharLeftEdgeOffset[i] = farthestLeft;
                    mCharWidth[i] = farthestRight - farthestLeft + 1;
                    }
                }
                

            delete [] charRGBA;
            }
                        
        delete [] spriteRGBA;
        }
    }



Font::~Font() {
    for( int i=0; i<256; i++ ) {
        if( mSpriteMap[i] != NULL ) {
            freeSprite( mSpriteMap[i] );
            }
        }
    }

// double pixel size
//static double scaleFactor = 1.0 / 16;
static double scaleFactor = 1.0 / 8;



double Font::drawString( const char *inString, doublePair inPosition,
                         TextAlignment inAlign ) {
    unsigned int numChars = strlen( inString );
    
    double x = inPosition.x;
    
    // compensate for extra headspace in accent-equipped font files
    double y = inPosition.y + scaleFactor * mSpriteHeight / 4;
    
    double stringWidth = 0;
    
    if( inAlign != alignLeft ) {
        stringWidth = measureString( inString );
        }
    
    switch( inAlign ) {
        case alignCenter:
            x -= stringWidth / 2;
            break;
        case alignRight:
            x -= stringWidth;
            break;
        default:
            // left?  do nothing
            break;            
        }
    
    // character sprites are drawn on their centers, so the alignment
    // adjustments above aren't quite right.
    if( !mFixedWidth ) {        
        x += scaleFactor * mCharWidth[ (int)( inString[0] ) ] / 2 +
            scaleFactor * mCharLeftEdgeOffset[ (int)( inString[0] ) ];
        }
    else {
        x += scaleFactor * mSpriteWidth / 2;
        }
    

    for( unsigned int i=0; i<numChars; i++ ) {
        doublePair charPos = { x, y };
        
        double charWidth = drawCharacter( inString[i], charPos );
        x += charWidth + mCharSpacing * scaleFactor;
        }
    // no spacing after last character
    x -= mCharSpacing * scaleFactor;

    return x;
    }





double Font::drawCharacter( char inC, doublePair inPosition ) {
    if( inC == ' ' ) {
        return mSpaceWidth * scaleFactor;
        }

    if( !mFixedWidth ) {
        inPosition.x -= mCharLeftEdgeOffset[ (int)inC ] * scaleFactor;
        }
    
    SpriteHandle spriteID = mSpriteMap[ (int)inC ];
    
    if( spriteID != NULL ) {
        drawSprite( mSpriteMap[ (int)inC ], inPosition, scaleFactor );
        }
    
    if( mFixedWidth ) {
        return mSpriteWidth * scaleFactor;
        }
    else {
        return mCharWidth[ (int)inC ] * scaleFactor;
        }
    }



double Font::measureString( const char *inString ) {
    unsigned int numChars = strlen( inString );

    double width = 0;
    
    for( unsigned int i=0; i<numChars; i++ ) {
        char c = inString[i];
        
        if( c == ' ' ) {
            width += mSpaceWidth * scaleFactor;
            }
        else if( mFixedWidth ) {
            width += mSpriteWidth * scaleFactor;
            }
        else {
            width += mCharWidth[ (int)c ] * scaleFactor;
            }
    
        width += mCharSpacing * scaleFactor;
        }

    // no extra space at end 
    width -= mCharSpacing * scaleFactor;
    
    return width;
    }
