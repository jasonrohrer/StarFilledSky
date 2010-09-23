#include "PlayerSprite.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



PlayerSprite::PlayerSprite() {

    Image centerImage( 16, 16, 4, true );
    
    double *channels[4];
    
    for( int i=0; i<4; i++ ) {
        channels[i] = centerImage.getChannel( i );
        }
    

    // symmetrical
    for( int y=0; y<16; y++ ) {
        for( int x=0; x<8; x++ ) {
            int pixIndex = y * 16 + x;
            
            // only one chan with color, full saturation
            for( int i=0; i<3; i++ ) {
                channels[i][ pixIndex ] = 0;
                }
            
            int chanWithColor = randSource.getRandomBoundedInt( 0, 2 );
            
            channels[chanWithColor][pixIndex] = randSource.getRandomDouble();

            int chanWithColor2 = randSource.getRandomBoundedInt( 0, 2 );

            channels[chanWithColor2][pixIndex] = randSource.getRandomDouble();

            int chanWithColor3 = randSource.getRandomBoundedInt( 0, 2 );

            channels[chanWithColor3][pixIndex] = randSource.getRandomDouble();
            }
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
    for( int r=0; r<20; r++ ) {
        int x=7;
        int y = 7;
        

        for( int s=0; s<30; s++ ) {    
            channels[3][ y * 16 + x ] = 1;

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
            }       
        }
    
    /*
    for( int p=0; p<16*16; p++ ) {
        channels[3][p] = 1;
        }
    */

    mCenterSprite = fillSprite( &centerImage );
    
    
    Image borderImage( 16, 16, 4, true );

    
    double *borderChannels[4];
    
    for( int i=0; i<4; i++ ) {
        borderChannels[i] = borderImage.getChannel( i );
        }
    
    for( int y=1; y<15; y++ ) {
        for( int x=1; x<15; x++ ) {
            
            if( channels[3][ y*16+x ] != 0 ) {
                
                // fill empty neighbors in border image with black

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
    }
