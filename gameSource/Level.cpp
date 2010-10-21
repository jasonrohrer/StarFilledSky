#include "Level.h"
#include "drawUtils.h"
#include "fixedSpriteBank.h"
#include "powerUpProperties.h"
#include "bulletSizeSet.h"

#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>


char Level::sGridWorldSpotsComputed;
doublePair Level::sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];

extern CustomRandomSource randSource;


double maxEnemySpeed = 0.10;
double enemyBulletSpeed = 0.2;






static int getEnemyMaxHealth( PowerUpSet *inSet ) {
    return 1 + getMaxHealth( inSet );
    }





void Level::generateReproducibleData() {

    if( mDataGenerated ) {
        // already generated
        return;
        }



    // dynamically allocate base arrays

    //char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
    //short mSquareIndices[MAX_LEVEL_H][MAX_LEVEL_W];
    
    mWallFlags = new char*[MAX_LEVEL_H];
    mSquareIndices = new short*[MAX_LEVEL_H];

    int x, y;
    
    for( y=0; y<MAX_LEVEL_H; y++ ) {
        mWallFlags[y] = new char[MAX_LEVEL_W];
        mSquareIndices[y] = new short[MAX_LEVEL_W];

        // blank all
        memset( mWallFlags[y], 0, MAX_LEVEL_W );
        
        // no need to blank square indices, because we never use
        // a non-existent y,x pair to index mSquareIndices
        }
    
    //char mFloorEdgeFlags[MAX_LEVEL_SQUARES];
    mFloorEdgeFlags = new char[MAX_LEVEL_SQUARES];
    mIndexToGridMap = new GridPos[MAX_LEVEL_SQUARES];
    

    

    randSource.restoreFromSavedState( mRandSeedState );
    

    // start in center
    x = MAX_LEVEL_W / 2;
    y = MAX_LEVEL_H / 2;
    

    // fill in floor first
    int floorColorIndex = 0;


    mNumUsedSquares = 0;
    
    mNumFloorSquares = 0;
    mNumWallSquares = 0;
    
    SimpleVector<Color> gridColorsWorking;

    int xLimit = 2;
    int numFloorSquaresMax = MAX_FLOOR_SQUARES;
    int stepLimit = 4000;
    if( mSymmetrical ) {
        // stop at center
        xLimit = x;
        
        // generate half squares
        numFloorSquaresMax /= 2;
        stepLimit /= 2;
        }
    
    

    
    // random walk with buffer from grid edge
    // limit in number of random steps taken (for time) or
    // number of floor squares generated
    for( int i=0; i<stepLimit && mNumFloorSquares < numFloorSquaresMax; i++ ) {
        if( mWallFlags[y][x] != 1 ) {
            mNumFloorSquares++;

            
            mSquareIndices[y][x] = mNumUsedSquares;
            mIndexToGridMap[mNumUsedSquares].x = x;
            mIndexToGridMap[mNumUsedSquares].y = y;
            
            mNumUsedSquares++;
            
            gridColorsWorking.push_back(
                mColors.secondary.elements[floorColorIndex] );

            floorColorIndex = (floorColorIndex + 1) % 3;
            }
        
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
        if( x < xLimit ) {
            x = xLimit;
            }
        if( y >= MAX_LEVEL_H - 2 ) {
            y = MAX_LEVEL_H - 3;
            }
        if( y < 2 ) {
            y = 3;
            }    
        }


    if( mSymmetrical ) {
        int floorSquaresFirstHalf = mNumFloorSquares;
        
        for( int i=0; i<floorSquaresFirstHalf; i++ ) {
            
            int x = mIndexToGridMap[i].x;
            int y = mIndexToGridMap[i].y;
                            
            int copyX = MAX_LEVEL_W - x - 1;
                
            mNumFloorSquares++;

            mSquareIndices[y][copyX] = mNumUsedSquares;
            mIndexToGridMap[mNumUsedSquares].x = copyX;
            mIndexToGridMap[mNumUsedSquares].y = y;
            
            mNumUsedSquares++;
                    
            // copy colors symmetrically too
            Color copyColor =
                *( gridColorsWorking.getElement( 
                       mSquareIndices[y][x] ) );
                    
            gridColorsWorking.push_back( copyColor );
            
            mWallFlags[y][copyX] = 1;
            }
        }
    
    


    // now walls around floor
    // set loop boundaries so it's safe to check neighbors
    int wallColorIndex = 0;

    mNumWallSquares = 0;
    
    int xStart = 1;
    if( mSymmetrical ) {
        xStart = MAX_LEVEL_W/2;
        }

    // opt:  only look at floor squares, instead of checking all squares
    for( int i=0; i<mNumFloorSquares; i++ ) {
        
        int x = mIndexToGridMap[i].x;
        int y = mIndexToGridMap[i].y;

        // check if on proper side of symm line
        if( y >= 1 && y < MAX_LEVEL_H - 1
            &&
            x >= xStart && x < MAX_LEVEL_W - 1 ) {
                
            int nxStart = x - 1;
            if( x == xStart && mSymmetrical ) {
                // avoid sticking walls across symm line
                nxStart = x;
                }
                
            for( int ny=y-1; ny<=y+1; ny++ ) {
                for( int nx=nxStart; nx<=x+1; nx++ ) {
                        
                    if( mWallFlags[ny][nx] == 0 ) {
                        // empty spot adjacent to this floor square

                        mWallFlags[ny][nx] = 2;
                                                
                        mSquareIndices[ny][nx] = mNumUsedSquares;
                        mIndexToGridMap[mNumUsedSquares].x = nx;
                        mIndexToGridMap[mNumUsedSquares].y = ny;

                        mNumUsedSquares++;

                        gridColorsWorking.push_back( 
                            mColors.primary.elements[wallColorIndex] );
                        wallColorIndex = (wallColorIndex + 1) % 3;
                    
                        mNumWallSquares ++;
                        }
                    }
                }
            }
        }




    if( mSymmetrical ) {
        
        int wallSquaresFirstHalf = mNumWallSquares;
        
        for( int i=mNumFloorSquares; 
             i<mNumFloorSquares + wallSquaresFirstHalf; i++ ) {
            
            int x = mIndexToGridMap[i].x;
            int y = mIndexToGridMap[i].y;
                
            int copyX = MAX_LEVEL_W - x - 1;
            
            mNumWallSquares++;

            mSquareIndices[y][copyX] = mNumUsedSquares;
            mIndexToGridMap[mNumUsedSquares].x = copyX;
            mIndexToGridMap[mNumUsedSquares].y = y;
                    
            mNumUsedSquares++;
                    
            // copy colors symmetrically too
            Color copyColor =
                *( gridColorsWorking.getElement( 
                       mSquareIndices[y][x] ) );
                    
            gridColorsWorking.push_back( copyColor );
                    
            mWallFlags[y][copyX] = 2;
            }
        }
    



    // make indexed versions of these for quick looping later
    mWallFlagsIndexed = new char[mNumUsedSquares];
    mGridWorldSpots = new doublePair*[mNumUsedSquares];

    for( int i=0; i<mNumUsedSquares; i++ ) {
        int x = mIndexToGridMap[i].x;
        int y = mIndexToGridMap[i].y;

        mGridWorldSpots[ i ] = &( sGridWorldSpots[y][x] );

        mWallFlagsIndexed[ i ] = mWallFlags[y][x];
        }






    mHardGridColors = gridColorsWorking.getElementArray();
        
    

    Color mGridColorsBlurred[MAX_LEVEL_SQUARES];

    // blur all grid colors
    
    #define R  1

    for( int i=0; i<mNumUsedSquares; i++ ) {        
        char thisWallFlag = mWallFlagsIndexed[i];
        float cSums[3] = { 0, 0, 0 };
        
        int numInSum = 0;
        
        int x = mIndexToGridMap[i].x;
        int y = mIndexToGridMap[i].y;
        

        for( int dy = -R; dy <= R; dy++ ) {
            for( int dx = -R; dx <= R; dx++ ) {
                
                if( mWallFlags[ y + dy ][ x + dx ] == thisWallFlag ) {
                    
                    Color *c = 
                        &( mHardGridColors[ 
                               mSquareIndices[y+dy][x+dx] ] );
                    
                    cSums[0] += c->r;
                    cSums[1] += c->g;
                    cSums[2] += c->b;
                    
                    numInSum ++;
                    }
                }
            }
                
        for( int i=0; i<3; i++ ) {
            mGridColorsBlurred[ mSquareIndices[y][x] ][i] = 
                cSums[i] / numInSum;
            }
        
        }

    // copy over
    mSoftGridColors = new Color[ mNumUsedSquares ];
 
    for( int i=0; i<mNumUsedSquares; i++ ) {        
       mSoftGridColors[ i ] = mGridColorsBlurred[ i ];
        }

    // actual, working grid colors
    mGridColors = new Color[ mNumUsedSquares ];
    memcpy( mGridColors, mSoftGridColors, sizeof( Color ) * mNumUsedSquares );
    
    mColorMix = new float[ mNumUsedSquares ];
    mColorMixDelta = new float[ mNumUsedSquares ];
    for( int i=0; i<mNumUsedSquares; i++ ) {
        mColorMix[i] = 0;
        mColorMixDelta[i] = randSource.getRandomBoundedDouble( 0.04, 0.08 );
        }


    



    


    // now compute which walls should have edges
    // don't bother filling at all for non-floor squares
    for( int i=0; i<mNumUsedSquares; i++ ) {
        if( mWallFlagsIndexed[i] == 1 ) {
            // floor here
            int x = mIndexToGridMap[ i ].x;
            int y = mIndexToGridMap[ i ].y;
            
            
            char flag = 0;
            if( mWallFlags[y + 1][x] != 1 ) {
                flag |= 0x01;
                }
            if( mWallFlags[y][x+1] != 1 ) {
                flag |= 0x02;
                }
            if( mWallFlags[y - 1][x] != 1 ) {
                flag |= 0x04;
                }
            if( mWallFlags[y][x-1] != 1 ) {
                flag |= 0x08;
                }

            mFloorEdgeFlags[i] = flag;                
            }
        }
    

    
    mDataGenerated = true;
    }




void Level::freeReproducibleData() {
    if( mDataGenerated ) {
        

        for( int i=0; i<MAX_LEVEL_H; i++ ) {
            delete [] mWallFlags[i];
            delete [] mSquareIndices[i];
            }
        delete [] mWallFlags;
        delete [] mSquareIndices;
        delete [] mFloorEdgeFlags;

    
        delete [] mGridColors;
        delete [] mSoftGridColors;
        delete [] mHardGridColors;
        delete [] mColorMix;
        delete [] mColorMixDelta;
        delete [] mWallFlagsIndexed;
        delete [] mIndexToGridMap;
        delete [] mGridWorldSpots;

        mDataGenerated = false;
        }
    }



//#include "minorGems/system/Thread.h"



Level::Level( ColorScheme *inPlayerColors, ColorScheme *inColors, 
              int inLevelNumber, char inSymmetrical ) 
        : mLevelNumber( inLevelNumber ), 
          mPlayerSprite( inPlayerColors ),
          mPlayerPowers( new PowerUpSet( inLevelNumber - 1 ) ) {

    int health, max;
    getPlayerHealth( &health, &max );
    mPlayerHealth = max;
    
    
    
    //Thread::staticSleep( 1000 );
    

    if( !sGridWorldSpotsComputed ) {
        
        // precompute to-world coord mapping
        for( int y=0; y<MAX_LEVEL_H; y++ ) {
            for( int x=0; x<MAX_LEVEL_W; x++ ) {
                
                sGridWorldSpots[y][x].x = x - MAX_LEVEL_W/2;
                sGridWorldSpots[y][x].y = y - MAX_LEVEL_H/2;
                }
            }
        sGridWorldSpotsComputed = true;
        }


    
    randSource.saveState();
    mRandSeedState = randSource.getSavedState();
    
    mDataGenerated = false;
    mSymmetrical = inSymmetrical;

    if( inColors != NULL ) {
        // copy
        mColors = *( inColors );
        }
    // else use randomly-generated mColors from stack


    mFrozen = false;
    mDrawFloorEdges = true;
    mEdgeFadeIn = 0.0f;
    
    mWindowSet = false;
    
    if( mSymmetrical ) {
        mMousePos.x = -0.5;
        mMousePos.y = 0;
        mPlayerPos.x = -0.5;
        mPlayerPos.y = 0;
        }
    else {
        mMousePos.x = 0;
        mMousePos.y = 0;
        mPlayerPos.x = 0;
        mPlayerPos.y = 0;
        }
    
    mEnteringMouse = false;
    

    
    generateReproducibleData();
    
    

    // place enemies in random floor spots

    for( int i=0; i<10; i++ ) {
        
        // pick random floor spot until found one away from player
        
        int floorPick = 
            randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
        
        char hit = false;
        
        int numTries = 0;
        
        while( ! hit && numTries < 20 ) {
            numTries++;
            
            doublePair spot = *( mGridWorldSpots[ floorPick ] );
                    
            // keep enemies away from player starting spot (fair)

            doublePair playerSpot = {0,0};
            
            if( distance( spot, playerSpot ) > 20 ) {
                
                doublePair v = { 0, 0 };
                doublePair a = { 0, 0 };
                

                PowerUpSet *p = new PowerUpSet( mLevelNumber - 1 );
                
                Enemy e = { spot, v, a, 20, 
                            randSource.getRandomBoundedInt( 0, 10 ),
                            new EnemySprite(),
                            p,
                            getEnemyMaxHealth( p ) };
                        
                mEnemies.push_back( e );
                hit = true;
                }
            else {
                // try new pick
                floorPick = 
                    randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
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
    

    
    int powerUpMaxLevel = mLevelNumber / POWER_SET_SIZE;

    for( int i=0; i<10; i++ ) {

        // pick random floor spot until found one not on rise marker
        // or existing power token
        
        int floorPick = 
            randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
        
        char hit = false;
        
        int numTries = 0;
        
        while( ! hit && numTries < 20 ) {
            numTries++;

            
            GridPos pickPos = mIndexToGridMap[ floorPick ];

            if( mRisePosition.x != pickPos.x
                &&
                mRisePosition.y != pickPos.y ) {
                
                
                hit = true;
                
                int numExisting = mPowerUpTokens.size();
                
                for( int j=0; j<numExisting && hit; j++ ) {
                    PowerUpToken *p = mPowerUpTokens.getElement( j );
                    
                    if( p->gridPosition.x == pickPos.x
                        &&
                        p->gridPosition.y == pickPos.y ) {
                        
                        hit = false;
                        }
                    }
                }

            if( hit ) {
                
                doublePair worldPos = 
                    sGridWorldSpots[ pickPos.y ][ pickPos.x ];

                PowerUp mainPower = getRandomPowerUp( powerUpMaxLevel );
                

                // powers must sum to main power
                PowerUpSet *subPowers = new PowerUpSet( mainPower.level,
                                                        mainPower.powerType );
                

                char startedEmpty = ( mainPower.powerType == powerUpEmpty );
                
                PowerUpToken t = { mainPower,
                                   startedEmpty,
                                   pickPos,
                                   worldPos,
                                   new PowerUpSprite( mainPower, 
                                                      subPowers,
                                                      startedEmpty ),
                                   subPowers };
                
                mPowerUpTokens.push_back( t );
                }
            else {
                // try new pick
                floorPick = 
                    randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
                }
            }
        
        }



    mLastEnterPointSprite = &mPlayerSprite;
    mLastEnterPointPowers = mPlayerPowers;
    
    mLastEnterPointPowerTokenIndex = -1;
    }



Level::~Level() {
    freeReproducibleData();


    for( int i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        delete e->sprite;
        delete e->powers;
        }

    for( int i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( i );
        delete t->sprite;
        delete t->subPowers;
        }

    delete mPlayerPowers;
    }



void Level::compactLevel() {
    freeReproducibleData();
    }

        
void Level::decompactLevel() {
    generateReproducibleData();
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




void Level::setItemWindowPosition( doublePair inPosition, itemType inType ) {
    int index;

    if( inType == enemy && isEnemy( inPosition, &index ) ) {
        mWindowSet = true;
        mWindowPosition.index = index;
        mWindowPosition.type = enemy;
        }
    else if( inType == power && isPowerUp( inPosition, &index ) ) {
        mWindowSet = true;
        mWindowPosition.index = index;
        mWindowPosition.type = power;
        }
    else if( inType == player && isPlayer( inPosition ) ) {
        mWindowSet = true;
        mWindowPosition.type = player;
        }
    }



void Level::step() {
    int i;
    
    // step bullets
    for( i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );
        
        doublePair adjustedVelocity = b->velocity;
        
        if( b->heatSeek > 0 ) {
            
            // vector toward closest target
            doublePair closestTarget = mPlayerPos;
            if( b->playerFlag ) {
                
                // search for closest enemy to waypoint 
                // (closest to reticle at time of firing)
                
                double minDistance = DBL_MAX;
                int minIndex = -1;
                for( int j=0; j<mEnemies.size(); j++ ) {
                    Enemy *e = mEnemies.getElement( j );
                    
                    double dist = distance( e->position, b->heatSeekWaypoint );
                    if( dist < minDistance ) {
                        minDistance = dist;
                        minIndex = j;
                        }
                    }
                
                Enemy *e = mEnemies.getElement( minIndex );
                closestTarget = e->position;                        
                }
            
            doublePair vectorToTarget = normalize( sub( closestTarget, 
                                                        b->position ) );
            adjustedVelocity = normalize( adjustedVelocity );
            
            //b->heatSeek = 0.1;
            
            // how much to weight heat seek tendency
            vectorToTarget = mult( vectorToTarget, b->heatSeek );
            
            // how much to weight forward-velocity tendency 
            adjustedVelocity = mult( adjustedVelocity, 1 - b->heatSeek );
            
            adjustedVelocity = add( adjustedVelocity, vectorToTarget );
            
            // maintain bullet speed after we've picked a direction
            adjustedVelocity = normalize( adjustedVelocity );
            adjustedVelocity = mult( adjustedVelocity, b->speed );
            b->velocity = adjustedVelocity;
            }
        


        b->position = add( b->position, adjustedVelocity );        
        
        GridPos p = getGridPos( b->position );


        char hit = false;


        // first make sure it's in bounds of sparse world tiles
        
        if( mWallFlags[p.y][p.x] == 0 ) {
            // bullet has escaped out of bounds, kill it
            hit = true;
            }
        else {
            // in bounds of tiles, safe to look up square index
            int squareIndex = mSquareIndices[p.y][p.x];

            // light up square passing over (or wall hit)

            mColorMix[ squareIndex ] = 1;

            if( isWall( b->position ) ) {
                hit = true;

                // jump to hard when hit, then fade out
                mGridColors[ squareIndex ] = mHardGridColors[ squareIndex ];
                }
            else {
                // color floor

                // more subtle than wall hit.... jump to soft?
                mGridColors[ squareIndex ] = mSoftGridColors[ squareIndex ];
            

                float hitRadius = 0.5 + b->size / 16;
            
                if( b->playerFlag ) {
                    // check if hit enemy
                                    
                    
                    for( int j=0; j<mEnemies.size() && !hit; j++ ) {
                        Enemy *e = mEnemies.getElement( j );
                    
                        if( distance( e->position, b->position ) < 
                            hitRadius  ) {
                            
                            hit = true;

                            // make sure enemy health is up-to-date
                            // (its power-ups may have been modified)
                            int maxHealth = 
                                getEnemyMaxHealth( e->powers );
                        
                            if( e->health > maxHealth ) {
                                e->health = maxHealth;
                                }

                            e->health --;
                            if( e->health == 0 ) {
                                delete e->sprite;
                                delete e->powers;
                                
                                mEnemies.deleteElement( j );
                                }
                            else {
                                // redisplay health bar
                                e->healthBarFade = 1;
                                }
                            }
                        }
                    }
                else {
                    // check if hit player
                    if( distance( mPlayerPos, b->position ) < hitRadius ) {
                        hit = true;
                        mPlayerHealth -= 1;
                        if( mPlayerHealth < 0 ) {
                            mPlayerHealth = 0;
                            }
                        }
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

            float accuracy = getAccuracy( e->powers );

            // set speed
            // enemy bullets are slower than equivalent player bullets
            float bulletSpeed = getBulletSpeed( e->powers ) / 2;
            
            
            addBullet( e->position, mPlayerPos, accuracy,
                       getSpread( e->powers ),
                       getHeatSeek( e->powers ),
                       mPlayerPos,
                       bulletSpeed, false, i );
            

            //e->stepsTilNextBullet = e->stepsBetweenBullets;
            e->stepsTilNextBullet = getStepsBetweenBullets( e->powers );
            }
        else {
            e->stepsTilNextBullet --;
            }
        
        if( e->healthBarFade > 0 ) {
            e->healthBarFade -= 0.03;
            if( e->healthBarFade < 0 ) {
                e->healthBarFade = 0;
                }
            }
        }


    // step square colors
    for( int i=0; i<mNumUsedSquares; i++ ) {
        mColorMix[i] += mColorMixDelta[i];
        
        if( mColorMix[i] > 1 ) {
            mColorMix[i] = 1;
            mColorMixDelta[i] *= -1;
            }
        else if( mColorMix[i] < 0 ) {
            mColorMix[i] = 0;
            mColorMixDelta[i] *= -1;
            }
        
        // never go fully hard
        float mix = mColorMix[i] * 0.2;
        float counterMix = 1 - mix;

        // average our grid color with the current target mix
        mix *= 0.05;
        counterMix *= 0.05;
        mGridColors[i].r += 
            mHardGridColors[i].r * mix 
            + mSoftGridColors[i].r * counterMix;
        mGridColors[i].g += 
            mHardGridColors[i].g * mix 
            + mSoftGridColors[i].g * counterMix;
        mGridColors[i].b += 
            mHardGridColors[i].b * mix 
            + mSoftGridColors[i].b * counterMix;
        
        
        mGridColors[i].r /= 1.05;
        mGridColors[i].g /= 1.05;
        mGridColors[i].b /= 1.05;
        }

    }



void Level::drawMouse( double inFade ) {
    // reticle
    setDrawColor( 1, 1, 1, 0.5 * inFade );
    if( mEnteringMouse ) {
        drawSprite( enterCrosshair, mMousePos );
        }
    else {
        drawSprite( crosshair, mMousePos );
        }
    }



void Level::drawPlayer( double inFade ) {
    // player
    //setDrawColor( 1, 0, 0, inFade );
    //drawSquare( mPlayerPos, 0.25 );
    mPlayerSprite.draw( mPlayerPos, inFade );
    }


        
void Level::drawLevel( doublePair inViewCenter, double inViewSize ) {
    
    if( !mFrozen ) {
        step();
        }
    else {
        // frozen, but keep any token that was sub-level entry point updated
        
        if( mLastEnterPointPowerTokenIndex != -1 ) {
            PowerUpToken *t = mPowerUpTokens.getElement( 
                mLastEnterPointPowerTokenIndex );
            
            if( t->startedEmpty ) {
                t->power.powerType = t->subPowers->getMajorityType();
                }

            t->power.level = t->subPowers->getLevelSum( t->power.powerType );
            }
        }
    
        


    int i;
    
    //mTileSet.startDrawingWalls();


    doublePair riseSpot = { mRisePosition.x - MAX_LEVEL_W/2,
                            mRisePosition.y - MAX_LEVEL_H/2 };


    // opt:  don't draw whole grid, just visible part
    
    int yVisStart = (int)( inViewCenter.y - inViewSize / 2 + MAX_LEVEL_H / 2 );
    int yVisEnd = (int)( inViewCenter.y + inViewSize / 2 + MAX_LEVEL_H / 2 );
    
    // bit extra
    yVisStart --;
    yVisEnd ++;

    if( yVisStart < 0 ) {
        yVisStart = 0;
        }
    if( yVisEnd >= MAX_LEVEL_H ) {
        yVisEnd = MAX_LEVEL_H - 1;
        }

    int xVisStart = (int)( inViewCenter.x - inViewSize / 2 + MAX_LEVEL_W / 2 );
    int xVisEnd = (int)( inViewCenter.x + inViewSize / 2 + MAX_LEVEL_W / 2 );
    
    // bit extra
    xVisStart --;
    xVisEnd ++;

    if( xVisStart < 0 ) {
        xVisStart = 0;
        }
    if( xVisEnd >= MAX_LEVEL_W ) {
        xVisEnd = MAX_LEVEL_W - 1;
        }
    


    // draw walls
    for( int y=yVisStart; y<=yVisEnd; y++ ) {
        for( int x=xVisStart; x<=xVisEnd; x++ ) {
            if( mWallFlags[y][x] == 2 ) {
                Color *c = &( mGridColors[mSquareIndices[y][x]] );
                
                setDrawColor( c->r,
                              c->g,
                              c->b, 1 );
            
                drawSquare( sGridWorldSpots[y][x], 0.5 );
                }
            }
        }
    
    
    if( mDrawFloorEdges ) {
        Color c = mColors.primary.elements[3];
        setDrawColor( c.r,
                      c.g,
                      c.b, 1 );
        
        double maxDistance = 5;
        
        double dist = distance( mPlayerPos, riseSpot );
        
        if( mEdgeFadeIn >= 1 && dist > maxDistance ) {
            
            // draw floor edges
            for( int y=yVisStart; y<=yVisEnd; y++ ) {
                for( int x=xVisStart; x<=xVisEnd; x++ ) {
                    if( mWallFlags[y][x] == 1 &&
                        mFloorEdgeFlags[mSquareIndices[y][x]] != 0 ) {
                        
                        drawSquare( sGridWorldSpots[y][x], 0.5625 );
                        }
                    }
                }
            }
        else {
            
            double fade = mEdgeFadeIn;
            
            if( dist <= maxDistance ) {
                if( dist > 1 ) {
                    fade = (dist - 1) / (maxDistance-1);

                    if( fade > mEdgeFadeIn ) {
                        // not done fading in yet, don't let
                        // "near rise marker" fade trump fade-in
                        fade = mEdgeFadeIn;
                        }
                    }
                else {
                    fade = 0;
                    }
                }
            

            // use stencil to draw transparent floor edge w/out overlap 
            // artifacts
        
            startAddingToStencil( false, true );
            
            for( int y=yVisStart; y<=yVisEnd; y++ ) {
                for( int x=xVisStart; x<=xVisEnd; x++ ) {
                    if( mWallFlags[y][x] == 1 &&
                        mFloorEdgeFlags[mSquareIndices[y][x]] != 0 ) {
                        
                        drawSquare( sGridWorldSpots[y][x], 0.5625 );
                        }
                    }
                }
            
            startDrawingThroughStencil();
            
            setDrawColor( c.r,
                          c.g,
                          c.b, fade );
            
            drawRect( inViewCenter.x - inViewSize/2, 
                      inViewCenter.y - inViewSize/2, 
                      inViewCenter.x + inViewSize/2, 
                      inViewCenter.y + inViewSize/2 );
            
            
            if( mEdgeFadeIn < 1 ) {    
                mEdgeFadeIn += 0.01;
                }
            
            stopStencil();
            }
        
        }
    
    // draw floor
    for( int y=yVisStart; y<=yVisEnd; y++ ) {
        for( int x=xVisStart; x<=xVisEnd; x++ ) {
            if( mWallFlags[y][x] == 1 ) {
                Color *c = &( mGridColors[mSquareIndices[y][x]] );
                
                setDrawColor( c->r,
                              c->g,
                              c->b, 1 );
                
                drawSquare( sGridWorldSpots[y][x], 0.5 );
                }
            }
        }
    

    // draw rise marker
    // color same as floor tile
    /*Color *c = 
        &( mGridColors[mSquareIndices[mRisePosition.y][mRisePosition.x]] );
    */
    Color *c = &( mColors.special );
    setDrawColor( c->r,
                  c->g,
                  c->b, 1 );
    drawSprite( riseMarker, riseSpot );


    // draw power-ups
    for( i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *p = mPowerUpTokens.getElement( i );
        
        drawPowerUp( p->power, p->position );        
        }
    
    

    // draw bullets
    for( i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );

        
        if( b->playerFlag ) {
            setDrawColor( 1, 1, 1, 1 );
            }
        else {
            setDrawColor( 0, 0, 0, 1 );
            }
        drawBullet( b->size, b->position );
        }



    // draw enemies
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        e->sprite->draw( e->position, 1 );
        
        if( e->healthBarFade > 0 ) {
            
            doublePair pos = e->position;
            
            // hold at full vis until half-way through fade
            float fade = 1;

            if( e->healthBarFade < 0.5 ) {
                // from held at 1 to sin fade out
                fade = sin( e->healthBarFade * M_PI );
                }
            

            setDrawColor( 0.25, 0.25, 0.25, fade );
            drawRect( pos.x - 0.5, pos.y + 0.5, 
                      pos.x + 0.5, pos.y + 0.25 );
            
            float healthFraction = e->health / 
                (float)getEnemyMaxHealth( e->powers );
            
            setDrawColor( 0, 0, 0, fade );
            drawRect( pos.x - 0.4375 + 0.875 * healthFraction, 
                      pos.y + 0.4375, 
                      pos.x + 0.4375, 
                      pos.y + 0.3125 );
            
    
            setDrawColor( 1, 0, 0, fade );
            drawRect( pos.x -0.4375, 
                      pos.y + 0.4375,
                      pos.x - 0.4375 + 0.875 * healthFraction, 
                      pos.y + 0.3125 );
            }
        }
    
    



    // window for zoom
    // use whole sprite, with border, as window
    // draw border on top as part of shade
    if( mWindowSet && mWindowPosition.type == enemy ) {
        startAddingToStencil( false, true );
        Enemy *e = mEnemies.getElement( mWindowPosition.index );
        e->sprite->draw( e->position, 1 );
        }
    else if( mWindowSet && mWindowPosition.type == player ) {
        drawMouse( 1 );
        drawPlayer( 1 );
        
        startAddingToStencil( false, true );
        mPlayerSprite.draw( mPlayerPos );
        }
    else if( mWindowSet && mWindowPosition.type == power ) {
        startAddingToStencil( false, true );
        PowerUpToken *t = mPowerUpTokens.getElement( mWindowPosition.index );
        
        drawPowerUp( t->power, t->position, 1 );
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



void Level::drawWindowShade( double inFade, double inFrameFade ) {
    if( mWindowSet ) {
        stopStencil();
        
        if( mWindowPosition.type == player ) {
            mPlayerSprite.drawCenter( mPlayerPos, inFade );
            mPlayerSprite.drawBorder( mPlayerPos, inFrameFade );
            
            // mouse drawn under player
            }
        else if( mWindowPosition.type == enemy ) {    
            Enemy *e = mEnemies.getElement( mWindowPosition.index );
            e->sprite->drawCenter( e->position, inFade );
            e->sprite->drawBorder( e->position, inFrameFade );


            // mouse and player drawn on top of enemy
            // fade these a bit sooner to get them out of the way
            double overlieFade = (inFade - 0.63) / 0.37;
            if( overlieFade > 0 ) {
                drawMouse( overlieFade );
                drawPlayer( overlieFade );
                }
            
            }
        else if( mWindowPosition.type == power ) {    
            PowerUpToken *t = 
                mPowerUpTokens.getElement( mWindowPosition.index );
            
            drawPowerUpCenter( t->power, t->position, inFade );
            drawPowerUpBorder( t->position, inFrameFade );


            // mouse and player drawn on top of enemy
            // fade these a bit sooner to get them out of the way
            double overlieFade = (inFade - 0.63) / 0.37;
            if( overlieFade > 0 ) {
                drawMouse( overlieFade );
                drawPlayer( overlieFade );
                }
            
            }
        
        }
    }



void Level::forgetItemWindow() {
    mWindowSet = false;
    }



GridPos Level::getGridPos( doublePair inWorldPos ) {
    GridPos p;
    p.x = (int)( rint( inWorldPos.x ) );
    p.y = (int)( rint( inWorldPos.y ) );
    
    p.x += MAX_LEVEL_W/2;
    p.y += MAX_LEVEL_H/2;
    
    if( p.x < 0 || p.x >= MAX_LEVEL_W ||
        p.y < 0 || p.y >= MAX_LEVEL_H ) {
        // out of bounds
        
        p.x = 0;
        p.y = 0;
        }
    return p;
    }



char Level::isWall( doublePair inPos ) {
    /*
    int x = (int)( rint( inPos.x ) );
    int y = (int)( rint( inPos.y ) );
    
    x += MAX_LEVEL_W/2;
    y += MAX_LEVEL_H/2;
    
    if( x < 0 || x >= MAX_LEVEL_W ||
        y < 0 || y >= MAX_LEVEL_H ) {
        // out of bounds
        return false;
        }
    */
    GridPos p = getGridPos( inPos );
        
    return ( mWallFlags[p.y][p.x] == 2 );
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


doublePair Level::getPowerUpCenter( int inPowerUpIndex ) {
    PowerUpToken *t = mPowerUpTokens.getElement( inPowerUpIndex );
    return t->position;
    }




char Level::isEnemy( doublePair inPos, int *outEnemyIndex ) {
    for( int j=0; j<mEnemies.size(); j++ ) {
        Enemy *e = mEnemies.getElement( j );
        
        if( distance( e->position, inPos ) < 0.5 ) {

            if( outEnemyIndex != NULL ) {
                *outEnemyIndex = j;
                }
            return true;
            }
        }
    return false;
    }



char Level::isPlayer( doublePair inPos ) {
    return ( distance( mPlayerPos, inPos ) < 0.5 );
    }



char Level::isPowerUp( doublePair inPos, int *outPowerUpIndex ) {
    for( int j=0; j<mPowerUpTokens.size(); j++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( j );
        
        if( distance( t->position, inPos ) < 0.5 ) {
            if( outPowerUpIndex != NULL ) {
                *outPowerUpIndex = j;
                }

            return true;
            }
        }
    return false;    
    }


PowerUp Level::getPowerUp( doublePair inPos ) {
    for( int j=0; j<mPowerUpTokens.size(); j++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( j );
        
        if( distance( t->position, inPos ) < 0.5 ) {
            
            PowerUp p = t->power;

            delete t->sprite;
            delete t->subPowers;

            mPowerUpTokens.deleteElement( j );
            

            return p;
            }
        }
    
    printf( "WARNING:  Level::getPowerUp failed\n" );
    return getRandomPowerUp( mLevelNumber / POWER_SET_SIZE );
    }




ColorScheme Level::getLevelColors() {
    return mColors;
    }



ColorScheme Level::getEnteringPointColors( doublePair inPosition,
                                           itemType inType ) {
    switch( inType ) {
        case player: {
            mLastEnterPointSprite = &mPlayerSprite;
            mLastEnterPointPowers = mPlayerPowers;
            mLastEnterPointPowerTokenIndex = -1;
            
            return mPlayerSprite.getColors();
            }
            break;
        case enemy: {
            int i;
    
            if( isEnemy( inPosition, &i ) ) {
                Enemy *e = mEnemies.getElement( i );
                
                mLastEnterPointSprite = e->sprite;
                mLastEnterPointPowers = e->powers;
                mLastEnterPointPowerTokenIndex = -1;

                return e->sprite->getColors();
                }
            }
            break;
        case power: {
            int i;
    
            if( isPowerUp( inPosition, &i ) ) {
                PowerUpToken *t = mPowerUpTokens.getElement( i );
                
                mLastEnterPointSprite = t->sprite;
                mLastEnterPointPowers = t->subPowers;
                mLastEnterPointPowerTokenIndex = i;

                if( t->startedEmpty &&
                    t->power.powerType != powerUpEmpty ) {
                    
                    // type has been changed by some sub-level activity
                
                    // fix the type away from empty now
                    t->startedEmpty = false;
                    t->sprite->mStartedEmpty = false;
                    }
                

                return t->sprite->getColors();
                }
            }
            break;            
        }
    
    // default
    ColorScheme c;
    return c;
    }



int Level::getEnteringPointSubLevel( doublePair inPosition,
                                     itemType inType ) {
    switch( inType ) {
        case player: {
            return mLevelNumber - 1;
            }
            break;
        case enemy: {
            int i;
    
            if( isEnemy( inPosition, &i ) ) {
                Enemy *e = mEnemies.getElement( i );
                
                int powerSum = 0;
                for( int j=0; j<POWER_SET_SIZE; j++ ) {
                    
                    powerSum += e->powers->mPowers[j].level;
                    }
                return powerSum;
                }
            }
            break;
        case power: {
            int i;
    
            if( isPowerUp( inPosition, &i ) ) {
                return mLevelNumber / POWER_SET_SIZE;
                }
            }
            break;            
        }
    
    // default
    return 0;
    }



int Level::getLevelNumber() {
    return mLevelNumber;
    }



BorderSprite *Level::getLastEnterPointSprite() {
    return mLastEnterPointSprite;
    }



PowerUpSet *Level::getLastEnterPointPowers() {
    return mLastEnterPointPowers;
    }


PlayerSprite *Level::getPlayerSprite() {
    return &mPlayerSprite;
    }


PowerUpSet *Level::getPlayerPowers() {
    return mPlayerPowers;
    }



void Level::getPlayerHealth( int *outValue, int *outMax ) {
    *outValue = mPlayerHealth;
    *outMax = 3 + getMaxHealth( mPlayerPowers );
    }



void Level::restorePlayerHealth() {
    int v, m;
    getPlayerHealth( &v, &m );
    mPlayerHealth = m;
    }





void Level::freezeLevel( char inFrozen ) {
    mFrozen = inFrozen;
    }


void Level::drawFloorEdges( char inDraw ) {
    if( inDraw && !mDrawFloorEdges ) {
        // start fade-in
        mEdgeFadeIn = 0;
        }
    
    mDrawFloorEdges = inDraw;
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
            // closest fraction of whole pixels without going into wall

            int intY = (int)rint( xMoveAlone.y );
            if( velocityY > 0 ) {
                xMoveAlone.y = intY + 0.375;
                }
            else {
                xMoveAlone.y = intY - 0.375;
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
                    yMoveAlone.x = intX + 0.375;
                    }
                else {
                    yMoveAlone.x = intX - 0.375;
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




// when aim already factors in accuracy
static doublePair getBulletVelocity( doublePair inPosition,
                                     doublePair inAimPosition,
                                     double inSpeed ) {

    double aimDist = distance( inAimPosition, inPosition );

    // first add center bullet
    doublePair bulletVelocity = sub( inAimPosition, inPosition );
    
    if( aimDist > 0 ) {                
        // normalize
        bulletVelocity.x /= aimDist;
        bulletVelocity.y /= aimDist;
        }
    else {
        bulletVelocity.x = 0;
        bulletVelocity.y = 1;
        }            
            
    doublePair aimDirection = bulletVelocity;
    

    bulletVelocity.x *= inSpeed;
    bulletVelocity.y *= inSpeed;


    return bulletVelocity;
    }

        


void Level::addBullet( doublePair inPosition,
                       doublePair inAimPosition,
                       double inAccuracy,
                       double inSpread,
                       double inHeatSeek,
                       doublePair inHeatSeekWaypoint,
                       double inSpeed, char inPlayerBullet,
                       int inEnemyIndex ) {

    double exactAimDist = distance( inAimPosition, inPosition );

    double distanceScaleFactor = exactAimDist / 10;
    

    inAccuracy *= distanceScaleFactor;


    inAimPosition.x += 
        randSource.getRandomBoundedDouble( -inAccuracy, inAccuracy );
    inAimPosition.y += 
        randSource.getRandomBoundedDouble( -inAccuracy, inAccuracy );
    



    float size = 1;

    if( inPlayerBullet ) {
        size = getBulletSize( mPlayerPowers );
        }
    else {
        size = getBulletSize( mEnemies.getElement( inEnemyIndex )->powers );
        }


    // first add center bullet
    doublePair bulletVelocity = getBulletVelocity( inPosition, inAimPosition,
                                                   inSpeed );

    Bullet b = { inPosition, bulletVelocity, inSpeed, inHeatSeek,
                 inHeatSeekWaypoint,
                 inPlayerBullet, size };
    mBullets.push_back( b );


    
    if( inSpread > 0 ) {
        doublePair perpToAim = { - bulletVelocity.y, bulletVelocity.x };

        perpToAim = normalize( perpToAim );
        
        int numInPack = (int)inSpread;
        
        if( numInPack > 0 ) {
            
            doublePair packOffset = perpToAim;
            
            packOffset.x *= spreadD2 * distanceScaleFactor;
            packOffset.y *= spreadD2 * distanceScaleFactor;
            
            for( int i=0; i<numInPack; i++ ) {
                doublePair packMemberOffset = { (i+1) * packOffset.x,
                                                (i+1) * packOffset.y };
                
                // left pack member
                doublePair packMemberAimPos = 
                    add( inAimPosition, packMemberOffset );
                
                doublePair bulletVelocity = 
                    getBulletVelocity( inPosition, 
                                       packMemberAimPos,
                                       inSpeed );

                Bullet b = { inPosition, bulletVelocity,
                             inSpeed, inHeatSeek, inHeatSeekWaypoint,
                             inPlayerBullet, size };
                mBullets.push_back( b );


                // right pack member
                packMemberAimPos = 
                    sub( inAimPosition, packMemberOffset );
                
                bulletVelocity = 
                    getBulletVelocity( inPosition, 
                                       packMemberAimPos,
                                       inSpeed );

                Bullet br = { inPosition, bulletVelocity,
                              inSpeed, inHeatSeek, inHeatSeekWaypoint,
                              inPlayerBullet, size };
                mBullets.push_back( br );
                }
            
            }
        
        double outsiderOffsetFactor = 1 - (inSpread - numInPack);
        outsiderOffsetFactor *= spreadD1 * distanceScaleFactor;
        
        doublePair outsiderOffset = perpToAim;
        
        outsiderOffset.x *= outsiderOffsetFactor;
        outsiderOffset.y *= outsiderOffsetFactor;
        
        
        // left outsider
        doublePair outsiderAimPos = 
            add( inAimPosition, outsiderOffset );
                
        doublePair bulletVelocity = 
            getBulletVelocity( inPosition, 
                               outsiderAimPos,
                               inSpeed );

        Bullet b = { inPosition, bulletVelocity, 
                     inSpeed, inHeatSeek, inHeatSeekWaypoint,
                     inPlayerBullet, size };
        mBullets.push_back( b );


        // right pack member
        outsiderAimPos = 
            sub( inAimPosition, outsiderOffset );
                
        bulletVelocity = 
            getBulletVelocity( inPosition, 
                               outsiderAimPos,
                               inSpeed );
        
        Bullet br = { inPosition, bulletVelocity,
                      inSpeed, inHeatSeek, inHeatSeekWaypoint,
                      inPlayerBullet, size };
        mBullets.push_back( br );
        }
    }

    

