#include "Level.h"
#include "drawUtils.h"

#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>

extern CustomRandomSource randSource;


Level::Level() {
    
    int x;
    int y;
    
    // blank all
    for( y=0; y<MAX_LEVEL_H; y++ ) {
        for( x=0; x<MAX_LEVEL_W; x++ ) {
            mWallFlags[y][x] = 0;
            }
        }
    

    // start in center
    x = MAX_LEVEL_W / 2;
    y = MAX_LEVEL_H / 2;
    

    // fill in floor first

    // random walk
    for( int i=0; i<1000; i++ ) {
        mWallFlags[y][x] = 1;

        // move only in x or y, not both
        if( randSource.getRandomBoolean() ) {
            x += randSource.getRandomBoundedInt( -1, 1 );
            }
        else {
            y += randSource.getRandomBoundedInt( -1, 1 );
            }
        
        if( x >= MAX_LEVEL_W - 2 ) {
            x = MAX_LEVEL_W - 3;
            }
        if( x < 2 ) {
            x = 2;
            }
        if( y >= MAX_LEVEL_H - 2 ) {
            y = MAX_LEVEL_H - 3;
            }
        if( y < 2 ) {
            y = 3;
            }    
        }

    // now walls around floor
    // set loop boundaries so it's safe to check neighbors
    for( y=1; y<MAX_LEVEL_H - 1; y++ ) {
        for( x=1; x<MAX_LEVEL_W - 1; x++ ) {
         
            if( mWallFlags[y][x] == 0 ) {
                
                char floorNeighbor = false;
                
                for( int dy=-1; dy<=1 && !floorNeighbor; dy++ ) {
                    for( int dx=-1; dx<=1 && !floorNeighbor; dx++ ) {
                        
                        if( mWallFlags[y+dy][x+dx] == 1 ) {
                            floorNeighbor = true;
                            }
                        }
                    }
                

                if( floorNeighbor ) {
                    mWallFlags[y][x] = 2;
                    }
                }
            }
        }
    }



Level::~Level() {
    }


        
void Level::drawLevel( doublePair inViewCenter ) {

    // step bullets
    for( int i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );
        
        b->position = add( b->position, b->velocity );
        if( isWall( b->position ) ) {
            // bullet done
            mBullets.deleteElement( i );
            i--;
            }
        }
    

    for( int y=0; y<MAX_LEVEL_H; y++ ) {
        for( int x=0; x<MAX_LEVEL_W; x++ ) {
            

            if( mWallFlags[y][x] != 0 ) {
                doublePair spot = { x - MAX_LEVEL_W/2,
                                    y - MAX_LEVEL_H/2 };
                
                
                if( mWallFlags[y][x] == 1 ) {
                    // draw floor        
                    setDrawColor( 0.5, 0.5, 0.5, 1 );                    
                    }
                else if( mWallFlags[y][x] == 2 ) {
                    // wall
                    setDrawColor( 0.25, 0.25, 0.25, 1 );
                    }
                
                drawSquare( spot, 0.5 );
                }
            
            }
        }

    // draw bullets
    for( int i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );

        // border
        setDrawColor( 0, 0, 0, 1 );
        drawSquare( b->position, 0.1 );

        // fill
        if( b->playerFlag ) {
            setDrawColor( 0, 1, 0, 1 );
            }
        else {
            setDrawColor( 1, 0, 0, 1 );
            }
        drawSquare( b->position, 0.05 );
        }
    
    }



char Level::isWall( doublePair inPos ) {
    int x = (int)( rint( inPos.x ) );
    int y = (int)( rint( inPos.y ) );
    
    x += MAX_LEVEL_W/2;
    y += MAX_LEVEL_H/2;
    
    if( x < 0 || x >= MAX_LEVEL_W ||
        y < 0 || y >= MAX_LEVEL_H ) {
        // out of bounds
        return false;
        }
    
        
    return ( mWallFlags[y][x] == 2 );
    }



void Level::addBullet( doublePair inPosition,
                       doublePair inVelocity, char inPlayerBullet ) {
    
    Bullet b = { inPosition, inVelocity, inPlayerBullet };
    mBullets.push_back( b );
    }

    

