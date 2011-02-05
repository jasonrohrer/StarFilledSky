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
#include "tutorial.h"
#include "musicPlayer.h"


#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/game.h"
#include "minorGems/util/random/CustomRandomSource.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Time.h"
#include "minorGems/io/file/File.h"
#include "minorGems/graphics/filters/FastBlurFilter.h"

#include "minorGems/math/probability/ProbabilityMassFunction.h"

#include <math.h>


extern char outputMapImages;


char Level::sGridWorldSpotsComputed;
doublePair Level::sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];

extern CustomRandomSource randSource;

extern double frameRateFactor;


// track as both a full array, for fast lookup, and as
// a vector of entries, for fast clearing
static char blurHitMap[MAX_LEVEL_H][MAX_LEVEL_W];
static SimpleVector<GridPos> blurHitEntries;



double maxEnemySpeed = 0.05;


static int stepsBetweenGlowTrails = 4;

static double trailJitter = 0.25;


static int shadowBlowUpFactor = 4;



static int getEnemyMaxHealth( PowerUpSet *inSet ) {
    return 1 + getMaxHealth( inSet );
    }





static void outputLevelMapImage( Image *inMapImage ) {
    File mapImageDir( NULL, "mapImages" );
    
    if( !mapImageDir.exists() ) {
        mapImageDir.makeDirectory();
        }

    // find next event recording file
    int fileNumber = 0;
        
    char hit = true;

    while( hit ) {
        fileNumber++;
        char *fileName = autoSprintf( "map%05d.tga", 
                                      fileNumber );
        File *file = mapImageDir.getChildFile( fileName );
            
        delete [] fileName;
            
        if( !file->exists() ) {
            hit = false;
            
            char *fullFileName = file->getFullFileName();
        
            writeTGAFile( fullFileName, inMapImage );
            delete [] fullFileName;
            }
        delete file;
        }
    }








/**
 * Blur convolution filter that uses a box for averaging.
 *
 * Faster accumulative implementation, as suggested by Gamasutra.
 *
 * For speed, does NOT handle edge pixels correctly
 *
 * For even more speed, does not support multiple radii (only radius=1)
 *
 *
 * Also, changed to process uchar channels (instead of doubles) for speed
 *
 * @author Jason Rohrer 
 */
class FastBoxBlurFilter { 
	
	public:
		
		/**
		 * Constructs a box filter.
		 */
		FastBoxBlurFilter();
		
		// implements the ChannelFilter interface 
        // (but for uchars and sub-regions, and a subset of pixels in that
        //  region)
		void applySubRegion( unsigned char *inChannel,
                             int *inTouchPixelIndices,
                             int inNumTouchPixels,
                             int inWidth, int inHeight,
                             int inXStart, int inYStart, 
                             int inXEnd, int inYEnd );

	};
	
	
	
FastBoxBlurFilter::FastBoxBlurFilter() {	
	
	}





void FastBoxBlurFilter::applySubRegion( unsigned char *inChannel, 
                                        int *inTouchPixelIndices,
                                        int inNumTouchPixels,
                                        int inWidth, int inHeight,
                                        int inXStart, int inYStart, 
                                        int inXEnd, int inYEnd ) {

    


    // use pointer tricks to walk through neighbor box of each pixel

    // four "corners" around box in accumulation table used to compute
    // box total
    // these are offsets to current accumulation pointer
    int cornerOffsetA = inWidth + 1;
    int cornerOffsetB = -inWidth + 1;
    int cornerOffsetC = inWidth - 1;
    int cornerOffsetD = -inWidth - 1;

    // sides around box
    int sideOffsetA = inWidth;
    int sideOffsetB = -inWidth;
    int sideOffsetC = 1;
    int sideOffsetD = -1;

    unsigned char *sourceData = new unsigned char[ inWidth * inHeight ];
    
    memcpy( sourceData, inChannel, inWidth * inHeight );
    
    
    
    // sum boxes right into passed-in channel

    for( int i=0; i<inNumTouchPixels; i++ ) {

        int pixelIndex = inTouchPixelIndices[ i ];
        

        unsigned char *sourcePointer = &( sourceData[ pixelIndex ] );

        inChannel[ pixelIndex ] =
            ( sourcePointer[ 0 ] +
              sourcePointer[ cornerOffsetA ] +
              sourcePointer[ cornerOffsetB ] +
              sourcePointer[ cornerOffsetC ] +
              sourcePointer[ cornerOffsetD ] +
              sourcePointer[ sideOffsetA ] +
              sourcePointer[ sideOffsetB ] +
              sourcePointer[ sideOffsetC ] +
              sourcePointer[ sideOffsetD ]
              ) / 9;
        }

    delete [] sourceData;
    

    return;
    }



void Level::findCutPointsRecursive( int inCurrentIndex,
                                    int *inDepths,
                                    int *inLowpoints, int inXLowerLimit ) {
    // recurse on each neighbor

    GridPos currentPos = mIndexToGridMap[ inCurrentIndex ];

    GridPos neighbors[4] = { currentPos, currentPos, currentPos, currentPos };
    
    neighbors[0].x -= 1;
    neighbors[1].x += 1;
    neighbors[2].y -= 1;
    neighbors[3].y += 1;
    
    for( int i=0; i<4; i++ ) {
        
        GridPos n = neighbors[i];

        int nIndex = mSquareIndices[n.y][n.x];

        if( n.x >= inXLowerLimit && mWallFlagsIndexed[ nIndex ] == 1 ) {
            // a floor neighbor

            // already seen?
            if( inDepths[ nIndex ] != -1 ) {
                if( inDepths[ nIndex ] < inLowpoints[ inCurrentIndex ] ) {
                    
                    // take this node's depth as new lowpoint for current node
                    inLowpoints[ inCurrentIndex ] = inDepths[ nIndex ];
                    }
                }
            else {
                // new node!
                
                inDepths[ nIndex ] = inDepths[ inCurrentIndex ] + 1;
                inLowpoints[ nIndex ] = inDepths[ inCurrentIndex ];

                // recurse into it

                findCutPointsRecursive( nIndex, inDepths, inLowpoints, 
                                        inXLowerLimit );
                
                // done exploring this neighbor.

                if( inLowpoints[ nIndex ] == inDepths[ inCurrentIndex ] ) {
                    // this node is a cut vertex!
                    mCutVertexFloorFlags[ inCurrentIndex ] = true;
                    }
                else if( inLowpoints[ nIndex ] < 
                         inLowpoints[ inCurrentIndex ] ) {
                    
                    // new lowpoint reachable from this node
                    inLowpoints[ inCurrentIndex ] = inLowpoints[ nIndex ];
                    }
                    
                }
            }
        }
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


    int numWallLayers = 3;
    
    // leave extra room for blur checking
    int floorSeparationFromEdge = numWallLayers + 1;

    int xLimit = floorSeparationFromEdge;
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

    


    RandomWalker *walker = 
        mWalkerSet.pickWalker( xLimit, 
                               floorSeparationFromEdge, 
                               MAX_LEVEL_W - 1 - floorSeparationFromEdge,
                               MAX_LEVEL_H - 1 - floorSeparationFromEdge );
    

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
            walker = mWalkerSet.pickWalker( 
                xLimit, 
                floorSeparationFromEdge, 
                MAX_LEVEL_W - 1 - floorSeparationFromEdge,
                MAX_LEVEL_H - 1 - floorSeparationFromEdge );
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
    

    // keep a list of grid positions that are on boundary between
    // walls and floor
    // use this for optimized shadow generation later
    int numWallAndFloorBoundaries = 0;
    GridPos wallAndFloorBoundaries[ MAX_LEVEL_SQUARES ];
    
    // only walls that are touching floor
    int numWallBoundaries = 0;
    GridPos wallBoundaries[ MAX_LEVEL_SQUARES ];
    


    // now walls around floor
    // set loop boundaries above so it's safe to check all floor neighbors
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
               
            char floorTouchingWalls = false;
            
            for( int ny=y-1; ny<=y+1; ny++ ) {
                for( int nx=nxStart; nx<=x+1; nx++ ) {
                        
                    char wallFlag = mWallFlags[ny][nx];

                    if( wallFlag == 0 ) {
                        // empty spot adjacent to this floor square
                        floorTouchingWalls = true;
                        
                        mWallFlags[ny][nx] = 2;
                                                
                        mSquareIndices[ny][nx] = mNumUsedSquares;
                        mIndexToGridMap[mNumUsedSquares].x = nx;
                        mIndexToGridMap[mNumUsedSquares].y = ny;
                        
                        // this wall is on boundary too, and hasn't been added
                        wallAndFloorBoundaries[ numWallAndFloorBoundaries ] =  
                            mIndexToGridMap[mNumUsedSquares];
                        wallBoundaries[ numWallBoundaries ] = 
                            mIndexToGridMap[mNumUsedSquares];
                        
                        numWallAndFloorBoundaries++;
                        numWallBoundaries++;

                        if( mSymmetrical ) {
                            // handle "twin" wall square as well
                            GridPos twinPos = mIndexToGridMap[mNumUsedSquares];
                            twinPos.x = MAX_LEVEL_W - 
                                mIndexToGridMap[mNumUsedSquares].x - 1;
                    
                            wallAndFloorBoundaries[ numWallAndFloorBoundaries ]
                                = twinPos;
                            wallBoundaries[ numWallBoundaries ] = 
                                twinPos;;

                            numWallAndFloorBoundaries++;
                            numWallBoundaries++;
                            }

                        mNumUsedSquares++;

                        gridColorsWorking.push_back( 
                            mColors.primary.elements[wallColorIndex] );
                        wallColorIndex = (wallColorIndex + 1) % 3;
                    
                        mNumWallSquares ++;
                        }
                    else if( wallFlag == 2 ) {
                        // wall here already that this floor is touching
                        floorTouchingWalls = true;
                        }
                    }
                }
            
            if( floorTouchingWalls ) {
                wallAndFloorBoundaries[ numWallAndFloorBoundaries ] =
                    mIndexToGridMap[i];
                numWallAndFloorBoundaries++;

                if( mSymmetrical ) {
                    // handle "twin" floor square as well
                    GridPos twinPos = mIndexToGridMap[i];
                    twinPos.x = MAX_LEVEL_W - mIndexToGridMap[i].x - 1;
                    
                    wallAndFloorBoundaries[ numWallAndFloorBoundaries ] =
                        twinPos;
                    numWallAndFloorBoundaries++;
                    }
                }
            }
        }



    // extra wall layers darker and darker
    float darkFactor = 1;
    
    // one layer already created above
    for( int r=0; r<numWallLayers-1; r++ ){
        darkFactor *= 0.5;
        
        // now grow all walls by 1 unit thickness, expanding each wall into
        // any empty neighbors
        int wallNeighborsX[4] = { 1, -1, 0,  0 };
        int wallNeighborsY[4] = { 0,  0, 1, -1 };
    
        int lastRoundWallSquares = mNumWallSquares;

        for( int i=mNumFloorSquares; 
             i<mNumFloorSquares + lastRoundWallSquares; i++ ) {
        
            int x = mIndexToGridMap[i].x;
            int y = mIndexToGridMap[i].y;

            for( int n=0; n<4; n++ ) {
            
                int nx = x + wallNeighborsX[n];
                int ny = y + wallNeighborsY[n];
            
                if( mWallFlags[ny][nx] == 0 ) {
                    // empty spot adjacent to this wall square
                    
                    // not accross sym line
                    if( nx >= xStart || !mSymmetrical ) {

                        mWallFlags[ny][nx] = 2;
                    
                        mSquareIndices[ny][nx] = mNumUsedSquares;
                        mIndexToGridMap[mNumUsedSquares].x = nx;
                        mIndexToGridMap[mNumUsedSquares].y = ny;
                    
                        mNumUsedSquares++;
                    
                        Color c = mColors.primary.elements[wallColorIndex];
                        //c.r *= darkFactor;
                        //c.g *= darkFactor;
                        //c.b *= darkFactor;
                        c.a *= darkFactor;
                    
                        gridColorsWorking.push_back( 
                            c );
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


    mCutVertexFloorFlags = new char[ mNumFloorSquares ];
    
    memset( mCutVertexFloorFlags, false, mNumFloorSquares );
    

    int *depthInSearch = new int[ mNumFloorSquares ];
    int *lowpointInSearch = new int[ mNumFloorSquares ];
    
    for( int i=0; i<mNumFloorSquares; i++ ) {
        depthInSearch[i] = -1;
        lowpointInSearch[i] = 0;
        }
    findCutPointsRecursive( 0, depthInSearch, lowpointInSearch, xLimit );

    delete [] depthInSearch;
    delete [] lowpointInSearch;







    // place rise marker in random floor spot
    // also away from player
    char placed = false;

    while( !placed ) {
        // place first rise marker on right side, if symmetrical, since
        // we only compute cut points on right side
        int x = randSource.getRandomBoundedInt( xLimit, MAX_LEVEL_H - 1 );
        int y = randSource.getRandomBoundedInt( 0, MAX_LEVEL_W - 1 );

        if( mWallFlags[y][x] == 1 &&
            // never place rise markers on cut points.
            ! mCutVertexFloorFlags[ mSquareIndices[y][x] ] ) {
        
            doublePair spot = sGridWorldSpots[y][x];
            
            doublePair playerSpot = {0,0};
            
            if( distance( spot, playerSpot ) > 10 ) {

                placed = true;
                mRisePosition.x = x;
                mRisePosition.y = y;

                mRiseWorldPos.x = mRisePosition.x - MAX_LEVEL_W/2;
                mRiseWorldPos.y = mRisePosition.y - MAX_LEVEL_H/2;

                mRiseWorldPos2 = mRiseWorldPos;
                mRiseWorldPos2.x = - mRiseWorldPos2.x - 1;

                mRisePosition2 = getGridPos( mRiseWorldPos2 );
                }
            }
        }

    mDoubleRisePositions = mSymmetrical;





        
    





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

        // keep hard alpha
        // to preserve outer wall layer darkness
        mGridColorsBlurred[ mSquareIndices[y][x] ].a =
            mHardGridColors[ mSquareIndices[y][x] ].a;
        }

    // copy over
    mSoftGridColors = new Color[ mNumUsedSquares ];
 
    for( int i=0; i<mNumUsedSquares; i++ ) {        
        mSoftGridColors[ i ] = mGridColorsBlurred[ i ];
        }

    
    // get ready to fill in static grid image (one pixel per square)
    int imageSize = 2;
    while( imageSize < MAX_LEVEL_W || imageSize < MAX_LEVEL_H ) {
        imageSize *= 2;
        }
    int imageXOffset = ( imageSize - MAX_LEVEL_W ) / 2;
    int imageYOffset = ( imageSize - MAX_LEVEL_H ) / 2;


    int imagePixels = imageSize * imageSize;    

    
    Image fullGridImage( imageSize, imageSize, 4, true );
    
    double *fullGridChannels[4];
    for( int c=0; c<4; c++ ) {
        fullGridChannels[c] = fullGridImage.getChannel( c );
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
        
        // set (and never change) hard alpha
        // to preserve outer wall layer darkness
        mGridColors[i].a = mHardGridColors[i].a;
        
        GridPos p = mIndexToGridMap[i];
        int imageIndex = 
            ( imageSize - (p.y + imageYOffset ) ) * imageSize + 
            p.x + imageXOffset;
        
        
        fullGridChannels[0][imageIndex] = mGridColors[i].r;
        fullGridChannels[1][imageIndex] = mGridColors[i].g;
        fullGridChannels[2][imageIndex] = mGridColors[i].b;
        fullGridChannels[3][imageIndex] = 1;
        }



    

    
    FastBlurFilter filter;
    fullGridImage.filter( &filter );


    
    // no eye version
    if( outputMapImages ) {
        outputLevelMapImage( &fullGridImage );
        }



    // fill in rise position colors, box around rise marker
    // post-blur so that these blurry rise marker boxes correspond somewhat
    // to creature eyes
    for( int dy=-1; dy<=1; dy++ ) {
        for( int dx=-1; dx<=1; dx++ ) {

            GridPos p = mRisePosition;
            p.y += dy;
            p.x += dx;
            
            int imageIndex = 
                ( imageSize - (p.y + imageYOffset ) ) * imageSize + 
                p.x + imageXOffset;
    
            fullGridChannels[0][imageIndex] += mColors.special.r;
            fullGridChannels[0][imageIndex] /= 2;
            fullGridChannels[1][imageIndex] += mColors.special.g;
            fullGridChannels[1][imageIndex] /= 2;
            fullGridChannels[2][imageIndex] += mColors.special.b;
            fullGridChannels[2][imageIndex] /= 2;
            
            if( mDoubleRisePositions ) {
                p = mRisePosition2;
                p.y += dy;
                p.x += dx;
                
                int imageIndex = 
                    ( imageSize - (p.y + imageYOffset ) ) * imageSize + 
                    p.x + imageXOffset;
                
                fullGridChannels[0][imageIndex] += mColors.special.r;
                fullGridChannels[0][imageIndex] /= 2;
                fullGridChannels[1][imageIndex] += mColors.special.g;
                fullGridChannels[1][imageIndex] /= 2;
                fullGridChannels[2][imageIndex] += mColors.special.b;
                fullGridChannels[2][imageIndex] /= 2;
                }
            }
        }

    // centers over rise markers
    GridPos p = mRisePosition;
            
    int imageIndex = 
        ( imageSize - (p.y + imageYOffset ) ) * imageSize + 
        p.x + imageXOffset;
    
    fullGridChannels[0][imageIndex] = mColors.special.r;
    fullGridChannels[1][imageIndex] = mColors.special.g;
    fullGridChannels[2][imageIndex] = mColors.special.b;
                
    if( mDoubleRisePositions ) {
        p = mRisePosition2;
        
        int imageIndex = 
            ( imageSize - (p.y + imageYOffset ) ) * imageSize + 
            p.x + imageXOffset;
        
        fullGridChannels[0][imageIndex] = mColors.special.r;
        fullGridChannels[1][imageIndex] = mColors.special.g;
        fullGridChannels[2][imageIndex] = mColors.special.b;
        }
    
    
    // eye version (not as interesting looking)
    // leave soft edges before alpha threshold
    //if( outputMapImages ) {
    //    outputLevelMapImage( &fullGridImage );
    //    }

        

    
    // threshold the alpha channel
    for( int p=0; p<imagePixels; p++ ) {
        if( fullGridChannels[3][p] > 0 ) {
            fullGridChannels[3][p] = 1;
            }
        }

    mFullMapSprite = fillSprite( &fullGridImage, false );
    

    
    // now make a shadow sprite for the walls

    
    int blowUpFactor = shadowBlowUpFactor;
    int blownUpSize = imageSize * blowUpFactor;

    int numBlowupPixels = blownUpSize * blownUpSize;
    
    // opt:  no need to operate on all four channels
    // just process alpha channel now

    // opt:  do all this processing with uchars instead of doubles
    unsigned char *fullGridChannelsBlownUpAlpha =
        new unsigned char[ numBlowupPixels ];

    memset( fullGridChannelsBlownUpAlpha, 0, numBlowupPixels );
    

    // only process used sub-region of blown up image
    // don't waste time on blank areas outside walls
    int blowUpStartX = imageXOffset * blowUpFactor;
    int blowUpStartY = imageYOffset * blowUpFactor;
    int blowUpEndX = ( imageXOffset + MAX_LEVEL_W ) * blowUpFactor;
    int blowUpEndY = ( imageYOffset + MAX_LEVEL_H ) * blowUpFactor;



    int nunBlowUpPixelsPerSquare = blowUpFactor * blowUpFactor;
    
    int numBoundaryBlowUpPixels = 
        nunBlowUpPixelsPerSquare * numWallAndFloorBoundaries;

    int *boundaryBlowUpPixelIndices = 
        new int[ numBoundaryBlowUpPixels ];


    
    // blow up with nearest neighbor
    // ignore areas that are not on wall/floor boundary

    // track pixels in blow up image as they are touched
    int boundaryPixelIndex = 0;

    for( int i=0; 
         i<numWallAndFloorBoundaries; i++ ) {
        
        GridPos boundaryPos = wallAndFloorBoundaries[ i ];

        unsigned char alphaValue = 0;
        if( mWallFlags[boundaryPos.y][boundaryPos.x] == 2 ) {
            alphaValue = 255;
            }

        int x = boundaryPos.x + imageXOffset;
        int y = imageSize - ( boundaryPos.y + imageYOffset );
        
        for( int blowUpY= y * blowUpFactor; 
             blowUpY< y * blowUpFactor + blowUpFactor; 
             blowUpY++ ) {

            for( int blowUpX= x * blowUpFactor; 
                 blowUpX< x * blowUpFactor + blowUpFactor; 
                 blowUpX++ ) {
                

                int imageIndex = blowUpY * blownUpSize + blowUpX;
                
                fullGridChannelsBlownUpAlpha[ imageIndex ] = alphaValue;

                boundaryBlowUpPixelIndices[ boundaryPixelIndex ] = imageIndex;
                
                boundaryPixelIndex++;
                }
            }
        }
    


    FastBoxBlurFilter filter2;
        
    filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                            boundaryBlowUpPixelIndices,
                            numBoundaryBlowUpPixels,
                            blownUpSize, blownUpSize,
                            blowUpStartX, blowUpStartY,
                            blowUpEndX, blowUpEndY );
    
    
    // add a bit of noise
    
    double noiseFraction = 0.75;
    
    for( int i=0; i<numBoundaryBlowUpPixels; i++ ) {
        
        int pixelIndex = boundaryBlowUpPixelIndices[i];

        double oldValue = fullGridChannelsBlownUpAlpha[pixelIndex];

        if( oldValue > 0 ) {
            int tweakedValue =
                (int)( oldValue - 
                       randSource.
                       getRandomBoundedDouble( -oldValue * noiseFraction, 
                                               oldValue * noiseFraction ) );
            
            // clamp
            if( tweakedValue < 0 ) {
                tweakedValue = 0;
                }
            else if( tweakedValue > 255 ) {
                tweakedValue = 255;
                }

            fullGridChannelsBlownUpAlpha[pixelIndex] = tweakedValue;
            }
        }
    
    // blur again, post-noise

    filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                            boundaryBlowUpPixelIndices,
                            numBoundaryBlowUpPixels, 
                            blownUpSize, blownUpSize,
                            blowUpStartX, blowUpStartY,
                            blowUpEndX, blowUpEndY );
    
    filter2.applySubRegion( fullGridChannelsBlownUpAlpha, 
                            boundaryBlowUpPixelIndices,
                            numBoundaryBlowUpPixels, 
                            blownUpSize, blownUpSize,
                            blowUpStartX, blowUpStartY,
                            blowUpEndX, blowUpEndY );

    mFullMapWallShadowSprite = 
        fillSpriteAlphaOnly( fullGridChannelsBlownUpAlpha, 
                             blownUpSize, 
                             blownUpSize );

    delete [] fullGridChannelsBlownUpAlpha;
    delete [] boundaryBlowUpPixelIndices;
    


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

        delete [] mCutVertexFloorFlags;
        

        for( int i=0; i<mEnemies.size(); i++ ) {
            Enemy *e = mEnemies.getElement( i );
            if( e->sprite != mLastEnterPointSprite ) {
                e->sprite->compactSprite();
                }
            }    
        if( &mPlayerSprite != mLastEnterPointSprite ) {
            mPlayerSprite.compactSprite();
            }
        mDataGenerated = false;

        freeSprite( mFullMapSprite );
        freeSprite( mFullMapWallShadowSprite );
        }
    
    }



//#include "minorGems/system/Thread.h"



Level::Level( ColorScheme *inPlayerColors, NoteSequence *inPlayerMusicNotes,
              ColorScheme *inColors, 
              RandomWalkerSet *inWalkerSet,
              NoteSequence *inMusicNotes,
              PowerUpSet *inSetPlayerPowers,
              int inLevelNumber, char inSymmetrical, char inInsideEnemy,
              char inInsidePowerUp,
              char inIsKnockDown,
              int inTokenRecursionDepth,
              int inParentEnemyDifficultyLevel,
              int inParentTokenLevel,
              int inParentFloorTokenLevel,
              int inParentLevelDifficulty ) 
        : mLevelNumber( inLevelNumber ),
          mTokenRecursionDepth( inTokenRecursionDepth ),
          mPlayerSprite( inPlayerColors ) {

    /*
    if( shouldPowerUpsBeRigged() && mLevelNumber < 9 ) {
        mPlayerPowers = new PowerUpSet( 0 );
        }
    else {
        mPlayerPowers = new PowerUpSet( inLevelNumber - 3 );
        }
    */


    if( inSetPlayerPowers != NULL ) {
        // copy player powers (intentionally entering
        mPlayerPowers = new PowerUpSet( inSetPlayerPowers );
        }
    else {
        // this is the next level up, or an unintentional knock-down

        // Try power-ups for player ALWAYS starting off empty
        // getting knocked down a level, and having to fight through with a
        // pea shooter, is much more serious this way
        // give two heart token (to give player default of 3 health)
        mPlayerPowers = new PowerUpSet( 0 );
        mPlayerPowers->mPowers[1].powerType = powerUpHeart;
        mPlayerPowers->mPowers[1].level = 1;
        mPlayerPowers->mPowers[2].powerType = powerUpHeart;
        mPlayerPowers->mPowers[2].level = 1;
        
        if( ! inIsKnockDown ) {
            // make basic bullets faster, so shooting feels better at
            // start of game
            mPlayerPowers->mPowers[0].powerType = powerUpBulletSpeed;
            mPlayerPowers->mPowers[0].level = 1;
            }
        }
    
    



    int health, max;
    getPlayerHealth( &health, &max );
    mPlayerHealth = max;
    
    mPlayerHealthBarJittering = false;
    mPlayerHealthBarJitterProgress = 0;
    

    mNextEnemyPathFindIndex = 0;
    
    
    //Thread::staticSleep( 1000 );
    

    if( !sGridWorldSpotsComputed ) {
        
        // precompute to-world coord mapping
        // also clear blur hit map
        for( int y=0; y<MAX_LEVEL_H; y++ ) {
            for( int x=0; x<MAX_LEVEL_W; x++ ) {
                
                sGridWorldSpots[y][x].x = x - MAX_LEVEL_W/2;
                sGridWorldSpots[y][x].y = y - MAX_LEVEL_H/2;
                blurHitMap[y][x] = false;
                }
            }

        sGridWorldSpotsComputed = true;
        }


    
    randSource.saveState();
    mRandSeedState = randSource.getSavedState();

    mDataGenerated = false;
    mSymmetrical = inSymmetrical;
    
    mInsideEnemy = inInsideEnemy;
    mInsidePowerUp = inInsidePowerUp;
    mKnockDown = inIsKnockDown;
    

    if( inColors != NULL ) {
        // copy
        mColors = *( inColors );
        }
    // else use randomly-generated mColors from stack
    

    if( inWalkerSet != NULL ) {
        mWalkerSet = *( inWalkerSet );
        }
    // else use randomly-generated walker set from stack

    if( inMusicNotes != NULL ) {
        mHarmonyNotes = *( inMusicNotes );
        }
    else {    
        // else randomly-generated notes
        // copy player timbre
        mHarmonyNotes = generateRandomNoteSequence( PARTS - 2 );
        }
    
    if( inInsidePowerUp ) {
        // immediately inside power-up (which may be inside enemy)
        // long, random beat
        mRiseDrumBeat = generateRandomDrumSequence( 8 );
        }
    else if( mInsideEnemy ) {
        // short random beat
        mRiseDrumBeat = generateRandomDrumSequence( 4 );    
        }
    else {
        // inside player
        // straight techno beat
        mRiseDrumBeat = generateStraightDrumSequence( 4 );
        }
    
    if( inPlayerMusicNotes != NULL ) {
        mPlayerMusicNotes = *( inPlayerMusicNotes );
        }
    else {
        
        // else randomly-generated notes

        // alternate part length with harmony notes to create nice
        // phase patterns

        int partLength = 16;
        
        if( mHarmonyNotes.partLength == partLength ) {
            partLength -= 4;
            }

        mPlayerMusicNotes = generateRandomNoteSequence( PARTS - 2,
                                                        partLength );
        }    


    mFrozen = false;
    mPlayerImmortalSteps = 0;
    
    mDrawFloorEdges = true;
    mEdgeFadeIn = 0.0f;
    mLastComputedEdgeFade = 0.0f;
    
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

    mPlayerStartPos = mPlayerPos;
    


    mPlayerVelocity.x = 0;
    mPlayerVelocity.y = 0;

    mPlayerStepsUntilNextGlowTrail = 
        (int)( stepsBetweenGlowTrails / frameRateFactor );
    
    mEnteringMouse = false;
    

    
    generateReproducibleData();


    mStartStepsToRiseMarker = getStepsToRiseMarker( mPlayerStartPos );
    mLastComputedStepsToRiseMarker = mStartStepsToRiseMarker;
    
    mLastCloseStepsToRiseMarker = mStartStepsToRiseMarker;
    mGettingFartherAwayFromRiseMarker = false;


    
    // negative levels get harder and harder the farther you go down
    mDifficultyLevel = mLevelNumber;
    if( mDifficultyLevel < 0 ) {
        mDifficultyLevel *= -1;
        }



    // make sure level is appropriately difficult based on parent level
    
    if( mLevelNumber >= 0 && mDifficultyLevel < inParentLevelDifficulty - 1 ) {
        
        if( !mKnockDown ) {
            // no more that one step less difficult
            mDifficultyLevel = inParentLevelDifficulty - 1;
            }
        else {
            // give player a break on knock-down
            // halfway between level's base difficulty and difficulty
            // suggested by parent
            mDifficultyLevel = 
                ( mDifficultyLevel + inParentLevelDifficulty - 1 ) / 2;
            }
        }
    else if( mLevelNumber < 0 && 
             mDifficultyLevel < inParentLevelDifficulty + 1 ) {
        
        // difficulty inverted for negative levels

        if( !mKnockDown ) {
            // no less than one step more difficult
            mDifficultyLevel = inParentLevelDifficulty + 1;
            }
        else {   
            // give player a break on knock-down
            // halfway between level's base difficulty and difficulty
            // suggested by parent
            mDifficultyLevel = 
                ( mDifficultyLevel + inParentLevelDifficulty + 1 ) / 2;
            }
        }
        


    // levels get harder the deeper we go inside power-ups
    int tokenFactor;

    // or if we enter an already-high (or low!) token 
    // (through previous leveling) that is inside player

    if( !mInsideEnemy ) {
        if( inInsidePowerUp ) {
            // recursion depth equal to parent token level
            tokenFactor = inParentTokenLevel;
            
            if( inParentTokenLevel > 1 || mTokenRecursionDepth > 1 ) {
                tokenFactor++;
                }
            }
        else {
            // no token factor
            tokenFactor = 0;
            }        
        }
    else {
        // inside enemy
        
        
        if( inInsidePowerUp ) {
            
            // can't get useful information from parent token, because it rises
            // at higher levels and offers no indication of recursion depth
            
            // instead, use tracked recursion depth directly
            tokenFactor = mTokenRecursionDepth;


            if( inParentTokenLevel != inParentFloorTokenLevel ) {
                // we're inside a token that is already lower or higher than
                // it should be

                // force an effectively deeper/shallower recursion when we
                // re-enter a power-up that we've already entered before.
                tokenFactor += 
                    ( inParentFloorTokenLevel - inParentTokenLevel );

                if( tokenFactor < 0 ) {
                    tokenFactor = 0;
                    }
                }            
            }
        else {
            // no token factor
            tokenFactor = 0;
            }    
        }
    
    
    if( tokenFactor > 0 ) {
        // raise difficulty level based on recursion into power-ups

        mDifficultyLevel = mDifficultyLevel + tokenFactor;
        
        if( tokenFactor > 1 ) {
            mDifficultyLevel += mDifficultyLevel / 2;
            }
        
        }




    // place enemies in random floor spots
    int musicPartIndex = 0;
    
    // fewer enemies in lower levels
    int maxNumEnemies = 10;

    if( mDifficultyLevel * 2 < 10 ) {
        maxNumEnemies = mDifficultyLevel * 2;
        }
    
    if( mLevelNumber == 0 ) {
        // always have 0 enemies at level zero, as a safe buffer zone
        // against inverse-difficulty-progression negative levels.

        // Don't want player to get knocked negative by accident.
        maxNumEnemies = 0;
        }
    

    // count number of enemies that have follow behavior
    int followCount = 0;
    
    int maxFollowCount = ( mDifficultyLevel - 12 ) / 2;
    
    if( maxFollowCount < 1 ) {
        maxFollowCount = 1;
        }
    else if( maxFollowCount > 4 ) {
        maxFollowCount = 4;
        }
    
    
    // limit how many enemies are near each other at lower levels
    int maxClusterSize = 2;
    
    if( mDifficultyLevel > 10 ) {
        maxClusterSize += ( mDifficultyLevel - 10  );
        }
    


    for( int i=0; i<maxNumEnemies; i++ ) {
        
        // pick random floor spot until found one away from player
        
        int floorPick = 
            randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
        
        char hit = false;
        
        int numTries = 0;
        
        while( ! hit && numTries < 20 ) {
            numTries++;
            
            doublePair spot = *( mGridWorldSpots[ floorPick ] );
                    
            
            char fixedSpot = false;
            

            if( i == 0 || ( mSymmetrical && i == 1 ) ) {
                
                fixedSpot = true;
                
                if( i == 0 ) {
                    spot = mRiseWorldPos;
                    }
                else {
                    spot = mRiseWorldPos2;
                    }
                }
            
            // count enemies that are already near this spot
            // limit on too many enemies right in same area
            int numClose = 0;
            
            for( int j=0; j<mEnemies.size(); j++ ) {
                Enemy *otherEnemy = mEnemies.getElement( j );
                
                double otherDistance = distance( spot, otherEnemy->position );

                if( otherDistance < 15 ) {
                    numClose++;
                    }                
                }

            // keep enemies away from player starting spot (fair)
            // unless on fixed spots

            doublePair playerSpot = {0,0};
            
            if( fixedSpot || 
                ( numClose < maxClusterSize && 
                  distance( spot, playerSpot ) > 20 ) ) {
                
                // random starting velocity
                doublePair baseMoveDirection = 
                    { randSource.getRandomBoundedDouble( -1, 1 ), 
                      randSource.getRandomBoundedDouble( -1, 1 ) };
                baseMoveDirection = normalize( baseMoveDirection );
                
                // start off with basic speed
                doublePair v = mult( baseMoveDirection, 
                                     maxEnemySpeed * frameRateFactor );
                doublePair a = { 0, 0 };
                
                char allowFollow = false;
                
                if( ! fixedSpot && followCount < maxFollowCount ) {
                    allowFollow = true;
                    }

                // don't have any leveled-up enemies until all power-ups
                // are available
                int enemyDifficultyLevel = mDifficultyLevel - 5;
                
                PowerUpSet *p = new PowerUpSet( enemyDifficultyLevel, 
                                                true, allowFollow );
                
                if( p->containsFollow() ) {
                    followCount++;
                    }
                


                RandomWalkerSet walkerSet;
                
                int maxHealth = getEnemyMaxHealth( p );

                Enemy e = { spot, v, a, baseMoveDirection, 20, 
                            randSource.getRandomBoundedInt( 0, 10 ),
                            new EnemySprite(),
                            p,
                            maxHealth,
                            maxHealth,
                            0,
                            spot,
                            NULL,
                            false,
                            randSource.getRandomBoundedDouble( 0.1, 0.8 ),
                            walkerSet,
                            generateRandomNoteSequence( musicPartIndex ),
                            (int)( stepsBetweenGlowTrails / 
                                   frameRateFactor ),
                            enemyDifficultyLevel,
                            // use enemy's initial index as its bullet marker
                            i };
                
                musicPartIndex ++;
                
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
    
    
    

    // no power ups in lowest levels during tutorial
    int maxNumPowerUps = 10;
    
    if( mLevelNumber < 3 && shouldPowerUpsBeRigged() ) {
        maxNumPowerUps = 0;
        }

    // keep 10 on floor during tutorial to give player enough to experiment
    // with
    // but forget about it if level 10 already reached (2 above where
    // entering things is first explained)
    if( ! isFullTutorialRunning() || levelAlreadyVisited( 10 ) || 
        mLevelNumber >= 10 ) {

        // random number of power ups, chosen from a probability distribution
        // 8 possible values, in [3..10]

        double probabilities[8];
        
        // geometric
        for( int i=0; i<8; i++ ) {
            probabilities[i] = 0.3 * pow( 0.7, i );
            }
        
        ProbabilityMassFunction pmf( &randSource, 8, probabilities );
        
        
        maxNumPowerUps = pmf.sampleElement() + 3;
        }
    

    // skip to power-up parts (even if not all enemy parts used above)
    musicPartIndex = 10;
    
    // pick a random, unused part, in range, for each power up
    // (since we're placing <10 power ups many times, and we don't want
    // to get stuck with less variety when we have 10 instruments available)
    char musicPartsUsed[ PARTS ];
    for( int p=musicPartIndex; p<PARTS; p++ ) {
        musicPartsUsed[p] = false;
        }
    



    mFloorTokenLevel = 1;
    
    if( mInsideEnemy ) {
        // set to max, encourage sub-recursion into enemy        
        
        if( inInsidePowerUp ) {
            // floor power-ups grow weaker by one step the deeper we
            // recurse into sub-tokens

            // this is based purely in parent token level
            mFloorTokenLevel = inParentTokenLevel - 1;
            }
        else {
            // just inside an enemy, set based on difficulty of parent only
            mFloorTokenLevel = inParentEnemyDifficultyLevel / POWER_SET_SIZE;
            }
        
        
        if( mFloorTokenLevel < 1 ) {
            mFloorTokenLevel = 1;
            }
        }
    else {
        // set to min... encourage more sub-recursion
        // into self
        mFloorTokenLevel = 1;

        if( inInsidePowerUp ) {    
            // floor power ups are stronger the deeper we recurse
            // into a power-up
            
            // but first recursion keeps 1 tokens
            
            if( inParentTokenLevel == 1 && mTokenRecursionDepth == 1 ) {
                mFloorTokenLevel = 1;
                }
            else {
                // deeper recursion, or an already-raised parent token
                // that has been re-entered
                mFloorTokenLevel += inParentTokenLevel;
                }
            }
        }


    int maxFloorTokenLevel, minFloorTokenLevel;
    
    if( mInsideEnemy ) {
        maxFloorTokenLevel = mFloorTokenLevel;
        minFloorTokenLevel = 1;
        }
    else {
        // inside player or power-up inside player

        // raise whole range the deeper we go into power-ups
        maxFloorTokenLevel = ( mDifficultyLevel / 2 ) * mFloorTokenLevel;
        minFloorTokenLevel = mFloorTokenLevel;    

        if( maxFloorTokenLevel < minFloorTokenLevel ) {
            maxFloorTokenLevel = minFloorTokenLevel;
            }

        if( isFullTutorialRunning() && ! levelAlreadyVisited( 10 ) &&
            mLevelNumber < 10 ) {
            // don't let player stumble into a dangerous power-up level
            // during tutorial
            // (but stop protecting them if they've already risen to level 10)
            maxFloorTokenLevel = mFloorTokenLevel;
            }
        }
    


    int numValues = maxFloorTokenLevel - minFloorTokenLevel + 1;
    double *probabilities = new double[ numValues ];
    
    double pmfParam = 0.3;

    if( mInsideEnemy ) {
        // weights increase toward higher token levels (harder to find
        // low-value tokens inside enemy)

        // inverted geometric
        for( int i=0; i<numValues; i++ ) {
            probabilities[i] = pmfParam * pow( 1 - pmfParam, 
                                               ( numValues - i - 1 ) );
            }
        }
    else {
        // weights increase toward lower token levels (harder to find
        // high-value tokens inside player)

        // straight geometric
        for( int i=0; i<numValues; i++ ) {
            probabilities[i] = pmfParam * pow( 1 - pmfParam, i );
            }
        }
    
    ProbabilityMassFunction powerLevelPMF( &randSource, numValues,
                                           probabilities );
    delete [] probabilities;

    
    

    // for tutorial-mode power-up placement
    // (every other power-up is spread)
    char spreadToggle = false;
    // if knocked back down:  half or 1/3 of power-ups are hearts
    char heartToggle = 0;


    for( int i=0; i<maxNumPowerUps; i++ ) {

        // pick random floor spot until found one not on rise marker
        // or existing power token
        
        int floorPick = 
            randSource.getRandomBoundedInt( 0, mNumFloorSquares - 1 );
        
        char hit = false;
        
        int numTries = 0;

        while( ! hit && numTries < 20 ) {
            numTries++;

            
            GridPos pickPos = mIndexToGridMap[ floorPick ];
            
            doublePair worldPos = 
                sGridWorldSpots[ pickPos.y ][ pickPos.x ];
            
            doublePair playerSpot = {0,0};

            if( mRisePosition.x != pickPos.x
                &&
                // in case of double rise spot
                mRisePosition.x != - pickPos.x
                &&
                mRisePosition.y != pickPos.y 
                &&
                // not too close to player staring spot
                distance( worldPos, playerSpot ) > 5 ) {
                
                
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
                

                PowerUp mainPower = getRandomPowerUp( mFloorTokenLevel );
                
                // NEVER put empty power-ups on the floor
                while( mainPower.powerType == powerUpEmpty ) {
                    // re-roll
                    mainPower = getRandomPowerUp( mFloorTokenLevel );
                    }
                
                // all tokens on floor have levels sampled from PMF
                mainPower.level = 
                    powerLevelPMF.sampleElement() + minFloorTokenLevel;
                


                if( shouldPowerUpsBeRigged() ) {

                    if( mLevelNumber == 3 || mLevelNumber == 4 ) {
                        
                        if( mLevelNumber == 3 ) {
                            // spread first, easiest to notice
                            mainPower.powerType = powerUpSpread;
                            }
                        else if( mLevelNumber == 4 ) {
                            mainPower.powerType = powerUpBulletSize;
                            }
                        
                        // always give player option of hearts
                        // place 50%
                        heartToggle ++;
                        if( heartToggle == 2 ) {
                            heartToggle = 0;
                            mainPower.powerType = powerUpHeart;
                            }
                        }
                    else if( mLevelNumber == 5 ) {
                        
                        char heartPlaced = false;
                        // always give player option of hearts
                        // place 50% hearts
                        heartToggle ++;
                        if( heartToggle == 3 ) {
                            heartToggle = 0;
                            mainPower.powerType = powerUpHeart;
                            heartPlaced = true;
                            }
                        
                        if( !heartPlaced ) {    
                            // 50/50
                            if( spreadToggle ) {
                                mainPower.powerType = powerUpSpread;
                                }
                            else {
                                mainPower.powerType = powerUpBulletSize;
                                }
                            spreadToggle = !spreadToggle;
                            }
                        
                        }
                    }


                // powers must sum to main power
                PowerUpSet *subPowers = new PowerUpSet( mainPower.level,
                                                        mainPower.powerType );
                
                int thisPartIndex = 
                    randSource.getRandomBoundedInt( musicPartIndex,
                                                    musicPartIndex + 9 );
                
                while( musicPartsUsed[ thisPartIndex ] ) {
                    thisPartIndex = 
                        randSource.getRandomBoundedInt( musicPartIndex,
                                                        musicPartIndex + 9 );
                    }
                
                musicPartsUsed[ thisPartIndex ] = true;

                PowerUpToken t = { mainPower,
                                   pickPos,
                                   worldPos,
                                   new PowerUpSprite( mainPower, 
                                                      subPowers ),
                                   subPowers,
                                   generateRandomNoteSequence( 
                                       thisPartIndex ),
                                   (int)( stepsBetweenGlowTrails / 
                                          frameRateFactor ) };

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
    mPlayerSprite.decompactSprite();
    }




void Level::setPlayerPos( doublePair inPos ) {
    mPlayerPos = inPos;
    }

void Level::setPlayerVelocity( doublePair inVelocity ) {
    mPlayerVelocity = inVelocity;
    }


void Level::setMousePos( doublePair inPos ) {
    mMousePos = inPos;
    }

void Level::setEnteringMouse( char inEntering ) {
    mEnteringMouse = inEntering;
    }




void Level::setItemWindowPosition( doublePair inPosition, itemType inType ) {
    int index;

    char windowAlreadySet = mWindowSet;

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

    if( mWindowSet && !windowAlreadySet ) {
        mLastComputedFastWindowFade = 1;
        }
        
    }



typedef struct pathSearchRecord {
        GridPos pos;
        
        int squareIndex;
        
        int cost;
        double estimate;
        double total;
        
        // index of pred in done queue
        int predIndex;
        
        // links to create structure of search queue
        pathSearchRecord *nextSearchRecord;

    } pathSearchRecord;


double getGridDistance( GridPos inA, GridPos inB ) {
    int dX = inA.x - inB.x;
    int dY = inA.y - inB.y;
    
    // manhattan distance
    return abs( dX ) + abs( dY );
    //return sqrt( dX * dX + dY * dY );
    }


char equal( GridPos inA, GridPos inB ) {
    return inA.x == inB.x && inA.y == inB.y;
    }



typedef struct pathSearchQueue {
        pathSearchRecord *head;
    } pathSearchQueue;


// returns true if A better than B (sorting function
inline static char isRecordBetter( pathSearchRecord *inA, 
                                   pathSearchRecord *inB ) {
    
    if( inA->total <= inB->total ) {
        
        if( inA->total == inB->total ) {
            
            // pick record with lower estimated cost to break tie
            if( inA->estimate < inB->estimate ) {
                return true;
                }
            }
        else {
            return true;
            }
        }
    return false;
    }

    

// sorted insertion
void insertSearchRecord( pathSearchQueue *inQueue, 
                         pathSearchRecord *inRecordToInsert ) {
    
    // empty queue
    if( inQueue->head == NULL ) {
        inQueue->head = inRecordToInsert;
        return;
        }

    // better than head
    if( isRecordBetter( inRecordToInsert, inQueue->head ) ) {
        inRecordToInsert->nextSearchRecord = inQueue->head;    
        inQueue->head = inRecordToInsert;
        return;
        }
    
    // general case, search for spot to insert
    
    pathSearchRecord *currentRecord = inQueue->head;
    pathSearchRecord *nextRecord = currentRecord->nextSearchRecord;

    while( nextRecord != NULL ) {
        
        if( isRecordBetter( inRecordToInsert, nextRecord ) ) {
            
            // insert here
            inRecordToInsert->nextSearchRecord = nextRecord;
            
            currentRecord->nextSearchRecord = inRecordToInsert;
            return;
            }
        else {
            // keep going
            currentRecord = nextRecord;
            nextRecord = currentRecord->nextSearchRecord;
            }
        }
    

    // hit null, insert at end
    currentRecord->nextSearchRecord = inRecordToInsert;
    }



// sorted removal
pathSearchRecord *pullSearchRecord( pathSearchQueue *inQueue, 
                                    int inSquareIndex ) {

    if( inQueue->head == NULL ) {
        return NULL;
        }

    if( inQueue->head->squareIndex == inSquareIndex ) {
        // pull head
        pathSearchRecord *currentRecord = inQueue->head;

        inQueue->head = currentRecord->nextSearchRecord;

        currentRecord->nextSearchRecord = NULL;

        return currentRecord;
        }
    

    pathSearchRecord *previousRecord = inQueue->head;
    pathSearchRecord *currentRecord = previousRecord->nextSearchRecord;
    

    while( currentRecord != NULL && 
           currentRecord->squareIndex != inSquareIndex ) {
    
        previousRecord = currentRecord;
        
        currentRecord = previousRecord->nextSearchRecord;
        }
    
    if( currentRecord == NULL ) {
        return NULL;
        }
    
    // else pull it

    // skip it in the pointer chain
    previousRecord->nextSearchRecord = currentRecord->nextSearchRecord;
    
    
    currentRecord->nextSearchRecord = NULL;

    return currentRecord;
    }





GridPos Level::pathFind( GridPos inStart, doublePair inStartWorld, 
                         GridPos inGoal, double inMoveSpeed,
                         int *outNumStepsToGoal,
                         GridPos **outFullPath ) {

    // watch for degen case where start and goal are equal
    if( equal( inStart, inGoal ) ) {
        
        if( outNumStepsToGoal != NULL ) {
            *outNumStepsToGoal = 0;
            }
        if( outFullPath != NULL ) {
            *outFullPath = NULL;
            }
        return inStart;
        }
        


    // insertion-sorted queue of records waiting to be searched
    pathSearchQueue recordsToSearch;

    
    // keep records here, even after we're done with them,
    // to ensure they get deleted
    SimpleVector<pathSearchRecord*> searchQueueRecords;


    SimpleVector<pathSearchRecord> doneQueue;
    
    

    // quick lookup of touched but not done squares
    // indexed by floor square index number
    char *openMap = new char[ mNumFloorSquares ];
    memset( openMap, false, mNumFloorSquares );

    char *doneMap = new char[ mNumFloorSquares ];
    memset( doneMap, false, mNumFloorSquares );

            
    pathSearchRecord startRecord = 
        { inStart,
          mSquareIndices[ inStart.y ][ inStart.x ],
          0,
          getGridDistance( inStart, inGoal ),
          getGridDistance( inStart, inGoal ),
          -1,
          NULL };

    // can't keep pointers into a SimpleVector 
    // (change as vector expands itself)
    // push heap pointers into vector instead
    pathSearchRecord *heapRecord = new pathSearchRecord( startRecord );
    
    searchQueueRecords.push_back( heapRecord );
    
    
    recordsToSearch.head = heapRecord;


    openMap[ startRecord.squareIndex ] = true;
    


    char done = false;
            
            
    //while( searchQueueRecords.size() > 0 && !done ) {
    while( recordsToSearch.head != NULL && !done ) {

        // head of queue is best
        pathSearchRecord bestRecord = *( recordsToSearch.head );
        
        recordsToSearch.head = recordsToSearch.head->nextSearchRecord;


        if( false )
            printf( "Best record found:  "
                    "(%d,%d), cost %d, total %f, "
                    "pred %d, this index %d\n",
                    bestRecord.pos.x, bestRecord.pos.y,
                    bestRecord.cost, bestRecord.total,
                    bestRecord.predIndex, doneQueue.size() );
        
        doneMap[ bestRecord.squareIndex ] = true;
        openMap[ bestRecord.squareIndex ] = false;

        
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

                    int neighborSquareIndex = 
                        mSquareIndices[ neighbors[n].y ][ neighbors[n].x ];
                    
                    char alreadyOpen = openMap[ neighborSquareIndex ];
                    char alreadyDone = doneMap[ neighborSquareIndex ];
                    
                    if( !alreadyOpen && !alreadyDone ) {
                        
                        // for testing, color touched nodes
                        // mGridColors[ neighborSquareIndex ].r = 1;
                        
                        // add this neighbor
                        double dist = 
                            getGridDistance( neighbors[n], 
                                             inGoal );
                            
                        // track how we got here (pred)
                        pathSearchRecord nRecord = { neighbors[n],
                                                     neighborSquareIndex,
                                                     cost,
                                                     dist,
                                                     dist + cost,
                                                     predIndex,
                                                     NULL };
                        pathSearchRecord *heapRecord =
                            new pathSearchRecord( nRecord );
                        
                        searchQueueRecords.push_back( heapRecord );
                        
                        insertSearchRecord( 
                            &recordsToSearch, heapRecord );

                        openMap[ neighborSquareIndex ] = true;
                        }
                    else if( alreadyOpen ) {
                        pathSearchRecord *heapRecord =
                            pullSearchRecord( &recordsToSearch,
                                              neighborSquareIndex );
                        
                        // did we reach this node through a shorter path
                        // than before?
                        if( cost < heapRecord->cost ) {
                            
                            // update it!
                            heapRecord->cost = cost;
                            heapRecord->total = heapRecord->estimate + cost;
                            
                            // found a new predecessor for this node
                            heapRecord->predIndex = predIndex;
                            }

                        // reinsert
                        insertSearchRecord( &recordsToSearch, heapRecord );
                        }
                            
                    }
                }
                    

            }
        }

    delete [] openMap;
    delete [] doneMap;
    
    for( int i=0; i<searchQueueRecords.size(); i++ ) {
        delete *( searchQueueRecords.getElement( i ) );
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

    // don't check for straight-line path for EVERY step, all the way
    // from goal backwards... wasteful in most situations.

    // instead, compile full path back to start, and then walk forward
    // from start, looking for first spot that is not straight-line accessible.

    SimpleVector<GridPos> finalPath;
    finalPath.push_back( currentRecord->pos );

    while( ! equal(  predRecord->pos, inStart ) && ! done ) {
        currentRecord = predRecord;
        finalPath.push_back( currentRecord->pos );

        predRecord = 
            doneQueue.getElement( currentRecord->predIndex );
        
        }

    if( outNumStepsToGoal != NULL ) {
        *outNumStepsToGoal = finalPath.size() - 1;
        }
    if( outFullPath != NULL ) {
        *outFullPath = finalPath.getElementArray();
        }
    
    

    // found next step away from start
    
    // next, walk forward through path as long as there is a straight-light,
    // unobstructed path from start pos to the path spot

    char stopAtFirstObstruction = true;
    

    doublePair finalGoalWorld = sGridWorldSpots[ inGoal.y ][ inGoal.x ];
    
    if( distance( inStartWorld, finalGoalWorld ) < 29 ) {
        
        // path steps are happening on-screen

        // keep searching past first obstruction, because grid paths can get
        // blocked by obstructions that straight-line paths are not blocked by
        
        // makes on-screen path-finding smoother, while keeping off-screen
        // path-finding fast
        stopAtFirstObstruction = false;
        }
    
    

    // last step that we could reach obstruction-free
    GridPos lastGoodStep = *( finalPath.getElement( finalPath.size() - 1 ) );
    

    int nextStepIndex = finalPath.size() - 2;
    
    while( nextStepIndex >= 0  ) {
        GridPos nextStep = *( finalPath.getElement( nextStepIndex ) );

        // straight-line, unobstructed path from start to next step?

        doublePair stepPos = { inStartWorld.x, inStartWorld.y };
        doublePair goalPos = 
            sGridWorldSpots[ nextStep.y ][ nextStep.x ];
        GridPos stepGridPos = inStart;
        
        doublePair stepDelta = mult( normalize( sub( goalPos, stepPos ) ),
                                     inMoveSpeed );
        
        while( !equal( stepGridPos, nextStep ) &&
               mWallFlags[ stepGridPos.y ][ stepGridPos.x ] == 1 ) {
            
            stepPos = add( stepPos, stepDelta );
            stepGridPos = getGridPos( stepPos );
            }
        
        if( ! equal( stepGridPos, nextStep ) ) {
            
            // can't reach nextStep with straight line...
            // gone too far?

            if( stopAtFirstObstruction ) {
                // stick with our last obstruction-free step
                return lastGoodStep;
                }
            else {
                // keep going past the obstruction
                nextStepIndex --;
                }
            }
        else {
            // new step reachable with straight line, keep going
            lastGoodStep = nextStep;
            nextStepIndex --;
            }
        }
    
    // straight-line path all the way to goal 
    // OR further steps have obstructions all the way to goal
    // (ran out of nextSteps )
    return lastGoodStep;                
    }




void Level::generateEnemyDestructionSmoke( Enemy *inE ) {

    ColorScheme colors = inE->sprite->getColors();
    
    Color borderColor = colors.primary.elements[3];
                                
    // big smoke for border, dead center
    HitSmoke s = { inE->position, 0, 0.75, 3, 
                   borderColor };
                                
    mSmokeClouds.push_back( s );
                                

    // one extra small puff for each other color
    for( int i=0; i<3; i++ ) {
        doublePair pos = inE->position;
                                    
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
                                


        pos = inE->position;
                                    
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
    }




static void setPartLoudnessAndStereo( doublePair inSourcePos, 
                                      doublePair inPlayerPos,
                                      int inPartIndex,
                                      double inLoudnessFalloff,
                                      double inStereoSpread ) {
    
    double playerDist = distance( inPlayerPos, inSourcePos );
    if( playerDist < 4 ) {
        partLoudness[ inPartIndex ] = 1;
        }
    else {
        double playerDistModified = playerDist - 4;
        partLoudness[ inPartIndex ] = 
            inLoudnessFalloff / 
            ( inLoudnessFalloff + 
              playerDistModified * playerDistModified );
        }
    double vectorCosine = (inSourcePos.x - inPlayerPos.x) / playerDist;
    partStereo[ inPartIndex ] = 
        vectorCosine * inStereoSpread + 0.5;
    }






static char wasBeatHit = false;

void beatHit() {
    wasBeatHit = true;
    }




int Level::getStepsToRiseMarker( doublePair inPos ) {
    // loudness for closest rise markers
    doublePair closestRisePosition = mRiseWorldPos;
    
    GridPos closestRiseGrid = mRisePosition;

    if( mSymmetrical && 
        distance( mRiseWorldPos2, inPos ) < 
        distance( mRiseWorldPos, inPos ) ) {
        
        closestRisePosition = mRiseWorldPos2;
        closestRiseGrid = mRisePosition2;
        }

    int numStepsToRise;


    if( !wasBeatHit ) {

        // not on beat, or player shooting

        wasBeatHit = false;
        

        // just count steps, don't mark full path
        pathFind( getGridPos( inPos ), inPos, closestRiseGrid, 
                  0.01, &numStepsToRise  );
        
        
        }
    else {
        // beat just hit
        wasBeatHit = false;
        
        // mark full path

        GridPos *fullPath;



        pathFind( getGridPos( inPos ), inPos, closestRiseGrid, 
                  0.01, &numStepsToRise, &fullPath );

        if( fullPath != NULL ) {

            float baseFade = 1;
            
            if( numStepsToRise < 25 ) {
                
                if( numStepsToRise < 20 ) {
                    baseFade = 0;
                    }
                else {
                    baseFade = (numStepsToRise - 20) / 5.0f;
                    }
                }
            
            // only mark path if we are getting farther from last closest
            // point to rise marker, and only if substantially farther
            if( numStepsToRise <= mLastCloseStepsToRiseMarker + 20 ) {
                baseFade = 0;
                }
            
            if( baseFade > 0 ) {
                
                // clear any existing path
                mRiseMarkerPathSteps.deleteAll();

                
                int fullPathLength = numStepsToRise + 1;
            
                int fadeSteps = 4;
                
                int fadeSkip = 0;
            

                int fadeStart = fullPathLength - fadeSkip - fadeSteps;

                for( int i=0; i<fullPathLength; i++ ) {
                    
                    GridPos p = fullPath[i];
                
                    int squareIndex = mSquareIndices[ p.y ][ p.x ];
                    
                    float fade = 1;
                
                    if( i > fadeStart ) {
                    
                        if( i -  fadeStart < fadeSteps ) {
                            
                            fade = 1.0f - 
                                ( i - fadeStart ) / (float)fadeSteps;    
                            }
                        else {
                            fade = 0;
                            }
                        }
                
                
                    fade *= baseFade;


                    RiseMarkerPathStep step =
                        { squareIndex, fade };
                    
                    mRiseMarkerPathSteps.push_back( step );
                    }

                mRiseMarkerPathStepFadeProgress = 1;
                }
            // else don't add a path, because it would be invisible anyway

            delete [] fullPath;        
            }
        }
    
    

    return numStepsToRise;
    }



void Level::setLoudnessForAllParts() {
    
    // zero-out parts to account for destroyed enemies and picked-up power
    // tokens
    for( int p=0; p<PARTS-2; p++ ) {
        partLoudness[p] = 0;
        }
    double loudnessFalloffFactor = 40;
    double stereoSpread = 0.1;

    int i;
    
    // enemy loudness
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        
        setPartLoudnessAndStereo( e->position, 
                                  mPlayerPos,
                                  e->musicNotes.partIndex,
                                  loudnessFalloffFactor,
                                  stereoSpread );
        }
    
    
    // set loudness for tokens, too
    for( i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *p = mPowerUpTokens.getElement( i );
        
        setPartLoudnessAndStereo( p->position, 
                                  mPlayerPos,
                                  p->musicNotes.partIndex,
                                  loudnessFalloffFactor,
                                  stereoSpread );
        }


    

    // keep normal stereo computation
    // always keep beat centered
    partStereo[ PARTS-4 ] = 0.5;
    partStereo[ PARTS-3 ] = 0.5;
    

    
    // override loudness to make it linear instead
    // (always a straight build up from start position to rise marker)
    // build up hits max at distance 3 from rise marker
    double riseBeatLoudness = 1 - 
        ( ( (double)mLastComputedStepsToRiseMarker - 3 ) / 
          ( (double)mStartStepsToRiseMarker - 3 ) );

    
    if( riseBeatLoudness < 0 ) {
        riseBeatLoudness = 0;
        }
    else if( riseBeatLoudness > 1 ) {
        riseBeatLoudness = 1;
        }
    
    
    partLoudness[ PARTS-4 ] = riseBeatLoudness;
    partLoudness[ PARTS-3 ] = riseBeatLoudness;
    
    
    
    
    // further weight loudness based on edge fade, except for super-part
    // and player part
    
    
    // now weight them all, except rise marker parts, player part,
    // and harmony part
    for( int p=0; p<PARTS-4; p++ ) {
        partLoudness[p] *= mLastComputedEdgeFade;
        }

    }




void Level::step( doublePair inViewCenter, double inViewSize ) {
    
    // call rand at least once per step to ensure some mixing between
    // saves and restores when rising and falling (compacting and decompacting
    //  levels in stack)
    // Because some levels (negative ones) have nothing happening that uses
    //  the random number generator (unless player shoots).
    // Doing this causes random state to depend on exactly how many frame
    //  steps have happened.
    randSource.getRandomInt();
    

    mPlayerImmortalSteps --;
    if( mPlayerImmortalSteps < 0 ) {
        mPlayerImmortalSteps = 0;
        }
    
    mPlayerStepsUntilNextGlowTrail --;
    
    if( mPlayerStepsUntilNextGlowTrail <= 0 ) {
        
        doublePair trailPos = mPlayerPos;
        
        trailPos.x += randSource.getRandomBoundedDouble( -trailJitter, 
                                                         trailJitter );
        
        trailPos.y += randSource.getRandomBoundedDouble( -trailJitter, 
                                                         trailJitter );
        

        GlowSpriteTrail playerTrail = { trailPos, 1, 1, &mPlayerSprite };
        mGlowTrails.push_back( playerTrail );
    
        mPlayerStepsUntilNextGlowTrail = 
            (int)( stepsBetweenGlowTrails / frameRateFactor );
        }
    
    int i;

    // step token glow trails
    for( i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( i );

        t->stepsUntilNextGlowTrail --;
        
        if( t->stepsUntilNextGlowTrail <= 0 ) {
            
            doublePair trailPos = t->position;
        
            trailPos.x += randSource.getRandomBoundedDouble( -trailJitter, 
                                                             trailJitter );
            
            trailPos.y += randSource.getRandomBoundedDouble( -trailJitter, 
                                                             trailJitter );
            
            // token glow trails less strong, to make them easier to read
            GlowSpriteTrail trail = { trailPos, 0.5, 1, t->sprite };
        
            mGlowTrails.push_back( trail );

            t->stepsUntilNextGlowTrail = 
                (int)( stepsBetweenGlowTrails / frameRateFactor );
            }
        }
    

    
    // opt:  for certain steppable things,
    // don't step all of them, just visible portion
    
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

    
    // mignt be outside sGridWorldSpots, compute world-coord visual boundaries
    // from scratch
    doublePair visStart;
    doublePair visEnd;
    
    visStart.x = xVisStart - MAX_LEVEL_W/2;
    visStart.y = yVisStart - MAX_LEVEL_H/2;

    visEnd.x = xVisEnd - MAX_LEVEL_W/2;
    visEnd.y = yVisEnd - MAX_LEVEL_H/2;




    
    // step bullets
    
    // first, remove any that are on their final frame
    // they shouldn't take any more steps or do any more damage
    for( i=0; i<mBullets.size(); i++ ) {
        Bullet *b = mBullets.getElement( i );
        
        if( b->finalFrame ) {
            mBullets.deleteElement( i );
            i--;
            }
        }
    

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

                    // update heat seek waypoint... stick to the first enemy
                    // that we've started tracking, even if it moves,
                    // and then switch to the next closest enemy if the 
                    // currently-tracked one is destroyed.
                    b->heatSeekWaypoint = closestTarget;
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
        
        // opt:  don't create smoke clouds for
        // stuff happening off screen
        doublePair pos = b->position;
        char bulletOnScreen = false;
        if( pos.x >= visStart.x && 
            pos.y >= visStart.y &&
            pos.x <= visEnd.x && 
            pos.y <= visEnd.y ) {
            
            bulletOnScreen = true;
            }


        // first make sure it's in bounds of sparse world tiles
        
        if( mWallFlags[p.y][p.x] == 0 ) {
            // bullet has escaped out of bounds, kill it
            hit = true;
            }
        // for in-bound bullets, only consider those that are not past
        // the end of their life (or are exploding this step)
        else if( b->distanceLeft > 0 || b->explode > 0 ){
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

                    // do true intersection-point computation
                    // (otherwise, bullet smoke slides along wall in final 
                    //  frame and looks strange)
                    
                    doublePair desiredPos = add( oldBulletPos, b->velocity );
                    
                    if( ! equal( desiredPos, b->position ) ) {
                        
                        doublePair actualVelocity = 
                            sub( b->position, oldBulletPos );
                    
                        // find which velocity component was cut off more
                        double xVelocityFraction = 
                            actualVelocity.x / b->velocity.x;
                        double yVelocityFraction = 
                            actualVelocity.y / b->velocity.y;
                        
                        double minVelocityFraction = xVelocityFraction;
                        if( yVelocityFraction < minVelocityFraction ) {
                            minVelocityFraction = yVelocityFraction;
                            }

                        if( minVelocityFraction > 0 ) {
                            

                            doublePair finalVelocityStep = 
                                mult( b->velocity,
                                      minVelocityFraction );

                            // true intersection
                            b->position = add( oldBulletPos, 
                                               finalVelocityStep );
                            }
                        else {
                            // old position already beyond where
                            // stopMoveWithWall allows us to go
                       
                            // one more frame of old position
                            b->position = oldBulletPos;
                            }
                        
                        }
                    }
                else {
                    b->bouncesLeft --;
                    
                    // give bullet a small distance boost (20% of original)
                    // so that it can live out more of its bounces
                    b->distanceLeft += b->startDistance * 0.20;

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
            
                // did it hit a player or enemy?
                
                // ignore bullets that are half faded out (no effect)


                if( b->playerFlag && ! b->halfFadedOut ) {
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
                        
                            if( maxHealth < e->lastMaxHealth ) {
                                // max health has been lowered
                                if( e->health > maxHealth ) {
                                    e->health = maxHealth;
                                    }
                                }
                            else if( maxHealth > e->lastMaxHealth ) {
                                // max health has been raised
                                
                                // keep whatever health was missing
                                // from the old health bar

                                int oldMissingHealth =
                                    e->lastMaxHealth - e->health;
                            
                                e->health = maxHealth - oldMissingHealth;
                                }
                            
                            e->lastMaxHealth = maxHealth;

                            e->health --;
                            e->sprite->startSquint();
                            

                            tutorialEnemyHit();

                            if( e->health == 0 ) {
                                if( bulletOnScreen ) {
                                    
                                    // add hit smoke at enemy center
                                    generateEnemyDestructionSmoke( e );
                                    }
                                                                
                                // don't generate other hit smoke
                                destroyed = true;
                                
                                // clean up glow trails that use
                                // enemy sprite too
                                for( int t=0; t<mGlowTrails.size(); t++ ) {
                                    GlowSpriteTrail *trail = 
                                        mGlowTrails.getElement( t );
    
                                    if( trail->sprite == e->sprite ) {
                                        mGlowTrails.deleteElement( t );
                                        t--;
                                        }
                                    }
                                
                                // force-finish bullets that enemy fired
                                for( int b=0; b<mBullets.size(); b++ ) {
                                    Bullet *bullet =
                                        mBullets.getElement( b );
                                
                                    if( bullet->enemyMarker ==
                                        e->bulletMarker ) {

                                        // prevent future explosion
                                        bullet->explode = 0;
                                        
                                        if( bullet->distanceLeft > 1 ) {
                                            // curtail it's tradjectory
                                            bullet->distanceLeft = 1;
                                            }
                                        }
                                    }
                                


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
                else if( mPlayerImmortalSteps <= 0 && ! b->halfFadedOut ) {

                    // check if hit player
                    if( distance( mPlayerPos, b->position ) < hitRadius ) {
                        hit = true;
                        damage = true;
                        mPlayerHealth -= 1;
                        mPlayerSprite.startSquint();

                        mPlayerHealthBarJittering = true;
                        mPlayerHealthBarJitterProgress = 0;

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
            
            if( bulletOnScreen &&
                ( hit || b->explode > 0 ) ) {
                // draw smoke whenever bullet hits something or explodes

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
                    // start bigger if player knocked down
                    // (as feedback for player)
                    // this happens on freeze frame of zoom-in, so
                    // it is framerate independent
                    progress = 0.25;
                    }
                else if( b->explode > 0 ) {
                    // or for exploding bullets, so it looks more
                    // like smoke is "throwing" other bullets out
                    // this must take framerate into account
                    progress = 0.125 * frameRateFactor;
                    }

                HitSmoke s = { b->position, progress, 0.5, type, c };
                
                mSmokeClouds.push_back( s );


                if( type == 2 && mWallFlags[p.y][p.x] == 1 ) {
                    // in bounds, safe to look up
                    int squareIndex = mSquareIndices[p.y][p.x];

                    // leave blood on floor
                    char found = false;
                    BloodStain *stain = NULL;
                    for( int s=0; 
                         s<mBloodStains.size() && !found; s++ ) {
                    
                        stain = mBloodStains.getElement( s );
                        if( stain->floorIndex == squareIndex ) {
                            found = true;
                            }
                        }

                    if( !found ) {
                        // new one
                        BloodStain s = { squareIndex, 0 };
                        mBloodStains.push_back( s );
                                
                        stain = mBloodStains.getElement( 
                            mBloodStains.size() - 1 );
                        }
                            
                    stain->blendFactor += 0.05;
                    if( stain->blendFactor > 0.6 ) {
                        stain->blendFactor = 0.6;
                        }
                    }
                }
            
            if( b->explode > 0 ) {
                // make explosion happen
                
                Bullet explosionTemplate = *b;
                
                // reset bounces and distance
                explosionTemplate.distanceLeft = b->startDistance;
                explosionTemplate.bouncesLeft = b->startBounces;
                
                // no sub-explosions
                explosionTemplate.explode = 0;
                
                // keep bullet velocity unchanged


                // angle between bullets shrinks as explode parameter grows
                double angleBetweenBullets = 2 * M_PI / ( b->explode + 1 );
                
                // minimum 2 sub-bullets
                // keep adding bullets until full angle spread is at least
                // a half-circle (when angle is small, more bullets are
                // needed to fill a half-circle)
                int numSubBullets = 2;
                
                double totalAngle = angleBetweenBullets;
                
                while( totalAngle < M_PI ) {
                    numSubBullets++;
                    totalAngle += angleBetweenBullets;
                    }
                
                
                double startAngle =  - totalAngle / 2;
                
                for( int s=0; s<numSubBullets; s++ ) {
                    Bullet subBullet = explosionTemplate;
                    subBullet.velocity = rotate( subBullet.velocity,
                                                 startAngle + 
                                                 s * angleBetweenBullets );
                    
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
            
            if( damage || b->explode > 0 ) {
                // draw one more frame of this bullet, THEN delete it
                // immediately on next step (so we can see the bullet
                // that hit whatever it hit)
                b->finalFrame = true;
                }
            else {
                // bullet done now
                mBullets.deleteElement( i );
                i--;
                }
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
    char enemyPathFindingDoneThisStep = false;
    if( mNextEnemyPathFindIndex > mEnemies.size() - 1 ) {
        mNextEnemyPathFindIndex = 0;
        }
    


    
    


    // step enemies
    for( i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );

        e->stepsUntilNextGlowTrail --;
        
        if( e->stepsUntilNextGlowTrail <= 0 ) {
            
            doublePair trailPos = e->position;
        
            trailPos.x += randSource.getRandomBoundedDouble( -trailJitter, 
                                                             trailJitter );
            
            trailPos.y += randSource.getRandomBoundedDouble( -trailJitter, 
                                                             trailJitter );
            
            GlowSpriteTrail trail = { trailPos, 1, 1, e->sprite };
        
            mGlowTrails.push_back( trail );

            e->stepsUntilNextGlowTrail = 
                (int)( stepsBetweenGlowTrails / frameRateFactor );
            }
        

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
                    enemyPathFindingDoneThisStep = true;
                    
                    e->followNextWaypoint = 
                        sGridWorldSpots[ targetGridPos.y ]
                        [ targetGridPos.x ];
                    }
                }


            // if we're less that one step away from goal, limit move speed
            // so we don't bounce back and forth around goal

            doublePair followVelocity = sub( e->followNextWaypoint, 
                                             e->position );
            
            if( distance( e->followNextWaypoint, e->position ) > moveSpeed ) {
                // far enough away to move with normal speed
                followVelocity = 
                    mult( normalize( followVelocity ), moveSpeed );
                }
            

            // weighted sum with old velocity to smooth out movement

            // good even on last step to goal (adds a little, soft bounce to
            //  enemy stop)
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


            // perfect aim using quadratic equations
            //
            // similar to what is described here:
            // http://playtechs.blogspot.com/2007/04/
            //           aiming-at-moving-target.html

            doublePair relativePlayerPos = sub( mPlayerPos, e->position );

            double xt = relativePlayerPos.x;
            double yt = relativePlayerPos.y;
            
            double xv = mPlayerVelocity.x;
            double yv = mPlayerVelocity.y;
            
            double bv = bulletSpeed;
            
            // quadratic terms
            double A = xv * xv + yv * yv - bv * bv;
            double B = 2 * ( xt * xv + yt * yv );
            double C = xt * xt + yt * yt;
            
            double D = B * B - 4 * A * C;
            
            double hitTime = 0;
            
            if( D >= 0 ) {
                
                double sqrtD = sqrt( D );

                double t1 = ( -B + sqrtD ) / ( 2 * A );
                double t2 = ( -B - sqrtD ) / ( 2 * A );
                
                // pick smallest positive hit time
                if( t1 > 0 && t2 > 0 ) {
                    if( t1 < t2 ) {
                        hitTime = t1;
                        }
                    else {
                        hitTime = t2;
                        }
                    }
                else if( t1 > 0 ) {
                    hitTime = t1;
                    }
                else if( t2 > 0 ) {
                    hitTime = t2;
                    }
                }
            
            double bulletDistance = getBulletDistance( e->powers );

            if( bulletSpeed * hitTime > bulletDistance ) {
                // bullet will die before it can reach target
                hitTime = 0;
                }
            

            
            doublePair aimPos = mPlayerPos;

            if( hitTime > 0 ) {
                aimPos = add( mPlayerPos,
                              mult( mPlayerVelocity, hitTime ) );
                }            


            addBullet( e->position, aimPos, 
                       e->powers,
                       mPlayerPos,
                       bulletSpeed, false, e->bulletMarker );
            

            //e->stepsTilNextBullet = e->stepsBetweenBullets;
            e->stepsTilNextBullet = getStepsBetweenBullets( e->powers );
            }
        else {
            e->stepsTilNextBullet --;
            }
        
        if( e->healthBarFade > 0 ) {
            e->healthBarFade -= 0.015 * frameRateFactor;
            if( e->healthBarFade < 0 ) {
                e->healthBarFade = 0;
                }
            }

        doublePair lookDir = normalize( sub( mPlayerPos, e->position ) );
        
        e->sprite->setLookVector( lookDir );
        }

    
    // only one path finding operation per timestep
    if( ! enemyPathFindingDoneThisStep ) {
        
        int oldSteps = mLastComputedStepsToRiseMarker;

        // path find to rise marker outside audio lock, too
        mLastComputedStepsToRiseMarker = getStepsToRiseMarker( mPlayerPos );
        

        if( mGettingFartherAwayFromRiseMarker &&
            oldSteps > mLastComputedStepsToRiseMarker ) {
            
            // was getting farther away, but just turned around
            
            // reset our closest step cound
            mLastCloseStepsToRiseMarker = mLastComputedStepsToRiseMarker;

            mGettingFartherAwayFromRiseMarker = false;
            }
        
        

        if( mLastComputedStepsToRiseMarker < mLastCloseStepsToRiseMarker ) {
            mLastCloseStepsToRiseMarker = mLastComputedStepsToRiseMarker;
            mGettingFartherAwayFromRiseMarker = false;
            }
        else if( mLastComputedStepsToRiseMarker > 
                 mLastCloseStepsToRiseMarker ) {
            mGettingFartherAwayFromRiseMarker = true;
            }
        
        }
    



    // don't roll this into enemy behavior loop.
    // don't want to block audio thread during enemy behavior calculations
    lockAudio();

    setLoudnessForAllParts();

    unlockAudio();

    
    // player look
    doublePair lookDir = normalize( sub( mMousePos, mPlayerPos ) );
    mPlayerSprite.setLookVector( lookDir );
    

    // step square colors

    float dampingFactor = 0.025 * frameRateFactor;
    float totalWeight = 1 + dampingFactor;

    // opt:  only step visible ones
    for( int y=yVisStart; y<=yVisEnd; y++ ) {
        for( int x=xVisStart; x<=xVisEnd; x++ ) {
            if( mWallFlags[y][x] > 0 ) {
                int i = mSquareIndices[y][x];
                
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
        
        
                mGridColors[i].r /= totalWeight;
                mGridColors[i].g /= totalWeight;
                mGridColors[i].b /= totalWeight;        
                }
            }
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
    mPlayerSprite.draw( mPlayerPos, inFade );
    }



void Level::drawSmoke( double inFade, 
                       doublePair inVisStart, doublePair inVisEnd ) {
    // draw smoke
    for( int i=0; i<mSmokeClouds.size(); i++ ) {
        
        HitSmoke *s = mSmokeClouds.getElement( i );

        doublePair pos = s->position;
        
        if( pos.x >= inVisStart.x && pos.y >= inVisStart.y &&
            pos.x <= inVisEnd.x && pos.y <= inVisEnd.y ) {
        
            float fade = inFade * ( 0.5 - 0.5 * s->progress );
        
            switch( s->type ) {
                case 0:
                    setDrawColor( 0, 0, 0, fade );
                    break;
                case 1:
                    setDrawColor( 1, 1, 1, fade );
                    break;
                case 2:
                    setDrawColor( 0.85, 0, 0, fade * 2 );
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
    

    }



void Level::drawGlowTrails( double inFade, 
                            doublePair inVisStart, doublePair inVisEnd ) {

    toggleAdditiveBlend( true );
    toggleLinearMagFilter( true );
    for( int t=0; t<mGlowTrails.size(); t++ ) {
        GlowSpriteTrail *trail = mGlowTrails.getElement( t );
        
        doublePair pos = trail->position;
        
        if( pos.x >= inVisStart.x && pos.y >= inVisStart.y &&
            pos.x <= inVisEnd.x && pos.y <= inVisEnd.y ) {
            
            // remap [1..0] to [0..1], where full fade is at 1
            float mappedFade = 1 - trail->progress;

            if( mappedFade < 0.25 ) {
                // hits sine peak at 0.25
                mappedFade = sin( mappedFade * M_PI * 2 );
                }
            else {
                // decays linearly down from peak
                mappedFade = 1 - (mappedFade - 0.25 ) / 0.75;
                }
            

            trail->sprite->draw( trail->position, 
                                 mappedFade * 0.1 * inFade * trail->fade );
            }
        }
    toggleLinearMagFilter( false );
    toggleAdditiveBlend( false );
    }



void Level::drawEnemies( double inFade, int inLayer, 
                         doublePair inVisStart, doublePair inVisEnd ) {
    if( mLastComputedEdgeFade <= 0 ) {
        return;
        }
    
    int startIndex = 0;
    int endIndex = mEnemies.size();
    
    if( inLayer == 0 ) {
        if( mWindowSet ) {
            
            if( mWindowPosition.type == enemy ) {
                endIndex = mWindowPosition.index;
                }
            else if( mWindowPosition.type == power ) {
                
                // all enemies drawn ABOVE power ups, so none to draw
                // on layer 0 if a power up is our item window
                return;
                }
            }
        }
    else if( inLayer == 1 ) {
        // window is ALWAYS set if we're drawing layer 1
        if( mWindowSet ) {
            if( mWindowPosition.type == enemy ) {
                startIndex = mWindowPosition.index + 1;
                }
            else if( mWindowPosition.type == power ) {
                // draw all on layer 1, since all drawn above power-ups
                }
            }
        }
    

    double fade = inFade * mLastComputedEdgeFade;
        

    for( int i=startIndex; i<endIndex; i++ ) {
        Enemy *e = mEnemies.getElement( i );

        doublePair pos = e->position;
        
        if( pos.x >= inVisStart.x && pos.y >= inVisStart.y &&
            pos.x <= inVisEnd.x && pos.y <= inVisEnd.y ) {

            e->sprite->draw( pos, fade );
        
            if( e->healthBarFade > 0 ) {
                // hold at full vis until half-way through fade
                float fade = 1;

                if( e->healthBarFade < 0.5 ) {
                    // from held at 1 to sin fade out
                    fade = sin( e->healthBarFade * M_PI );
                    }
            
                int maxHealth = getEnemyMaxHealth( e->powers );
                

                double segmentWidth = 0.25;

                char drawSegments = false;
                
                double redBarWidth;

                
                if( maxHealth <= 3 ) {
                    redBarWidth = segmentWidth * maxHealth;
                    drawSegments = true;
                    }
                else if( maxHealth <= 4 ) {
                    // same total width as length 3 bar, as transition
                    segmentWidth = 0.1875;
                    
                    redBarWidth = segmentWidth * maxHealth;
                    
                    drawSegments = true;
                    }
                else if( maxHealth <= 8 ) {
                    segmentWidth = 0.125;
                    
                    redBarWidth = segmentWidth * maxHealth;
                    
                    drawSegments = true;
                    }
                else {
                    redBarWidth = segmentWidth * 4;
                    }

                double halfRedBarWidth = redBarWidth / 2;

                double borderWidth = 1.0 / 16.0;
                
                double barBGHalfWidth = halfRedBarWidth + borderWidth;

                // outline
                setDrawColor( 0.25, 0.25, 0.25, 0.75 * fade );
                drawRect( pos.x - barBGHalfWidth, pos.y + 0.5, 
                          pos.x + barBGHalfWidth, pos.y + 0.25 );
            
                float healthFraction = e->health / 
                    (float)getEnemyMaxHealth( e->powers );

                if( !drawSegments ) {
                    // round to nearest sub-pixel
                
                    healthFraction *= 32;
                    
                    healthFraction = roundf( healthFraction );
                    
                    healthFraction /= 32;
                    }
                
            
                // black behind empty part
                setDrawColor( 0, 0, 0, fade );
                drawRect( pos.x - halfRedBarWidth + 
                              redBarWidth * healthFraction, 
                          pos.y + 0.4375, 
                          pos.x + halfRedBarWidth, 
                          pos.y + 0.3125 );
            
    
                if( !drawSegments ) {
                    // solid red bar
                    setDrawColor( 0.85, 0, 0, fade );
                    drawRect( pos.x - halfRedBarWidth, 
                              pos.y + 0.4375,
                              pos.x - halfRedBarWidth + 
                                 redBarWidth * healthFraction, 
                              pos.y + 0.3125 );
                    }
                else {
                    int numSegments = e->health;
        
                    char swapColor = true;
        
                    for( int i=0; i<numSegments; i++ ) {
            
                        if( swapColor ) {
                            setDrawColor( 0.85, 0, 0, fade );
                            }
                        else {
                            setDrawColor( 0.65, 0, 0, fade );
                            }
                        swapColor = !swapColor;
            
                        drawRect( pos.x - halfRedBarWidth + i * segmentWidth, 
                                  pos.y + 0.4375,
                                  pos.x - halfRedBarWidth + 
                                    (i+1) * segmentWidth, 
                                  pos.y + 0.3125 );
                        }
                    }
                }
            }
        }
    }




static void computeVisBoundaries( doublePair inViewCenter, double inViewSize,
                                  doublePair *outVisStart, 
                                  doublePair *outVisEnd,
                                  GridPos *outVisStartGrid,
                                  GridPos *outVisEndGrid ) {
    
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

    
    outVisStartGrid->x = xVisStart;
    outVisStartGrid->y = yVisStart;

    outVisEndGrid->x = xVisEnd;
    outVisEndGrid->y = yVisEnd;
    

    // mignt be outside sGridWorldSpots, compute world-coord visual boundaries
    // from scratch
    
    outVisStart->x = xVisStart - MAX_LEVEL_W/2;
    outVisStart->y = yVisStart - MAX_LEVEL_H/2;

    outVisEnd->x = xVisEnd - MAX_LEVEL_W/2;
    outVisEnd->y = yVisEnd - MAX_LEVEL_H/2;
    }

    




        
void Level::drawLevel( doublePair inViewCenter, double inViewSize ) {
    
    if( !mFrozen ) {
        step( inViewCenter, inViewSize );
        }
    else {
        // frozen, but keep any token that was sub-level entry point updated
        
        if( mLastEnterPointPowerTokenIndex != -1 ) {
            PowerUpToken *t = mPowerUpTokens.getElement( 
                mLastEnterPointPowerTokenIndex );
            

            t->power.powerType = t->subPowers->getMajorityType();
            t->power.level = t->subPowers->getLevelSum( t->power.powerType );
            }
        }
    

    // step glow trails, even when frozen (so they fade out during a zoom
    // and don't cause things to slow down when they fill the screen at 
    // the end of a zoom)
    for( int t=0; t<mGlowTrails.size(); t++ ) {
        GlowSpriteTrail *trail = mGlowTrails.getElement( t );
    
        trail->progress -= 0.025 * frameRateFactor;
    
        if( trail->progress <= 0 ) {
            mGlowTrails.deleteElement( t );
            t--;
            }
        }

    // step health bar jitter even when frozen
    if( mPlayerHealthBarJittering ) {
        mPlayerHealthBarJitterProgress += 0.0125 * frameRateFactor;
        if( mPlayerHealthBarJitterProgress > 1 ) {
            mPlayerHealthBarJittering = false;
            }
        }
    
    // step rise path steps, even when frozen
    if( mRiseMarkerPathSteps.size() > 0 ) {
        
        mRiseMarkerPathStepFadeProgress -= 0.025 * frameRateFactor;
        
        if( mRiseMarkerPathStepFadeProgress < 0 ) {
            mRiseMarkerPathStepFadeProgress = 0;
            
            mRiseMarkerPathSteps.deleteAll();
            }
        }    



        
    int i;




    // opt:  don't draw whole grid, just visible part
    doublePair visStart, visEnd;
    GridPos visStartGrid, visEndGrid;
    
    computeVisBoundaries( inViewCenter, inViewSize, 
                          &visStart, &visEnd,
                          &visStartGrid, &visEndGrid );
    
    



    double edgeFade = 0;
    
    
    if( mDrawFloorEdges ) {
        
        double maxDistance = 5.5;
        
        double dist = distance( mPlayerPos, mRiseWorldPos );
        
        double dist2 = DBL_MAX;
        if( mDoubleRisePositions ) {
            dist2 = distance( mPlayerPos, mRiseWorldPos2 );
            }

        if( mEdgeFadeIn >= 1 && dist > maxDistance && dist2 > maxDistance ) {
            edgeFade = 1;
            }
        else {
            
            double fade = mEdgeFadeIn;
            
            if( dist <= maxDistance || dist2 <= maxDistance ) {
                double smallestDist = dist;
                if( dist2 < smallestDist ) {
                    smallestDist = dist2;
                    }
                
                if( smallestDist > 1 ) {
                    fade = (smallestDist - 1.5) / (maxDistance - 1.5);

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
            
            edgeFade = fade;

            if( mEdgeFadeIn < 1 ) {    
                mEdgeFadeIn += 0.01 * frameRateFactor;
                }
            }
        }
    
    mLastComputedEdgeFade = edgeFade;
    


    // draw walls
    if( edgeFade > 0 )
    for( int y=visStartGrid.y; y<=visEndGrid.y; y++ ) {
        for( int x=visStartGrid.x; x<=visEndGrid.x; x++ ) {
            if( mWallFlags[y][x] == 2 ) {
                Color *c = &( mGridColors[mSquareIndices[y][x]] );
                
                setDrawColor( c->r,
                              c->g,
                              c->b, c->a );
            
                drawSquare( sGridWorldSpots[y][x], 0.5 );
                }
            }
        }
    
    doublePair fullMapPos = { -.5, 0.5 };

    if( edgeFade > 0 ) {
        
        // soft glow over walls only
        setDrawColor( 1, 1, 1, 0.50 );
        //toggleAdditiveBlend( true );
        toggleLinearMagFilter( true );
        drawSprite( mFullMapSprite, fullMapPos, 1.0 );
        toggleLinearMagFilter( false );
        //toggleAdditiveBlend( false );
        }



    if( edgeFade > 0 ) {
        // draw edges at full opacity
        // we draw bitmap below over top to cause edges to fade in

        Color c = mColors.primary.elements[3];
        setDrawColor( c.r,
                      c.g,
                      c.b, 1 );
        
        for( int y=visStartGrid.y; y<=visEndGrid.y; y++ ) {
            for( int x=visStartGrid.x; x<=visEndGrid.x; x++ ) {
                if( mWallFlags[y][x] == 1 &&
                    mFloorEdgeFlags[mSquareIndices[y][x]] != 0 ) {
                    
                    drawSquare( sGridWorldSpots[y][x], 0.5625 );
                    }
                }
            }
        }
    
    


    // draw floor
    if( edgeFade > 0 ) {

        float shadowLevel = 0.75;
        
        if( mWindowSet ) {
            // don't shadows toward end of zoom
            // too many pixels to fill
            shadowLevel *= mLastComputedFastWindowFade;
            }


                
        if( shadowLevel > 0 ) {
            // draw shadows only on top of floor, not walls
            startAddingToStencil( true, true );
            }
        
        for( int y=visStartGrid.y; y<=visEndGrid.y; y++ ) {
            for( int x=visStartGrid.x; x<=visEndGrid.x; x++ ) {
                if( mWallFlags[y][x] == 1 ) {
                    Color *c = &( mGridColors[mSquareIndices[y][x]] );
                    
                    setDrawColor( c->r,
                                  c->g,
                                  c->b, 1 );
                    
                    drawSquare( sGridWorldSpots[y][x], 0.5 );
                    }
                }
            }

        setDrawColor( 1, 1, 1, 1 );
        
        if( false )drawRect( visStart.x, visStart.y, 
                             visEnd.x, visEnd.y );

        
        // draw rise marker *under* shadows
        Color *c = &( mColors.special );
        setDrawColor( c->r,
                      c->g,
                      c->b, 1 );
        drawSprite( riseMarker, mRiseWorldPos );
        
        if( mDoubleRisePositions ) {
            drawSprite( riseMarker, mRiseWorldPos2 );
            }
        
        // draw blood stains under shadows too
        
        // blood stains
        for( int s=0; s<mBloodStains.size(); s++ ) {
            BloodStain *stain = mBloodStains.getElement( s );
            setDrawColor( 0.80, 0, 0, stain->blendFactor );
            
            drawSquare( *( mGridWorldSpots[ stain->floorIndex ] ), 0.5 );
            }


        
        // rise marker path steps under shadow, same color as rise marker
        setDrawColor( c->r,
                      c->g,
                      c->b, 1 );

        float pathFade = 0.75 * mRiseMarkerPathStepFadeProgress;
        
        for( int s=0; s<mRiseMarkerPathSteps.size(); s++ ) {
            RiseMarkerPathStep *step = mRiseMarkerPathSteps.getElement( s );
            
            GridPos pos = mIndexToGridMap[ step->floorIndex ];
            
            if( pos.y >= visStartGrid.y && pos.y <= visEndGrid.y 
                &&
                pos.x >= visStartGrid.x && pos.x <= visEndGrid.x ) {
        
                if( step->blendFactor > 0 ) {

                    setDrawFade( step->blendFactor * pathFade );
            
                    drawSquare( *( mGridWorldSpots[ step->floorIndex ] ), 
                                0.5 );
                    }
                }
            }
        
        
        
        




        
        if( shadowLevel > 0 ) {
            
            startDrawingThroughStencil();
        
            // wall shadows on floor
            setDrawColor( 1, 1, 1, shadowLevel );
            //toggleAdditiveBlend( true );
            toggleLinearMagFilter( true );
            drawSprite( mFullMapWallShadowSprite, fullMapPos, 
                        1.0 / shadowBlowUpFactor );
        

            // bullet shadows under walls too

            // skip shadows if too many are already drawn over a given
            // grid square (so too much darkness doesn't accumulate)
            // init all to zero
            char shadowHitCounts[ MAX_LEVEL_H ][ MAX_LEVEL_W ] = { {0} };
                        
            
            for( i=0; i<mBullets.size(); i++ ) {
                    
                Bullet *b = mBullets.getElement( i );
                
                doublePair pos = b->position;
                
                if( pos.x >= visStart.x && pos.y >= visStart.y &&
                    pos.x <= visEnd.x && pos.y <= visEnd.y ) {
                    
                    GridPos gridPos = getGridPos( pos );
                    
                    if( shadowHitCounts[ gridPos.y ][ gridPos.x ] < 2 ) {
                        
                        shadowHitCounts[ gridPos.y ][ gridPos.x ]++;
                        

                        float fade = 1;
                    
                        if( b->explode == 0 && b->distanceLeft < 1 ) {
                            fade = b->distanceLeft;
                            }
                    
                        setDrawColor( 1, 1, 1, fade * shadowLevel );
                        
                        drawBulletShadow( b->size, b->position );
                        }
                    
                    }
                }                             

        
            // player shadow cut off by walls, too
            mPlayerSprite.drawShadow( mPlayerPos, shadowLevel );


            // same with crosshair shadow, but only if we're not
            // entering a power-up, so we don't draw it twice
            setDrawColor( 1, 1, 1, shadowLevel );
            drawCrosshairShadow( mEnteringMouse, mMousePos );
                

            // same with enemy shadows
            for( int i=0; i<mEnemies.size(); i++ ) {
                Enemy *e = mEnemies.getElement( i );
                
                doublePair pos = e->position;
                
                if( pos.x >= visStart.x && pos.y >= visStart.y &&
                    pos.x <= visEnd.x && pos.y <= visEnd.y ) {
                    
                    e->sprite->drawShadow( pos, shadowLevel );
                    }
                }
            
            // same with power-up shadows
            setDrawColor( 1, 1, 1, shadowLevel );
            for( int i=0; i<mPowerUpTokens.size(); i++ ) {
                PowerUpToken *p = mPowerUpTokens.getElement( i );
                
                doublePair pos = p->position;
                
                if( pos.x >= visStart.x && pos.y >= visStart.y &&
                    pos.x <= visEnd.x && pos.y <= visEnd.y ) {
                    
                    drawPowerUpShadow( p->position );
                    }
                }


            toggleLinearMagFilter( false );
            
            stopStencil();
            }
        
        

        }


    // draw power-ups
    for( i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *p = mPowerUpTokens.getElement( i );
        
        doublePair pos = p->position;
        
        if( pos.x >= visStart.x && pos.y >= visStart.y &&
            pos.x <= visEnd.x && pos.y <= visEnd.y ) {
            
            drawPowerUp( p->power, p->position, 1.0 );
            }
        }
    


    if( edgeFade < 1 ) {
        // fade this in over top as edges fade in
        
        setDrawColor( 1, 1, 1, 1 - edgeFade );
        
        drawSprite( mFullMapSprite, fullMapPos, 1.0 );
        }

    
    
    

    
    


    


    
    
    

    // draw bullets (even if in blur mode)
    for( i=0; i<mBullets.size(); i++ ) {
        
        Bullet *b = mBullets.getElement( i );

        doublePair pos = b->position;
        
        if( pos.x >= visStart.x && pos.y >= visStart.y &&
            pos.x <= visEnd.x && pos.y <= visEnd.y ) {
        
            float fade = 1;
        
            if( b->explode == 0 && b->distanceLeft < 1 ) {
                fade = b->distanceLeft;

                if( fade <= 0.5 ) {
                    b->halfFadedOut = true;
                    }
                }

            
            if( b->playerFlag ) {
                setDrawColor( 1, 1, 1, fade );
                }
            else {
                setDrawColor( 0.35, 0.35, 0.35, fade );
                }
            drawBullet( b->size, b->position, fade );
            }
        
        }




    // draw layer of enemies under any item window
    // (draws all enemies if no window set, or if window
    //   doesn't require enemies to be drawn on top [player window])
    drawEnemies( 1.0, 0, visStart, visEnd );
    


    


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

        if( edgeFade >  0 ) {
            drawGlowTrails( edgeFade, visStart, visEnd );
            }


        drawSmoke( 1, visStart, visEnd );        
        }
    



    if( mWindowSet ) {
        startDrawingThroughStencil();
        }    

    }



void Level::drawWindowShade( double inFade, double inFrameFade,
                             doublePair inViewCenter, double inViewSize ) {
    if( mWindowSet ) {
        
        doublePair visStart, visEnd;
        GridPos visStartGrid, visEndGrid;
        
        computeVisBoundaries( inViewCenter, inViewSize, 
                              &visStart, &visEnd,
                              &visStartGrid, &visEndGrid );


        stopStencil();
        
        double overlieFade = (inFade - 0.63) / 0.37;

        // for stuff that slows down zoom progress if drawn too big
        mLastComputedFastWindowFade = (inFade - 0.85) / 0.15;

        if( mWindowPosition.type == player ) {
            mPlayerSprite.drawBorder( mPlayerPos, inFrameFade );
            mPlayerSprite.drawCenter( mPlayerPos, inFade );
            
            // mouse drawn under player
            }
        else {
            if( mWindowPosition.type == enemy ) {    
                Enemy *e = mEnemies.getElement( mWindowPosition.index );
                e->sprite->drawBorder( e->position, inFrameFade );
                e->sprite->drawCenter( e->position, inFade );
                }
            else if( mWindowPosition.type == power ) {    
                PowerUpToken *t = 
                    mPowerUpTokens.getElement( mWindowPosition.index );
                
                drawPowerUpBorder( t->position, inFrameFade );
                drawPowerUpCenter( t->power, t->position, inFade );
                }
            

            // second enemy layer drawn on top of enemy or power-up window
            // mouse and player drawn on top of enemy or power-up
            // fade these a bit sooner to get them out of the way
            if( overlieFade > 0 ) {
                drawEnemies( overlieFade, 1, visStart, visEnd );
                drawMouse( overlieFade );
                drawPlayer( overlieFade );
                }
            }

        // glow trails drawn on top
        if( mLastComputedFastWindowFade > 0 &&  mLastComputedEdgeFade >  0 ) {
            drawGlowTrails( 
                mLastComputedFastWindowFade * mLastComputedEdgeFade, 
                visStart, visEnd );
            }


        // smoke drawn on top of all
        drawSmoke( overlieFade, visStart, visEnd );
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
    GridPos p = getGridPos( inPos );
    
    // include completely out-of-bounds areas as wall, too 
    return ( mWallFlags[p.y][p.x] != 1 );
    }


char Level::isRiseSpot( doublePair inPos ) {

    if( distance( mRiseWorldPos, inPos ) < 1 ) {
        return true;
        }
    else if( mDoubleRisePositions ) {
        if( distance( mRiseWorldPos2, inPos ) < 1 ) {
            return true;
            }
        }
    return false;


    int x = (int)( rint( inPos.x ) );
    int y = (int)( rint( inPos.y ) );
    
    int x2 = (int)( rint( -inPos.x - 1 ) );

    x += MAX_LEVEL_W/2;
    y += MAX_LEVEL_H/2;
    x2 += MAX_LEVEL_W/2;
    
    // no need to check if in-bounds, since we're not indexing with x and y
        
    if( !mDoubleRisePositions ) {
        return ( mRisePosition.x == x && mRisePosition.y == y );
        }
    else {
        return ( ( mRisePosition.x == x || mRisePosition.x == x2 )
                 && mRisePosition.y == y );
        }
    }



doublePair Level::getEnemyCenter( int inEnemyIndex ) {
    Enemy *e = mEnemies.getElement( inEnemyIndex );
    return e->position;
    }


doublePair Level::getPowerUpCenter( int inPowerUpIndex ) {
    PowerUpToken *t = mPowerUpTokens.getElement( inPowerUpIndex );
    return t->position;
    }



int Level::getEnemyDifficultyLevel( int inEnemyIndex ) {
    Enemy *e = mEnemies.getElement( inEnemyIndex );
    return e->difficultyLevel;
    }




char Level::isEnemy( doublePair inPos, int *outEnemyIndex ) {
    
    // find closest enemy to inPos, since enemies might overlap
    // (unlike power-ups)

    // return none if all are further away than this
    double closestDistance = 0.5;
    int closestIndex = -1;

    for( int j=0; j<mEnemies.size(); j++ ) {
        Enemy *e = mEnemies.getElement( j );
        
        double thisDist = distance( e->position, inPos );
        if( thisDist < closestDistance ) {
            closestDistance = thisDist;
            closestIndex = j;
            }
        }

    if( closestIndex != -1 ) {
        if( outEnemyIndex != NULL ) {
            *outEnemyIndex = closestIndex;
            }
        return true;
        }

    return false;
    }



char Level::isPlayer( doublePair inPos ) {
    return ( distance( mPlayerPos, inPos ) < 0.5 );
    }



static double widePickUpRadius = 0.85;


char Level::isPowerUp( doublePair inPos, int *outPowerUpIndex,
                       char inWidePickup ) {

    double radius = 0.5;
    if( inWidePickup ) {
        radius = widePickUpRadius;
        }
    

    for( int j=0; j<mPowerUpTokens.size(); j++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( j );
        
        if( distance( t->position, inPos ) < radius ) {
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
        
        if( distance( t->position, inPos ) < widePickUpRadius ) {
            
            PowerUp p = t->power;

            // clean up glow trails that use
            // token sprite too
            for( int i=0; i<mGlowTrails.size(); i++ ) {
                GlowSpriteTrail *trail = 
                    mGlowTrails.getElement( i );
                
                if( trail->sprite == t->sprite ) {
                    mGlowTrails.deleteElement( i );
                    i--;
                    }
                }


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


NoteSequence Level::getLevelNoteSequence() {
    return mHarmonyNotes;
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

                /*
                if( t->startedEmpty &&
                    t->power.powerType != powerUpEmpty ) {
                    
                    // type has been changed by some sub-level activity
                
                    // fix the type away from empty now
                    t->startedEmpty = false;
                    t->sprite->mStartedEmpty = false;
                    }
                */

                // entering a power-up to modify it.
                // keep its sprite updated
                t->sprite->mKeepUpdated = true;

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




NoteSequence Level::getEnteringPointNoteSequence( doublePair inPosition,
                                                  itemType inType ) {
    switch( inType ) {
        case player: {

            return mPlayerMusicNotes;
            }
            break;
        case enemy: {
            int i;
    
            if( isEnemy( inPosition, &i ) ) {
                Enemy *e = mEnemies.getElement( i );
                
                return e->musicNotes;
                }
            }
            break;
        case power: {
            int i;
    
            if( isPowerUp( inPosition, &i ) ) {
                PowerUpToken *t = mPowerUpTokens.getElement( i );
                
                return t->musicNotes;
                }
            }
            break;            
        }
    
    // default
    return generateRandomNoteSequence( 0 );
    }



int Level::getEnteringPointSubLevel( doublePair inPosition,
                                     itemType inType ) {
    
    // new method:  just decrement level no matter what we're entering
    
    // thus, there's effectively no limit on exponential player power growth
    // (though achieving huge player powers is still impractical)
    return mLevelNumber - 1;
    

    
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

                // if enemy has powers at all, sub level has to be at least
                // 3 so that those powers can be replaced (since levels < 3
                // have no powers on floor)

                // do this if enemy currently doesn't even have powers,
                // since we want to see the sense of entering level 0 enemies
                // that live on floor level 4.

                if( powerSum < 3 ) {
                    powerSum = 3;
                    }

                // ensure that level number always decrements
                if( powerSum <= mLevelNumber - 1 ) {
                    return powerSum;
                    }
                else {
                    return mLevelNumber - 1;
                    }
                }
            }
            break;
        case power: {
            int i;
    
            if( isPowerUp( inPosition, &i ) ) {
                int returnValue = mLevelNumber / POWER_SET_SIZE;
                
                // bottom out at level where there are power ups on floor
                // (unless we are descending from that bottom)
                // Thus, from floor level 4 upward, power ups can be
                // raised to a minimum of level 4 by entering them and
                // collecting three level 1 power ups of the same type.

                // This makes floor level 4 a good training spot for
                // entering all types of things.
                if( returnValue < 3 ) {
                    returnValue = 3;
                    }
                

                // ensure that level number always decrements
                if( returnValue <= mLevelNumber - 1 ) {
                    return returnValue;
                    }
                else {
                    return mLevelNumber - 1;
                    }
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



NoteSequence *Level::getPlayerNoteSequence() {
    return &mPlayerMusicNotes;
    }



void Level::setPlayerPowers( PowerUpSet *inPowers ) {
    mPlayerPowers->copySet( inPowers );
    }



void Level::getPlayerHealth( int *outValue, int *outMax ) {
    int max = 1 + getMaxHealth( mPlayerPowers );
    *outMax = max;
    
    // truncate player health, incase it was restored before a power-up
    // pickup that reduced max health
    if( mPlayerHealth > max ) {
        mPlayerHealth = max;
        }
    
    *outValue = mPlayerHealth;
    }



void Level::restorePlayerHealth() {
    int v, m;
    getPlayerHealth( &v, &m );
    mPlayerHealth = m;
    }



doublePair Level::getPlayerHealthBarJitter() {
    doublePair jitter = { 0, 0 };
    
    if( mPlayerHealthBarJittering ) {
        
        jitter.y = 
            0.375 *
            (1 - mPlayerHealthBarJitterProgress) *
            sin( mPlayerHealthBarJitterProgress * 20 );

        }
    return jitter;
    }





void Level::freezeLevel( char inFrozen ) {
    mFrozen = inFrozen;
    }



char Level::isFrozen() {
    return mFrozen;
    }



void Level::startPlayerImmortal() {
    mPlayerImmortalSteps = (int)( 60 / frameRateFactor );
    }



void Level::drawFloorEdges( char inDraw ) {
    if( inDraw && !mDrawFloorEdges ) {
        // start fade-in
        mEdgeFadeIn = 0;
        }
    
    mDrawFloorEdges = inDraw;
    }



// how close we can get to wall
// one game pixel
#define WALL_LIMIT 0.4375
#define WALL_BUFFER 0.0625


doublePair Level::stopMoveWithWall( doublePair inStart,
                                    doublePair inMoveDelta ) {
    
    doublePair newPos = inStart;


    double velocityX = inMoveDelta.x;
    double velocityY = inMoveDelta.y;


    doublePair movePos = add( newPos, inMoveDelta );
    

    // consider each component of move separately

    

    // is a component move too close to wall?
    // push a bit farther to test

    doublePair testXMove = movePos;
    
    if( velocityX > 0 ) {
        testXMove.x += WALL_BUFFER;
        }
    else if( velocityX < 0 ) {
        testXMove.x -= WALL_BUFFER;
        }


    doublePair testYMove = movePos;
    
    if( velocityY > 0 ) {
        testYMove.y += WALL_BUFFER;
        }
    else if( velocityY < 0 ) {
        testYMove.y -= WALL_BUFFER;
        }
    


    int xMoveOkay = false;

    if( !isWall( testXMove ) ) {    
        newPos.x = movePos.x;
        xMoveOkay = true;
        }
    else {
        
        // consider test x move alone
        
        testXMove.y = inStart.y;

        if( !isWall( testXMove ) ) {
            // only too close to wall because of y component of move

            // keep full x move
            newPos.x = movePos.x;

            // get as close as possible with y component
            int intY = (int)rint( inStart.y );
            if( velocityY > 0 ) {
                newPos.y = intY + WALL_LIMIT;
                }
            else {
                newPos.y = intY - WALL_LIMIT;
                }

            return newPos;
            }
        
        
        // too close to wall, even with no y move at all
        // get as close as possible instead
        int intX = (int)rint( inStart.x );
        if( velocityX > 0 ) {
            newPos.x = intX + WALL_LIMIT;
            }
        else {
            newPos.x = intX - WALL_LIMIT;
            }
        }


    if( !isWall( testYMove ) ) {    
        newPos.y = movePos.y;
        }
    else {

        // consider test y move alone
        
        testYMove.x = inStart.x;

        if( !isWall( testYMove ) ) {
            // only too close to wall because of x component of move
            
            // keep full y move
            newPos.y = movePos.y;

            // get as close as possible with x component
            int intX = (int)rint( inStart.x );
            if( velocityX > 0 ) {
                newPos.x = intX + WALL_LIMIT;
                }
            else {
                newPos.x = intX - WALL_LIMIT;
                }

            return newPos;
            }
        

        // too close to wall, even with no x move at all
        // get as close as possible instead
        int intY = (int)rint( inStart.y );
        if( velocityY > 0 ) {
            newPos.y = intY + WALL_LIMIT;
            }
        else {
            newPos.y = intY - WALL_LIMIT;
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
                       char inEnemyBulletMarker ) {

    //double exactAimDist = distance( inAimPosition, inPosition );

    //double distanceScaleFactor = exactAimDist / 10;
    
    //double inAccuracy = getAccuracy( inPowers );
    double inSpread = getSpread( inPowers );
    double inHeatSeek = getHeatSeek( inPowers );
    
    double distance = getBulletDistance( inPowers );    
    
    int bounce = getBounce( inPowers );
    
    double explode = getExplode( inPowers );

    float size = getBulletSize( inPowers );


    /*
      // for testing
    if( inPlayerBullet ) {
        explode = 4.9;
        //inSpread = 9.9;
        bounce = 9;
        distance = 29;
        }
    */

    /*
      Ignore accuracy for now.
    
    inAccuracy *= distanceScaleFactor;

    inAimPosition.x += 
        randSource.getRandomBoundedDouble( -inAccuracy, inAccuracy );
    inAimPosition.y += 
        randSource.getRandomBoundedDouble( -inAccuracy, inAccuracy );
    */




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
                     inPlayerBullet, size, inEnemyBulletMarker, false, false };
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
                      inPlayerBullet, size, inEnemyBulletMarker };
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
                             inPlayerBullet, size, inEnemyBulletMarker };
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
                              inPlayerBullet, size, inEnemyBulletMarker };
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
                 inPlayerBullet, size, inEnemyBulletMarker };
    mBullets.push_back( b );
    

    }





void Level::pushAllMusicIntoPlayer() {    
    lockAudio();
    
    for( int i=0; i<mEnemies.size(); i++ ) {
        Enemy *e = mEnemies.getElement( i );
        
        setNoteSequence( e->musicNotes );
        }

    for( int i=0; i<mPowerUpTokens.size(); i++ ) {
        PowerUpToken *t = mPowerUpTokens.getElement( i );
        
        setNoteSequence( t->musicNotes );
        }


    setNoteSequence( mPlayerMusicNotes );
    
    setNoteSequence( mRiseDrumBeat.parts[0] );
    setNoteSequence( mRiseDrumBeat.parts[1] );
    

    if( mHarmonyNotes.partIndex != PARTS - 1 ) {
        printf( "Copying timbre from %d into harmony slot\n", 
                mHarmonyNotes.partIndex );
        
        // this is a reference to some other part
        // copy timbre into harmony slot
        setCopiedPart( mHarmonyNotes.partIndex );
        }
    
    // copy harmony sequence, because it doesn't necessarily reference
    // PARTS - 1  (which we want to preserve, for when we need to re-set
    //  it later)
    NoteSequence harmonyCopy = mHarmonyNotes;
    
    harmonyCopy.partIndex = PARTS - 1;

    setNoteSequence( harmonyCopy );
    
    setLoudnessForAllParts();

    unlockAudio();    
    }




char Level::isInsideEnemy() {
    return mInsideEnemy;
    }


char Level::isInsidePlayer() {
    return !( mInsideEnemy || mInsidePowerUp );
    }



char Level::isKnockDown() {
    return mKnockDown;
    }



int Level::getTokenRecursionDepth() {
    return mTokenRecursionDepth;
    }


int Level::getFloorTokenLevel() {
    return mFloorTokenLevel;
    }


int Level::getDifficultyLevel() {
    return mDifficultyLevel;
    }




doublePair Level::getNextPlayerPosTowardRise( double inMoveSpeed ) {    
    GridPos closestRiseGrid = mRisePosition;

    if( mSymmetrical && 
        distance( mRiseWorldPos2, mPlayerPos ) < 
        distance( mRiseWorldPos, mPlayerPos ) ) {
        
        closestRiseGrid = mRisePosition2;
        }

    GridPos nextStepGrid = pathFind( getGridPos( mPlayerPos ),
                                     mPlayerPos, closestRiseGrid, 
                                     inMoveSpeed );
    
    return sGridWorldSpots[ nextStepGrid.y ][ nextStepGrid.x ];
    }

    

