#include "PlayerSprite.h"
#include "fixedSpriteBank.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>

extern CustomRandomSource randSource;



void PlayerSprite::generateReproducibleData() {
    if( mDataGenerated ) {
        // already generated
        return;
        }

    randSource.restoreFromSavedState( mRandSeedState );



    Image centerImage( 16, 16, 4, true );
    
    double *channels[4];
    
    for( int i=0; i<4; i++ ) {
        channels[i] = centerImage.getChannel( i );
        }
    

    for( int y=0; y<16; y++ ) {
        for( int x=0; x<8; x++ ) {
            int pixIndex = y * 16 + x;
            
            // only one chan with color, full saturation
            for( int i=0; i<3; i++ ) {
                channels[i][ pixIndex ] = 0;
                }
            
            // pick color randomly from primary
            int randPick = randSource.getRandomBoundedInt( 0, 2 );
            channels[0][pixIndex] = mColors.primary.elements[randPick].r;
            channels[1][pixIndex] = mColors.primary.elements[randPick].g;
            channels[2][pixIndex] = mColors.primary.elements[randPick].b;
            }       
        }

    // random walk to fill in some secondary color
    for( int r=0; r<3; r++ ) {
        int x=7;
        int y = 7;
        

        for( int s=0; s<30; s++ ) {    
        
            int pixIndex = y * 16 + x;
        

            int randPick = randSource.getRandomBoundedInt( 0, 2 );

            channels[0][ pixIndex ] = mColors.secondary.elements[randPick].r;
            channels[1][ pixIndex ] = mColors.secondary.elements[randPick].g;
            channels[2][ pixIndex ] = mColors.secondary.elements[randPick].b;
        
            if( randSource.getRandomBoolean() ) {
                x += randSource.getRandomBoundedInt( -1, 1 );
                }
            else {
                y += randSource.getRandomBoundedInt( -1, 1 );
                }
        
            if( x < 0 ) {
                x = 0;
                }
            else if( x > 7 ) {
                x = 7;
                }
            if( y < 0 ) {
                y = 0;
                }
            else if( y > 15 ) {
                y = 15;
                }
            }
        }
    
    // symmetrical
    for( int y=0; y<16; y++ ) {
        for( int x=8; x<16; x++ ) {
            for( int i=0; i<3; i++ ) {
                channels[i][ y * 16 + x ] =
                    channels[i][ y * 16 + (16 - x - 1) ];
                }
            }       
        }


    BoxBlurFilter filter( 1 );
    

    centerImage.filter( &filter );
    

    // start random walks in center to lay out alpha
    for( int r=0; r<15; r++ ) {
        int x=7;
        int y = 7;
        

        for( int s=0; s<30; s++ ) {    
            channels[3][ y * 16 + x ] = 1;
            mFillMap[y][x] = true;

            if( randSource.getRandomBoolean() ) {
                x += randSource.getRandomBoundedInt( -1, 1 );
                }
            else {
                y += randSource.getRandomBoundedInt( -1, 1 );
                }
            
            if( x < 1 ) {
                x = 1;
                }
            else if( x > 7 ) {
                x = 7;
                }
            if( y < 1 ) {
                y = 1;
                }
            else if( y > 14 ) {
                y = 14;
                }
            }
        }

    // symmetrical
    for( int y=0; y<16; y++ ) {
        for( int x=8; x<16; x++ ) {
            channels[3][ y * 16 + x ] =
                channels[3][ y * 16 + (16 - x - 1) ];
            mFillMap[y][x] = mFillMap[ y ][ 16 - x - 1 ];
            }       
        }
    
    /*
    for( int p=0; p<16*16; p++ ) {
        channels[3][p] = 1;
        }
    */

    mCenterSprite = fillSprite( &centerImage );
    
    
    Image borderImage( 16, 16, 4, false );

    
    double *borderChannels[4];
    
    for( int i=0; i<4; i++ ) {
        borderChannels[i] = borderImage.getChannel( i );
        }

    Color borderColor = mColors.primary.elements[3];

    for( int p=0; p<16*16; p++ ) {
        borderChannels[0][p] = borderColor.r;
        borderChannels[1][p] = borderColor.g;
        borderChannels[2][p] = borderColor.b;
        
        // all trans for now
        borderChannels[3][p] = 0;
        }
    
    for( int y=1; y<15; y++ ) {
        for( int x=1; x<15; x++ ) {
            
            if( channels[3][ y*16+x ] != 0 ) {
                
                // fill empty neighbors in border image with opaque pixels

                int n = (y-1) * 16 + x;
                
                if( channels[3][n] == 0 ) {
                    borderChannels[3][n] = 1;
                    }
                n = (y+1) * 16 + x;
                if( channels[3][n] == 0 ) {
                    borderChannels[3][n] = 1;
                    }
                n = y * 16 + x - 1;
                if( channels[3][n] == 0 ) {
                    borderChannels[3][n] = 1;
                    }
                n = y * 16 + x + 1;
                if( channels[3][n] == 0 ) {
                    borderChannels[3][n] = 1;
                    }
                }
            }
        }
    

    mBorderSprite = fillSprite( &borderImage );

    mDataGenerated = true;
    }



void PlayerSprite::freeReproducibleData() {
    if( mDataGenerated ) {
        freeSprite( mBorderSprite );
        mBorderSprite = NULL;
        freeSprite( mCenterSprite );
        mCenterSprite = NULL;
        }
    mDataGenerated = false;
    }



PlayerSprite::PlayerSprite( ColorScheme *inColors ) {

    if( inColors != NULL ) {
        mColors = *inColors;
        }
    

    mDataGenerated = false;
    randSource.saveState();
    mRandSeedState = randSource.getSavedState();

    generateReproducibleData();
    }



void PlayerSprite::compactSprite() {
    freeReproducibleData();
    }

        
void PlayerSprite::decompactSprite() {
    generateReproducibleData();
    }




static double scaleFactor = 1.0 / 16;

void PlayerSprite::drawCenter( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( mCenterSprite, inPosition, scaleFactor );
    
    setDrawColor( mColors.special.r,
                  mColors.special.g,
                  mColors.special.b, inFade );

    // round to single-pixel move
    doublePair roundedOffset = mult( mEyeOffset, 1 / scaleFactor );
    roundedOffset.x = rint( roundedOffset.x );
    roundedOffset.y = rint( roundedOffset.y );
    roundedOffset = mult( roundedOffset, scaleFactor );
    
    doublePair leftOffset = roundedOffset;
    
    if( leftOffset.x > -scaleFactor ) {
        leftOffset.x = -scaleFactor;
        }
    doublePair rightOffset = roundedOffset;
    
    if( rightOffset.x < scaleFactor ) {
        rightOffset.x = scaleFactor;
        }
    

    doublePair eyePos = add( inPosition, leftOffset );
    
    if( mSquintTimeLeft > 0 ) {
        drawSprite( eyeLeftSquint, eyePos );
        }
    else {
        drawSprite( eyeLeft, eyePos );
        }
    
    eyePos = add( inPosition, rightOffset );
    
    if( mSquintTimeLeft > 0 ) {
        drawSprite( eyeSquint, eyePos );                  
        }
    else {
        drawSprite( eye, eyePos );                  
        }
    }

