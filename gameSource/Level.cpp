#include "Level.h"
#include "drawUtils.h"

#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>

extern CustomRandomSource randSource;


double maxEnemySpeed = 0.10;
double enemyBulletSpeed = 0.2;


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
    for( int i=0; i<4000; i++ ) {
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


    // place enemies in random floor spots

    for( y=0; y<MAX_LEVEL_H; y++ ) {
        for( x=0; x<MAX_LEVEL_W; x++ ) {
            if( mWallFlags[y][x] == 1 ) {    
                // floor
                
                // small chance of enemy
                if( randSource.getRandomBoundedInt( 0, 100 ) > 97 ) {
                    // hit

                    doublePair spot = { x - MAX_LEVEL_W/2,
                                        y - MAX_LEVEL_H/2 };

                    doublePair v = { 0, 0 };
                    doublePair a = { 0, 0 };
                    
                    Enemy e = { spot, v, a, 20, 
                                randSource.getRandomBoundedInt( 0, 10 ) };
                    
                    mEnemies.push_back( e );
                    }
                }
            
            }
        }
    
    // place rise marker in random floor spot
    char placed = false;

    while( !placed ) {
        int x = randSource.getRandomBoundedInt( 0, MAX_LEVEL_H - 1 );
        int y = randSource.getRandomBoundedInt( 0, MAX_LEVEL_W - 1 );
        
        if( mWallFlags[y][x] == 1 ) {
            placed = true;
            mRisePosition.x = x;
            mRisePosition.y = y;
            }
        }
    
    }



Level::~Level() {
    }


        
void Level::drawLevel( doublePair inViewCenter ) {
    int i;
    
    // step bullets
    for( i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );
        
        b->position = add( b->position, b->velocity );
        
        char hit = false;
        
        if( isWall( b->position ) ) {
            hit = true;
                        }
        else if( b->playerFlag ) {
            // check if hit enemy
            char hit = false;
            
            for( int j=0; j<mEnemies.size() && !hit; j++ ) {
                Enemy *e = mEnemies.getElement( j );
                
                if( distance( e->position, b->position ) < 0.4 ) {
                    hit = true;
                    mEnemies.deleteElement( j );
                    }
                }
            }


        if( hit ) {
            // bullet done
            mBullets.deleteElement( i );
            i--;
            }
        
        
        }

    // step enemies
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
    
        doublePair oldPos = e->position;
        e->position = stopMoveWithWall( e->position,
                                        e->velocity );
        
        // get actual velocity, taking wall collision into account
        e->velocity = sub( e->position, oldPos );
        
        e->velocity = add( e->velocity, e->accel );

        if( e->velocity.x > maxEnemySpeed ) {
            e->velocity.x = maxEnemySpeed;
            }
        else if( e->velocity.x < -maxEnemySpeed ) {
            e->velocity.x = -maxEnemySpeed;
            }

        if( e->velocity.y > maxEnemySpeed ) {
            e->velocity.y = maxEnemySpeed;
            }
        else if( e->velocity.y < -maxEnemySpeed ) {
            e->velocity.y = -maxEnemySpeed;
            }
        
        
        // random adjustment to acceleration
        e->accel.x = randSource.getRandomBoundedDouble( -0.05, 0.05 );
        e->accel.y = randSource.getRandomBoundedDouble( -0.05, 0.05 );

        
        if( e->stepsTilNextBullet == 0 ) {
            // fire bullet

            double playerDist = distance( inViewCenter, e->position );
            doublePair bulletVelocity = sub( inViewCenter, e->position );
            
            // normalize
            bulletVelocity.x /= playerDist;
            bulletVelocity.y /= playerDist;
            
            // set speed
            bulletVelocity.x *= enemyBulletSpeed;
            bulletVelocity.y *= enemyBulletSpeed;
            
            
            addBullet( e->position, bulletVelocity, false );
            

            e->stepsTilNextBullet = e->stepsBetweenBullets;
            }
        else {
            e->stepsTilNextBullet --;
            }
        
        }
    
    
    // draw walls and floor
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


    // draw rise marker
    setDrawColor( 1, 1, 0, 1 ); 
    doublePair riseSpot = { mRisePosition.x - MAX_LEVEL_W/2,
                            mRisePosition.y - MAX_LEVEL_H/2 };
    drawSquare( riseSpot, 0.5 );
    

    // draw bullets
    for( i=0; i<mBullets.size(); i++ ) {
        
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



    // draw enemies
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        
        setDrawColor( 0, 0, 0, 1 );
        drawSquare( e->position, 0.25 );
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


char Level::isRiseSpot( doublePair inPos ) {
    int x = (int)( rint( inPos.x ) );
    int y = (int)( rint( inPos.y ) );
    
    x += MAX_LEVEL_W/2;
    y += MAX_LEVEL_H/2;
    
    if( x < 0 || x >= MAX_LEVEL_W ||
        y < 0 || y >= MAX_LEVEL_H ) {
        // out of bounds
        return false;
        }
    
        
    return ( mRisePosition.x == x && mRisePosition.y == y );
    }




char Level::isEnemy( doublePair inPos ) {
    for( int j=0; j<mEnemies.size(); j++ ) {
        Enemy *e = mEnemies.getElement( j );
        
        if( distance( e->position, inPos ) < 0.4 ) {
            return true;
            }
        }
    return false;
    }



doublePair Level::stopMoveWithWall( doublePair inStart,
                                    doublePair inMoveDelta ) {

    doublePair newPos = inStart;
    
    double velocityX = inMoveDelta.x;
    double velocityY = inMoveDelta.y;
    
    newPos.x += velocityX;
    newPos.y += velocityY;


    if( isWall( newPos ) ) {
        doublePair xMoveAlone = inStart;
        xMoveAlone.x += velocityX;
        
        if( !isWall( xMoveAlone ) ) {
            
            // push y as close as possible to nearest wall
            
            int intY = (int)rint( xMoveAlone.y );
            if( velocityY > 0 ) {
                xMoveAlone.y = intY + 0.45;
                }
            else {
                xMoveAlone.y = intY - 0.45;
                }
            
            newPos = xMoveAlone;
            }
        else {
            // try y move alone
            doublePair yMoveAlone = inStart;
            yMoveAlone.y += velocityY;
        
            if( !isWall( yMoveAlone ) ) {
                
                // push x as close as possible to nearest wall
            
                int intX = (int)rint( yMoveAlone.x );
                if( velocityX > 0 ) {
                    yMoveAlone.x = intX + 0.45;
                    }
                else {
                    yMoveAlone.x = intX - 0.45;
                    }


                newPos = yMoveAlone;
                }
            else {
                // both hit
                newPos = inStart;
                }
            }
        }

    return newPos;
    }

        


void Level::addBullet( doublePair inPosition,
                       doublePair inVelocity, char inPlayerBullet ) {
    
    Bullet b = { inPosition, inVelocity, inPlayerBullet };
    mBullets.push_back( b );
    }

    

