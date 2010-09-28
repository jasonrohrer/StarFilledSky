#include "TileSet.h"

#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



SpriteHandle TileSet::makeWallTile( ColorScheme inColors ) {
    Image wallImage( 16, 16, 4, true );
    
    double *channels[4];
    
    for( int i=0; i<4; i++ ) {
        channels[i] = wallImage.getChannel( i );
        }
    

    
    // asymmetrical, first secondary color border
    Color secondaryCenter = inColors.secondary.elements[0];
    for( int p=0; p<16*16; p++ ) {
        channels[0][p] = secondaryCenter.r;
        channels[1][p] = secondaryCenter.g;
        channels[2][p] = secondaryCenter.b;
        }

    for( int y=1; y<15; y++ ) {
        for( int x=1; x<15; x++ ) {
            int pixIndex = y * 16 + x;

            /*
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
            */

            // walls are primary
            Color c = 
                inColors.primary.elements[ 
                    randSource.getRandomBoundedInt( 0, 2 ) ];
            
            channels[0][ pixIndex ] = c.r;
            channels[1][ pixIndex ] = c.g;
            channels[2][ pixIndex ] = c.b;
            }
        }
    

    /*
    // white in center to test blur
    for( int y=1; y<15; y++ ) {
        for( int x=4; x<11; x++ ) {
            if( x %2 == 0 ) {
                
                int pixIndex = y * 16 + x;
                for( int i=0; i<3; i++ ) {
                    channels[i][ pixIndex ] = 1;
                    }
                }
            
            }
        }
    */

    BoxBlurFilter filter( 1 );    

    wallImage.filter( &filter );
    
    // full square opaque
    for( int p=0; p<16*16; p++ ) {
        channels[3][p] = 1;
        }

    return fillSprite( &wallImage );
    }



TileSet::TileSet() {

    ColorScheme colors;

    for( int i=0; i<NUM_TILE_PATTERNS; i++ ) {
        mWall[i] = makeWallTile( colors );
        }
    

    /*
    // corners trans
    channels[3][0] = 0;
    channels[3][15] = 0;
    channels[3][15 * 16] = 0;
    channels[3][15 * 16 + 15] = 0;
    */

    // edges

    Image wallImage( 16, 16, 4, true );
    
    double *channels[4];
    
    for( int i=0; i<4; i++ ) {
        channels[i] = wallImage.getChannel( i );
        }


    // full square trans
    for( int p=0; p<16*16; p++ ) {
        channels[3][p] = 0;
        }

    // pick edge color
    double edgeColor[3] = 
        { randSource.getRandomDouble(),
          randSource.getRandomDouble(),
          randSource.getRandomDouble() };
            
    // first fill in color for all edges
    for( int x=0; x<16; x++ ) {
        
        for( int i=0; i<3; i++ ) {
            channels[i][ x ] = edgeColor[i];
            }
    
        for( int i=0; i<3; i++ ) {
            channels[i][ 15 * 16 + x ] = edgeColor[i];
            }
        }
    for( int y=0; y<16; y++ ) {
        
        for( int i=0; i<3; i++ ) {
            channels[i][ y * 16 ] = edgeColor[i];
            }
    
        for( int i=0; i<3; i++ ) {
            channels[i][ y * 16 + 15 ] = edgeColor[i];
            }
        }
    
    // then select each edge using trans
    
    // top
    for( int x=0; x<16; x++ ) {
        channels[3][ x ] = 1;
        }
    mWallEdges[0] = fillSprite( &wallImage );
    
    // clear
    for( int x=0; x<16; x++ ) {
        channels[3][ x ] = 0;
        }



    // right
    for( int y=0; y<16; y++ ) {
        channels[3][ y * 16 + 15 ] = 1;
        }
    mWallEdges[1] = fillSprite( &wallImage );

    // clear
    for( int y=0; y<16; y++ ) {
        channels[3][ y * 16 + 15  ] = 0;
        }

        


    // bottom
    for( int x=0; x<16; x++ ) {
        channels[3][ 15 * 16 + x ] = 1;
        }
    mWallEdges[2] = fillSprite( &wallImage );

    // clear
    for( int x=0; x<16; x++ ) {
        channels[3][ 15 * 16 + x ] = 0;
        }



    // left
    for( int y=0; y<16; y++ ) {
        channels[3][ y * 16 ] = 1;
        }
    mWallEdges[3] = fillSprite( &wallImage );

    // clear
    for( int y=0; y<16; y++ ) {
        channels[3][ y * 16  ] = 0;
        }
    

    }

        
TileSet::~TileSet() {
    }


void TileSet::startDrawingWalls() {
    mNextTileToDraw = 0;
    }


static double scaleFactor = 1.0 / 16;

void TileSet::drawWall( doublePair inPosition ) {
    setDrawColor( 1, 1, 1, 1 );
    drawSprite( mWall[ mNextTileToDraw ], inPosition, scaleFactor );
    
    mNextTileToDraw = ( mNextTileToDraw + 1 ) % NUM_TILE_PATTERNS;
    }


void TileSet::drawWallEdges( doublePair inPosition, char inEdgeFlags ) {
    //setDrawColor( 1, 1, 1, 1 );

    for( int i=0; i<4; i++ ) {
        
        if( ( inEdgeFlags >> i ) & 0x01 ) {
            drawSprite( mWallEdges[i], inPosition, scaleFactor );
            }
        }
    }

    
        

        
void TileSet::drawFloor( doublePair inPosition ) {
    }

