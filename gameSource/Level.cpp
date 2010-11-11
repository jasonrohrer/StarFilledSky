#include "Level.h"
#include "drawUtils.h"
#include "fixedSpriteBank.h"
#include "powerUpProperties.h"
#include "bulletSizeSet.h"
#include "BasicRandomWalker.h"
#include "StraightRandomWalker.h"
#include "CurvedRandomWalker.h"
#include "RoundPodRandomWalker.h"
#include "RectPodRandomWalker.h"
#include "DiagRandomWalker.h"


#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>


char Level::sGridWorldSpotsComputed;
doublePair Level::sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];

extern CustomRandomSource randSource;

extern double frameRateFactor;


double maxEnemySpeed = 0.05;






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

    


    RandomWalker *walker = mWalkerSet.pickWalker( xLimit, 
                                                  3, 
                                                  MAX_LEVEL_W - 3,
                                                  MAX_LEVEL_H - 3 );
    

    char done = false;

    for( int i=0; 
         i<stepLimit && 
             mNumFloorSquares < numFloorSquaresMax &&
             !done; 
         i++ ) {

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
        
        GridPos p = { x, y };
        
        int batchSize = walker->getStepsLeftInBatch();

        p = walker->getNextStep( p );
        x = p.x;
        y = p.y;
        
        char batchDone = ( batchSize == 1 );

        
        batchSize = walker->getStepsLeftInBatch();

        if( mNumUsedSquares +  batchSize >= numFloorSquaresMax 
            ||
            i + batchSize >= stepLimit ) {
            
            // stop w/out adding any of this new batch
            done = true;
            }
        else if( batchDone ) {
            // can switch walkers
            delete walker;
            walker = mWalkerSet.pickWalker( xLimit, 
                                            3, 
                                            MAX_LEVEL_W - 3,
                                            MAX_LEVEL_H - 3 );
            }
        

        /*
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
        */
        
        }

    delete walker;
    


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
    
    mColorMix = new float[ mNumUsedSquares ];
    mColorMixDelta = new float[ mNumUsedSquares ];
    for( int i=0; i<mNumUsedSquares; i++ ) {
        mColorMix[i] = randSource.getRandomBoundedDouble( 0, 1 );
        mColorMixDelta[i] = 
            randSource.getRandomBoundedDouble( 0.005, 0.01 ) * frameRateFactor;
        
        // set starting point mix, different from target mix, so
        // that color shimmer starts right away
        float startMix = randSource.getRandomBoundedDouble( 0, 1 );
        float mix = startMix * 0.4;
        float counterMix = 1 - mix;

        mGridColors[i].r = 
            mHardGridColors[i].r * mix 
            + mSoftGridColors[i].r * counterMix;
        mGridColors[i].g = 
            mHardGridColors[i].g * mix 
            + mSoftGridColors[i].g * counterMix;
        mGridColors[i].b = 
            mHardGridColors[i].b * mix 
            + mSoftGridColors[i].b * counterMix;

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

        for( int i=0; i<mEnemies.size(); i++ ) {
            Enemy *e = mEnemies.getElement( i );
            e->sprite->compactSprite();
            }        

        mDataGenerated = false;
        }
    
    }



//#include "minorGems/system/Thread.h"



Level::Level( ColorScheme *inPlayerColors, ColorScheme *inColors, 
              RandomWalkerSet *inWalkerSet,
              int inLevelNumber, char inSymmetrical ) 
        : mLevelNumber( inLevelNumber ), 
          mPlayerSprite( inPlayerColors ),
          mPlayerPowers( new PowerUpSet( inLevelNumber - 1 ) ) {

    int health, max;
    getPlayerHealth( &health, &max );
    mPlayerHealth = max;
    
    mNextEnemyPathFindIndex = 0;
    
    
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
    

    if( inWalkerSet != NULL ) {
        mWalkerSet = *( inWalkerSet );
        }
    // else use randomly-generated walker set from stack


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
                
                // random starting velocity
                doublePair baseMoveDirection = 
                    { randSource.getRandomBoundedDouble( -1, 1 ), 
                      randSource.getRandomBoundedDouble( -1, 1 ) };
                baseMoveDirection = normalize( baseMoveDirection );
                
                // start off with basic speed
                doublePair v = mult( baseMoveDirection, 
                                     maxEnemySpeed * frameRateFactor );
                doublePair a = { 0, 0 };
                

                PowerUpSet *p = new PowerUpSet( mLevelNumber - 1, true );
                
                Enemy e = { spot, v, a, baseMoveDirection, 20, 
                            randSource.getRandomBoundedInt( 0, 10 ),
                            new EnemySprite(),
                            p,
                            getEnemyMaxHealth( p ),
                            0,
                            spot,
                            NULL,
                            false,
                            randSource.getRandomBoundedDouble( 0.1, 0.8 ) };
                        
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

    for( int i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        e->sprite->decompactSprite();
        }
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



typedef struct pathSearchRecord {
        GridPos pos;
        
        int cost;
        double estimate;
        double total;
        
        // index of pred in done queue
        int predIndex;
        

    } pathSearchRecord;


double getGridDistance( GridPos inA, GridPos inB ) {
    int dX = inA.x - inB.x;
    int dY = inA.y - inB.y;
    
    return sqrt( dX * dX + dY * dY );
    }


char equal( GridPos inA, GridPos inB ) {
    return inA.x == inB.x && inA.y == inB.y;
    }




GridPos Level::pathFind( GridPos inStart, doublePair inStartWorld, 
                         GridPos inGoal, double inMoveSpeed ) {
    SimpleVector<pathSearchRecord> searchQueue;
    SimpleVector<pathSearchRecord> doneQueue;
            
    pathSearchRecord startRecord = 
        { inStart,
          0,
          getGridDistance( inStart, inGoal ),
          getGridDistance( inStart, inGoal ),
          -1 };
                
    searchQueue.push_back( startRecord );
            


    char done = false;
            
            
    while( searchQueue.size() > 0 && !done ) {
                
        int bestIndex = -1;
        double bestTotal = DBL_MAX;
                
        for( int q=0; q<searchQueue.size(); q++ ) {
            pathSearchRecord *record = searchQueue.getElement( q );
                    
            if( record->total < bestTotal ) {
                bestTotal = record->total;
                bestIndex = q;
                }
            }
                

        pathSearchRecord bestRecord = 
            *( searchQueue.getElement( bestIndex ) );
                
        if( false )
            printf( "Best record found:  "
                    "(%d,%d), cost %d, total %f, "
                    "pred %d, this index %d\n",
                    bestRecord.pos.x, bestRecord.pos.y,
                    bestRecord.cost, bestRecord.total,
                    bestRecord.predIndex, doneQueue.size() );
                

        searchQueue.deleteElement( bestIndex );
                

        doneQueue.push_back( bestRecord );

        int predIndex = doneQueue.size() - 1;
                

        if( equal( bestRecord.pos, inGoal ) ) {
            // goal record has lowest total score in queue
            done = true;
            }
        else {
            // add neighbors
            GridPos neighbors[4];
                    
            GridPos bestPos = bestRecord.pos;
                    
            neighbors[0].x = bestPos.x;
            neighbors[0].y = bestPos.y - 1;

            neighbors[1].x = bestPos.x;
            neighbors[1].y = bestPos.y + 1;

            neighbors[2].x = bestPos.x - 1;
            neighbors[2].y = bestPos.y;

            neighbors[3].x = bestPos.x + 1;
            neighbors[3].y = bestPos.y;
                    
            // one step to neighbors from best record
            int cost = bestRecord.cost + 1;

            for( int n=0; n<4; n++ ) {
                        
                if( mWallFlags[ neighbors[n].y ][ neighbors[n].x ]
                    == 1 ) {
                    // floor
                            
                    char foundInDone = false;
                            

                    for( int q=0; 
                         q<doneQueue.size() && ! foundInDone; 
                         q++ ) {
                                
                        pathSearchRecord *record = 
                            doneQueue.getElement( q );
                                
                        if( equal( record->pos, neighbors[n] ) ) {
                            foundInDone = true;
                            }
                        }
                            
                    char foundInSearch = false;
                            
                    if( !foundInDone ) {
                                
                        for( int q=0; 
                             q<searchQueue.size() && 
                                 ! foundInSearch; 
                             q++ ) {
                                
                            pathSearchRecord *record = 
                                searchQueue.getElement( q );
                                    
                            if( equal( record->pos, 
                                       neighbors[n] ) ) {
                                            
                                foundInSearch = true;
                                }
                            }
                        }
                            
                            
                    if( !foundInDone && !foundInSearch ) {
                        // add this neighbor
                        double dist = 
                            getGridDistance( neighbors[n], 
                                             inGoal );
                            
                        // track how we got here (pred)
                        pathSearchRecord nRecord = { neighbors[n],
                                                     cost,
                                                     dist,
                                                     dist + cost,
                                                     predIndex };
                        searchQueue.push_back( nRecord );
                        }
                            
                    }
                }
                    

            }
        }
            
    // follow index to reconstruct path
    // last in done queue is best-reached goal node
            
    // stop following path back once we find a node that is straight-line
    // reachable from start with no obstacles

    int currentIndex = doneQueue.size() - 1;
            
    pathSearchRecord *currentRecord = 
        doneQueue.getElement( currentIndex );

    pathSearchRecord *predRecord = 
        doneQueue.getElement( currentRecord->predIndex );
            
    done = false;
    
    while( ! equal(  predRecord->pos, inStart ) && ! done ) {
        currentRecord = predRecord;
        predRecord = 
            doneQueue.getElement( currentRecord->predIndex );

        // straight-line, unobstructed path from start to currentRecord?

        doublePair stepPos = { inStartWorld.x, inStartWorld.y };
        doublePair goalPos = 
            sGridWorldSpots[ currentRecord->pos.y ][ currentRecord->pos.x ];
        GridPos stepGridPos = inStart;
        
        doublePair stepDelta = mult( normalize( sub( goalPos, stepPos ) ),
                                     inMoveSpeed );
        
        while( !equal( stepGridPos, currentRecord->pos ) &&
               mWallFlags[ stepGridPos.y ][ stepGridPos.x ] == 1 ) {
            
            stepPos = add( stepPos, stepDelta );
            stepGridPos = getGridPos( stepPos );
            }
        
        if( equal( stepGridPos, currentRecord->pos ) ) {
            done = true;
            }
        }
    // current record shows best move

    return currentRecord->pos;                
    }






void Level::step() {
    int i;
    
    // step bullets
    for( i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );
        
        if( b->heatSeek > 0 ) {

            // vector toward closest target
            doublePair closestTarget = mPlayerPos;
            char shouldAdjustVelocity = true;
            
            if( b->playerFlag ) {
            
                shouldAdjustVelocity = false;
                if( mEnemies.size() > 0 ) {
                    shouldAdjustVelocity = true;
                    
                    // search for closest enemy to waypoint 
                    // (closest to reticle at time of firing)
                
                    double minDistance = DBL_MAX;
                    int minIndex = -1;
                    for( int j=0; j<mEnemies.size(); j++ ) {
                        Enemy *e = mEnemies.getElement( j );
                        
                        double dist = distance( e->position, 
                                                b->heatSeekWaypoint );
                        if( dist < minDistance ) {
                            minDistance = dist;
                            minIndex = j;
                            }
                        }
                
                    Enemy *e = mEnemies.getElement( minIndex );
                    closestTarget = e->position;                        
                    }
                }
            
            
            if( shouldAdjustVelocity ) {
                
                doublePair vectorToTarget = normalize( sub( closestTarget, 
                                                            b->position ) );
                doublePair adjustedVelocity = normalize( b->velocity );
            
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
            }
        

        doublePair oldBulletPos = b->position;
        
        b->position = add( b->position, b->velocity );        
        
        b->distanceLeft -= b->speed;
        
        
        GridPos p = getGridPos( b->position );


        char hit = false;
        char damage = false;
        char destroyed = false;
        

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
                // jump to hard when hit, then fade out
                mGridColors[ squareIndex ] = mHardGridColors[ squareIndex ];

                if( b->bouncesLeft == 0 ) {
                    
                    hit = true;
                    // stop bullet at wall boundary to prevent tunneling
                    // of explosion sub bullets through thin walls
                    b->position = stopMoveWithWall( oldBulletPos, 
                                                    b->velocity );
                    }
                else {
                    b->bouncesLeft --;
                    
                    doublePair oldPos = 
                        sub( b->position, b->velocity );        

                    doublePair yOnly = oldPos;
                    yOnly.y += b->velocity.y;
                    
                    doublePair xOnly = oldPos;
                    xOnly.x += b->velocity.x;
                    
                    if( isWall( yOnly ) ) {
                        b->velocity.y *= -1;
                    
                        if( isWall( xOnly ) ) {
                            b->velocity.x *= -1;
                            }
                        }
                    else if( isWall( xOnly ) ) {
                        b->velocity.x *= -1;
                        if( isWall( yOnly ) ) {
                            b->velocity.y *= -1;
                            }
                        }
                    else {
                        // corner case?
                        b->velocity.x *= -1;
                        b->velocity.y *= -1;
                        }
                    
                    // first step in bounce-back, to get outside of wall
                    //b->position = add( b->position, b->velocity );
                    }
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
                            damage = true;
                            
                            // make sure enemy health is up-to-date
                            // (its power-ups may have been modified)
                            int maxHealth = 
                                getEnemyMaxHealth( e->powers );
                        
                            if( e->health > maxHealth ) {
                                e->health = maxHealth;
                                }

                            e->health --;
                            if( e->health == 0 ) {

                                // add hit smoke at enemy center
                                
                                ColorScheme colors = e->sprite->getColors();
                                
                                Color borderColor = colors.primary.elements[3];
                                
                                // big smoke for border, dead center
                                HitSmoke s = { e->position, 0, 0.75, 3, 
                                               borderColor };
                                
                                mSmokeClouds.push_back( s );
                                

                                // one extra small puff for each other color
                                for( int i=0; i<3; i++ ) {
                                    doublePair pos = e->position;
                                    
                                    pos.x += 
                                        randSource.getRandomBoundedDouble(
                                            -0.25, 0.25 );
                                    pos.y += 
                                        randSource.getRandomBoundedDouble(
                                            -0.25, 0.25 );
                                    
                                    
                                    Color primary = colors.primary.elements[i];
                                    HitSmoke s2 = { pos, 0, 0.5, 3, 
                                                    primary };
                                
                                    mSmokeClouds.push_back( s2 );
                                


                                    pos = e->position;
                                    
                                    pos.x += 
                                        randSource.getRandomBoundedDouble(
                                            -0.25, 0.25 );
                                    pos.y += 
                                        randSource.getRandomBoundedDouble(
                                            -0.25, 0.25 );

                                    Color secondary = 
                                        colors.secondary.elements[i];
                                    HitSmoke s3 = { pos, 0, 0.5, 3, 
                                                    secondary };
                                
                                    mSmokeClouds.push_back( s3 );
                                    }
                                
                                

                                
                                // don't generate other hit smoke
                                destroyed = true;
                                

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
                        damage = true;
                        mPlayerHealth -= 1;
                        if( mPlayerHealth < 0 ) {
                            mPlayerHealth = 0;
                            }
                        if( mPlayerHealth == 0 ) {
                            destroyed = true;
                            }                                
                        }
                    }
            
                }
            }
        


        if( hit || b->distanceLeft <= 0 ) {
            // bullet done
            
            if( ( hit || b->explode > 0 ) && 
                ( !destroyed || ! b->playerFlag ) ) {
                // target not destroyed by hit, or player destroyed, 
                // draw smoke

                char type = 0;
                if( damage ) {
                    type = 2;
                    }
                else if( b->playerFlag ) {
                    type = 1;
                    }
                // unused
                Color c;
                
                double progress = 0;
                if( destroyed && ! b->playerFlag ) {
                    // start bigger
                    progress = 0.25;
                    }

                HitSmoke s = { b->position, progress, 0.5, type, c };
                
                mSmokeClouds.push_back( s );
                }
            
            if( b->explode > 0 ) {
                // make explosion happen
                
                Bullet explosionTemplate = *b;
                
                // reset bounces and distance
                explosionTemplate.distanceLeft = b->startDistance;
                explosionTemplate.bouncesLeft = b->startBounces;
                
                // no sub-explosions
                explosionTemplate.explode = 0;
                
                // start pointing back at shooter
                explosionTemplate.velocity.x *= -1;
                explosionTemplate.velocity.y *= -1;
                

                int numSubBullets = 3 + (int)( b->explode );
                
                double extra = b->explode - (int)( b->explode );
                
                double sepAngle = 2 * M_PI / numSubBullets;

                // as we approach next explosion level, get closer
                // to "perfect" explosion that sends no sub-bullet back
                // at shooter
                // move toward start angle that evenly splits back-facing
                // bullets on either side of original aim vector
                double startAngle = extra * sepAngle / 2;
                
                for( int s=0; s<numSubBullets; s++ ) {
                    Bullet subBullet = explosionTemplate;
                    subBullet.velocity = rotate( subBullet.velocity,
                                                 startAngle + s * sepAngle );
                    
                    mBullets.push_back( subBullet );
                    }
                }
            

            // clear out any enemy pointer to this bullet
            for( int j=0; j<mEnemies.size(); j++ ) {
                Enemy *e = mEnemies.getElement( j );
                
                if( e->dodgeBullet == b ) {
                    e->dodgeBullet = NULL;
                    }
                }
            
                
            mBullets.deleteElement( i );
            i--;
            }
        
        
        }


    // step smoke
    for( i=0; i<mSmokeClouds.size(); i++ ) {
        HitSmoke *s = mSmokeClouds.getElement( i );
        
        s->progress += 0.03125 * frameRateFactor;
        if( s->progress > 1 ) {
            
            mSmokeClouds.deleteElement( i );
            i--;
            }
        }
    
    
    
    mNextEnemyPathFindIndex ++;
    if( mNextEnemyPathFindIndex > mEnemies.size() - 1 ) {
        mNextEnemyPathFindIndex = 0;
        }
    

    // step enemies
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );


        // search for behaviors
        char follow = false;
        char dodge = false;
        char random = false;
        char circle = false;
        char moveStyle = false;

        double moveSpeed = maxEnemySpeed * frameRateFactor;
        
        for( int p=0; p<POWER_SET_SIZE; p++ ) {
            spriteID powerType =e-> powers->mPowers[p].powerType;
            
            switch( powerType ) {
                case enemyBehaviorFollow:
                    follow = true;
                    break;
                case enemyBehaviorDodge:
                    dodge = true;
                    break;
                case enemyBehaviorFast:
                    moveSpeed *= 2;
                    break;
                case enemyBehaviorRandom:
                    random = true;
                    break;
                case enemyBehaviorCircle:
                    circle = true;
                    break;
                default:
                    break;
                }
            }
        
        moveStyle = random || follow || circle;
        

        // temporarily disable follow during dodge
        if( follow && e->dodgeBullet == NULL ) {

            if( mNextEnemyPathFindIndex == i ) {
                

                // conduct pathfinding search
                GridPos start = getGridPos( e->position );
                
                GridPos goal = getGridPos( mPlayerPos );
                
                if( !equal( start, goal ) ) {
                    
                    GridPos targetGridPos = pathFind( start, e->position,
                                                      goal,
                                                      moveSpeed );
                
                    
                    e->followNextWaypoint = 
                        sGridWorldSpots[ targetGridPos.y ]
                        [ targetGridPos.x ];
                    }
                }
            
            
            doublePair followVelocity = 
                mult( normalize( sub( e->followNextWaypoint, 
                                      e->position ) ),
                      moveSpeed );

            
            // weighted sum with old velocity to smooth out movement
            doublePair weightedFollow = mult( followVelocity, 0.3 );
            
            e->velocity = mult( e->velocity, 0.7 );
            e->velocity = add( e->velocity, weightedFollow );
            
            // handle position update in normal move phase below
            }
        if( dodge ) {
            
            if( i == mNextEnemyPathFindIndex ) {
                

                // find closest player bullet
                int closestIndex = -1;
                double closestDistance = DBL_MAX;
                
                for( int b=0; b<mBullets.size(); b++ ) {
                    
                    Bullet *bullet = mBullets.getElement( b );
                    
                    if( bullet->playerFlag ) {
                        
                        double dist = distance( bullet->position, 
                                                e->position );
                        
                        if( dist < closestDistance ) {
                            closestDistance = dist;
                            closestIndex = b;
                            }               
                        }
                    }

                if( closestIndex != -1 &&
                    // ignore too far away to dodge
                    closestDistance < 5 ) {
                    e->dodgeBullet = mBullets.getElement( closestIndex );
                    }
                else {
                    e->dodgeBullet = NULL;
                    }
                }
            

            if( e->dodgeBullet != NULL ) {
                
                Bullet *bullet = e->dodgeBullet;
                
                // indexing in mBullets may change as bullets expire
                if( bullet->playerFlag ) {
                    
                    doublePair moveChoiceA = { bullet->velocity.y,
                                               -bullet->velocity.x };
                
                    doublePair moveChoiceB = { -bullet->velocity.y,
                                               bullet->velocity.x };
                

                    moveChoiceA = normalize( moveChoiceA );
                    moveChoiceB = normalize( moveChoiceB );
                
                    doublePair awayFromBullet = 
                        normalize( sub( e->position, bullet->position ) );
                
                    doublePair moveChoice;
                
                    if( dot( moveChoiceA, awayFromBullet ) > 0 ) {
                        moveChoice = moveChoiceA;
                        }
                    else {
                        moveChoice = moveChoiceB;
                        }



                    // weighted sum with old velocity to smooth out movement
                    doublePair dogeVelocity = mult( normalize( moveChoice ),
                                                    moveSpeed );
                    
                    doublePair weightedDodge = mult( dogeVelocity, 0.8 );
            
                    e->velocity = mult( e->velocity, 0.2 );
                    e->velocity = add( e->velocity, weightedDodge );
            
                
                    // don't update position here, do it in normal move phase
                    // below
                    }
                }
            }

        // normal movement for all enemies
        
        doublePair oldPos = e->position;
        e->position = stopMoveWithWall( e->position,
                                        e->velocity );
        
        doublePair desiredPosition = add( oldPos, e->velocity );
        
        char hitWall = false;
        if( !equal( desiredPosition, e->position ) ) {
            hitWall = true;
            }
        
        

        if( random ) {
            // get actual velocity, taking wall collision into account
            e->velocity = sub( e->position, oldPos );

            // random accel
            e->velocity = add( e->velocity, e->accel );

            if( e->velocity.x > moveSpeed ) {
                e->velocity.x = moveSpeed;
                }
            else if( e->velocity.x < -moveSpeed ) {
                e->velocity.x = -moveSpeed;
                }

            if( e->velocity.y > moveSpeed ) {
                e->velocity.y = moveSpeed;
                }
            else if( e->velocity.y < -moveSpeed ) {
                e->velocity.y = -moveSpeed;
                }
        
        
            // random adjustment to acceleration
            e->accel.x = frameRateFactor * 
                randSource.getRandomBoundedDouble( -0.005, 0.005 );
            e->accel.y = frameRateFactor *
                randSource.getRandomBoundedDouble( -0.005, 0.005 );
            }

        if( circle ) {

            if( hitWall ) {
                e->circleDirection = ! e->circleDirection;
                e->velocity = mult( e->velocity, -1 );
                
                // bounce off
                e->position = oldPos;
                }
                
            // accel toward center of circle
            doublePair accel = { e->velocity.y,
                                 -e->velocity.x };
            if( e->circleDirection ) {
                accel = mult( accel, -1 );
                }
            
            accel = mult( accel, 
                          e->circleRadiusFactor * 
                          moveSpeed );
            
            e->velocity = mult( normalize( add( e->velocity, accel ) ),
                                    moveSpeed );
            }
        
                

        
        if( !moveStyle ) {
            // standard move, back and forth between walls    
            if( hitWall ) {
                e->baseMoveDirection = mult( e->baseMoveDirection, -1 );
                
                // only update this when hit wall (otherwise, interferes
                // with dodge)
                // move speed might change as power ups change
                e->velocity = mult( e->baseMoveDirection, moveSpeed );
            
                // bounce off
                e->position = oldPos;                
                }
            }
        
        
        if( e->stepsTilNextBullet == 0 ) {
            // fire bullet

            // set speed
            // enemy bullets are slower than equivalent player bullets
            float bulletSpeed = getBulletSpeed( e->powers ) / 2;
            
            
            addBullet( e->position, mPlayerPos, 
                       e->powers,
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
    float dampingFactor = 0.025 * frameRateFactor;
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
        float mix = mColorMix[i] * 0.4;
        float counterMix = 1 - mix;

        // average our grid color with the current target mix
        mix *= dampingFactor;
        counterMix *= dampingFactor;
        mGridColors[i].r += 
            mHardGridColors[i].r * mix 
            + mSoftGridColors[i].r * counterMix;
        mGridColors[i].g += 
            mHardGridColors[i].g * mix 
            + mSoftGridColors[i].g * counterMix;
        mGridColors[i].b += 
            mHardGridColors[i].b * mix 
            + mSoftGridColors[i].b * counterMix;
        
        float totalWeight = 1 + dampingFactor;
        
        mGridColors[i].r /= totalWeight;
        mGridColors[i].g /= totalWeight;
        mGridColors[i].b /= totalWeight;
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



void Level::drawSmoke( double inFade ) {
    // draw smoke
    for( int i=0; i<mSmokeClouds.size(); i++ ) {
        
        HitSmoke *s = mSmokeClouds.getElement( i );
        
        float fade = inFade * ( 0.5 - 0.5 * s->progress );
        
        switch( s->type ) {
            case 0:
                setDrawColor( 0, 0, 0, fade );
                break;
            case 1:
                setDrawColor( 1, 1, 1, fade );
                break;
            case 2:
                setDrawColor( 1, 0, 0, fade * 2 );
                break;
            case 3:
                setDrawColor( s->enemyColor.r, 
                              s->enemyColor.g, 
                              s->enemyColor.b, fade * 2 );
                break;
            };
        

        
        //setDrawColor( 1, 1, 1, 1 - s->progress );
        
        drawSquare( s->position, s->maxSize * s->progress );
        }

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

        float fade = 1;
        
        if( b->explode == 0 && b->distanceLeft < 2 ) {
            fade = b->distanceLeft * 0.5;
            }

        
        if( b->playerFlag ) {
            setDrawColor( 1, 1, 1, fade );
            }
        else {
            setDrawColor( 0.35, 0.35, 0.35, fade );
            }
        drawBullet( b->size, b->position, fade );
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
    


    if( !mWindowSet ) {
        drawMouse( 1 );
        drawPlayer( 1 );

        drawSmoke( 1 );
        }
    



    if( mWindowSet ) {
        startDrawingThroughStencil();
        }
    
    
    }



void Level::drawWindowShade( double inFade, double inFrameFade ) {
    if( mWindowSet ) {
        stopStencil();
        
        double overlieFade = (inFade - 0.63) / 0.37;

        if( mWindowPosition.type == player ) {
            mPlayerSprite.drawCenter( mPlayerPos, inFade );
            mPlayerSprite.drawBorder( mPlayerPos, inFrameFade );
            
            // mouse drawn under player
            }
        else {
            if( mWindowPosition.type == enemy ) {    
                Enemy *e = mEnemies.getElement( mWindowPosition.index );
                e->sprite->drawCenter( e->position, inFade );
                e->sprite->drawBorder( e->position, inFrameFade );
                }
            else if( mWindowPosition.type == power ) {    
                PowerUpToken *t = 
                    mPowerUpTokens.getElement( mWindowPosition.index );
                
                drawPowerUpCenter( t->power, t->position, inFade );
                drawPowerUpBorder( t->position, inFrameFade );
                }
            

            // mouse and player drawn on top of enemy or power-up
            // fade these a bit sooner to get them out of the way
            if( overlieFade > 0 ) {
                drawMouse( overlieFade );
                drawPlayer( overlieFade );
                }
            }

        // smoke drawn on top of all
        drawSmoke( overlieFade );
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



PowerUp Level::peekPowerUp( doublePair inPos ) {
    for( int j=0; j<mPowerUpTokens.size(); j++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( j );
        
        if( distance( t->position, inPos ) < 0.5 ) {
            
            PowerUp p = t->power;

            return p;
            }
        }
    
    printf( "WARNING:  Level::peekPowerUp failed\n" );
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





RandomWalkerSet Level::getEnteringPointWalkerSet( doublePair inPosition,
                                                  itemType inType ) {
    switch( inType ) {
        case player: {

            return mPlayerWalkerSet;
            }
            break;
        case enemy: {
            int i;
    
            if( isEnemy( inPosition, &i ) ) {
                Enemy *e = mEnemies.getElement( i );
                
                return e->walkerSet;
                }
            }
            break;
        case power: {
            int i;
    
            if( isPowerUp( inPosition, &i ) ) {
                PowerUpToken *t = mPowerUpTokens.getElement( i );
                
                RandomWalkerSet set( t->power.powerType );
                
                return set;
                }
            }
            break;            
        }
    
    // default
    RandomWalkerSet w;
    return w;
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
                xMoveAlone.y = intY + 0.4375;
                }
            else {
                xMoveAlone.y = intY - 0.4375;
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
                    yMoveAlone.x = intX + 0.4375;
                    }
                else {
                    yMoveAlone.x = intX - 0.4375;
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
                       PowerUpSet *inPowers,
                       doublePair inHeatSeekWaypoint,
                       double inSpeed, char inPlayerBullet,
                       int inEnemyIndex ) {

    double exactAimDist = distance( inAimPosition, inPosition );

    double distanceScaleFactor = exactAimDist / 10;
    
    double inAccuracy = getAccuracy( inPowers );
    double inSpread = getSpread( inPowers );
    double inHeatSeek = getHeatSeek( inPowers );
    
    double distance = getBulletDistance( inPowers );    
    
    int bounce = getBounce( inPowers );
    
    double explode = getExplode( inPowers );
    

    /*
      // for testing
    if( inPlayerBullet ) {
        explode = 4.9;
        //inSpread = 9.9;
        bounce = 9;
        distance = 29;
        }
    */

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


    // velocity for center bullet
    // (add later, on top of pack)
    doublePair bulletVelocity = getBulletVelocity( inPosition, inAimPosition,
                                                   inSpeed );



    
    if( inSpread > 0 ) {
        
        doublePair relativeAimPosition = sub( inAimPosition, inPosition );
        

        // pack members spread wider for larger bullets
        double packSpreadAngle = spreadD2 + ( size * 0.02 );

        
        int numInPack = (int)inSpread;
        
        

        // first add outsiders, under pack members

        double outsiderOffsetAngle = 1 - (inSpread - numInPack);
        outsiderOffsetAngle *= spreadD1;
        
        // outsider spread in addtion to spread present in pack
        outsiderOffsetAngle += packSpreadAngle * (numInPack + 1);
        
        
        // left outsider
        doublePair outsiderAimPos = rotate( relativeAimPosition, 
                                            outsiderOffsetAngle );
        outsiderAimPos = add( outsiderAimPos, inPosition );
        
        doublePair bulletVelocity = 
            getBulletVelocity( inPosition, 
                               outsiderAimPos,
                               inSpeed );

        Bullet b = { inPosition, bulletVelocity, 
                     inSpeed, inHeatSeek, inHeatSeekWaypoint,
                     distance,
                     distance,
                     bounce,
                     bounce,
                     explode,
                     inPlayerBullet, size };
        mBullets.push_back( b );


        // right outsider
        outsiderAimPos = rotate( relativeAimPosition, 
                                 - outsiderOffsetAngle );
        outsiderAimPos = add( outsiderAimPos, inPosition );
        
        bulletVelocity = 
            getBulletVelocity( inPosition, 
                               outsiderAimPos,
                               inSpeed );
        
        Bullet br = { inPosition, bulletVelocity,
                      inSpeed, inHeatSeek, inHeatSeekWaypoint,
                      distance,
                      distance,
                      bounce,
                      bounce,
                      explode,
                      inPlayerBullet, size };
        mBullets.push_back( br );








        if( numInPack > 0 ) {
            
            // add pack members outside-in (so center ones are on top)
            for( int i=numInPack-1; i>=0; i-- ) {
                double packMemberOffsetAngle = (i+1) * packSpreadAngle;
                
                // left pack member
                doublePair packMemberAimPos = rotate( relativeAimPosition, 
                                                      packMemberOffsetAngle );
                packMemberAimPos = add( packMemberAimPos, inPosition );
                
                doublePair bulletVelocity = 
                    getBulletVelocity( inPosition, 
                                       packMemberAimPos,
                                       inSpeed );

                Bullet b = { inPosition, bulletVelocity,
                             inSpeed, inHeatSeek, inHeatSeekWaypoint,
                             distance,
                             distance,
                             bounce,
                             bounce,
                             explode,
                             inPlayerBullet, size };
                mBullets.push_back( b );


                // right pack member
                packMemberAimPos = rotate( relativeAimPosition, 
                                           - packMemberOffsetAngle );
                packMemberAimPos = add( packMemberAimPos, inPosition );

                bulletVelocity = 
                    getBulletVelocity( inPosition, 
                                       packMemberAimPos,
                                       inSpeed );

                Bullet br = { inPosition, bulletVelocity,
                              inSpeed, inHeatSeek, inHeatSeekWaypoint,
                              distance,
                              distance,
                              bounce,
                              bounce,
                              explode,
                              inPlayerBullet, size };
                mBullets.push_back( br );
                }
            
            }
        
        }
    
    // finally, one centered bullet on top
    Bullet b = { inPosition, bulletVelocity, inSpeed, inHeatSeek,
                 inHeatSeekWaypoint,
                 distance,
                 distance,
                 bounce,
                 bounce,
                 explode,
                 inPlayerBullet, size };
    mBullets.push_back( b );
    

    }

    

