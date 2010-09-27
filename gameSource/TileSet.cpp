#include "TileSet.h"

#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;


TileSet::TileSet() {
    Image wallImage( 16, 16, 4, true );
    
    double *channels[4];
    
    for( int i=0; i<4; i++ ) {
        channels[i] = wallImage.getChannel( i );
        }
    

    // asymmetrical, black border
    for( int y=1; y<15; y++ ) {
        for( int x=1; x<15; x++ ) {
            int pixIndex = y * 16 + x;
            
            // only one chan with color, full saturation
            for( int i=0; i<3; i++ ) {
                channels[i][ pixIndex ] = 0;
                }
            

            int chanWithColor = randSource.getRandomBoundedInt( 0, 2 );
            
            channels[chanWithColor][pixIndex] = randSource.getRandomDouble();

            int chanWithColor2 = randSource.getRandomBoundedInt( 0, 2 );

            channels[chanWithColor2][pixIndex] = randSource.getRandomDouble();

            //int chanWithColor3 = randSource.getRandomBoundedInt( 0, 2 );

            //channels[chanWithColor3][pixIndex] = randSource.getRandomDouble();
            }
        /*
        for( int x=8; x<16; x++ ) {
            for( int i=0; i<3; i++ ) {
                channels[i][ y * 16 + x ] =
                    channels[i][ y * 16 + (16 - x - 1) ];
                }
            }
        */       
        }

    BoxBlurFilter filter( 2 );    

    wallImage.filter( &filter );
    
    // full square opaque
    for( int p=0; p<16*16; p++ ) {
        channels[3][p] = 1;
        }
    

    
    mWall = fillSprite( &wallImage );

    

    }

        
TileSet::~TileSet() {
    }

static double scaleFactor = 1.0 / 16;

void TileSet::drawWall( doublePair inPosition ) {
    setDrawColor( 1, 1, 1, 1 );
    drawSprite( mWall, inPosition, scaleFactor );
    }

        
void TileSet::drawFloor( doublePair inPosition ) {
    }

