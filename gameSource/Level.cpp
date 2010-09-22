#include "Level.h"
#include "drawUtils.h"
#include "fixedSpriteBank.h"

#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>

extern CustomRandomSource randSource;


double maxEnemySpeed = 0.10;
double enemyBulletSpeed = 0.2;


Level::Level() {

    mFrozen = false;
    mWindowSet = false;
    
    mMousePos.x = 0;
    mMousePos.y = 0;
    mPlayerPos.x = 0;
    mPlayerPos.y = 0;
    mEnteringMouse = false;
    

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
                    
                    // keep enemies away from player starting spot (fair)

                    doublePair playerSpot = {0,0};
                    
                    if( distance( spot, playerSpot ) > 20 ) {
                    
                        doublePair v = { 0, 0 };
                        doublePair a = { 0, 0 };
                        
                        Enemy e = { spot, v, a, 20, 
                                    randSource.getRandomBoundedInt( 0, 10 ) };
                        
                        mEnemies.push_back( e );
                        }
                    
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



void Level::setPlayerPos( doublePair inPos ) {
    mPlayerPos = inPos;
    }


void Level::setMousePos( doublePair inPos ) {
    mMousePos = inPos;
    }

void Level::setEnteringMouse( char inEntering ) {
    mEnteringMouse = inEntering;
    }




void Level::setItemWindowPosition( doublePair inPosition ) {
    int index;
    char found = isEnemy( inPosition, &index );
    
    if( found ) {
        mWindowSet = true;
        mWindowPosition.index = index;
        mWindowPosition.isPlayer = false;
        }
    else if( isPlayer( inPosition ) ) {
        mWindowSet = true;
        mWindowPosition.isPlayer = true;
        }
    }



void Level::step() {
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

            double playerDist = distance( mPlayerPos, e->position );
            doublePair bulletVelocity = sub( mPlayerPos, e->position );
            
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
    }



void Level::drawMouse( double inFade ) {
    // reticle
    if( mEnteringMouse ) {
        setDrawColor( 1, 1, 1, 1 * inFade );
        }
    else {
        setDrawColor( 0, 1, 0, 0.5 * inFade );
        }
    
    drawSquare( mMousePos, 0.125 );

    setDrawColor( 0, 0, 0, 0.5 * inFade );

    drawSquare( mMousePos, 0.025 );
    }



void Level::drawPlayer( double inFade ) {
    // player
    setDrawColor( 1, 0, 0, inFade );
    drawSquare( mPlayerPos, 0.25 );
    }


        
void Level::drawLevel() {
    
    if( !mFrozen ) {
        step();
        }
    
    int i;
    
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
    doublePair riseSpot = { mRisePosition.x - MAX_LEVEL_W/2,
                            mRisePosition.y - MAX_LEVEL_H/2 };
    /*
    setDrawColor( 1, 1, 0, 1 ); 
    drawSquare( riseSpot, 0.5 );
    */
    setDrawColor( 1, 1, 1, 1 );
    drawSprite( riseMarker, riseSpot );
    


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
    
    



    if( mWindowSet && ! mWindowPosition.isPlayer ) {
        startAddingToStencil( false, true );
        Enemy *e = mEnemies.getElement( mWindowPosition.index );
        drawSquare( e->position, 0.2 );
        }
    else if( mWindowSet && mWindowPosition.isPlayer ) {
        drawMouse( 1 );
        drawPlayer( 1 );
        
        // player is window for zoom
        startAddingToStencil( false, true );
        drawSquare( mPlayerPos, 0.2 );
        }
    


    // draw player and mouse
    
    if( !mWindowSet ) {
        drawMouse( 1 );
        drawPlayer( 1 );
        }
    



    if( mWindowSet ) {
        startDrawingThroughStencil();
        }
    
    
    }



void Level::drawWindowShade( double inFade ) {
    if( mWindowSet ) {
        stopStencil();
        
        if( mWindowPosition.isPlayer ) {
            setDrawColor( 1, 0, 0, inFade );
            drawSquare( mPlayerPos, 0.2 );
            }
        else {    
            Enemy *e = mEnemies.getElement( mWindowPosition.index );
            setDrawColor( 0, 0, 0, inFade );
            drawSquare( e->position, 0.2 );

            drawMouse( inFade );
            drawPlayer( inFade );
            }
        
        }
    }



void Level::forgetItemWindow() {
    mWindowSet = false;
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



doublePair Level::getEnemyCenter( int inEnemyIndex ) {
    Enemy *e = mEnemies.getElement( inEnemyIndex );
    return e->position;
    }




char Level::isEnemy( doublePair inPos, int *outEnemyIndex ) {
    for( int j=0; j<mEnemies.size(); j++ ) {
        Enemy *e = mEnemies.getElement( j );
        
        if( distance( e->position, inPos ) < 0.4 ) {

            if( outEnemyIndex != NULL ) {
                *outEnemyIndex = j;
                }
            return true;
            }
        }
    return false;
    }



char Level::isPlayer( doublePair inPos ) {
    return ( distance( mPlayerPos, inPos ) < 0.4 );
    }



void Level::freezeLevel( char inFrozen ) {
    mFrozen = inFrozen;
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

    

