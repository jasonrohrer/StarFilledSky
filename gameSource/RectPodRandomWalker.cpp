#include "RectPodRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;



void RectPodRandomWalker::startNewPod( GridPos inIncludePos ) {
    
    int hRadius = randSource.getRandomBoundedInt( 2, 7 );
    int vRadius = randSource.getRandomBoundedInt( 2, 7 );
    
    int radius = hRadius;
    if( vRadius < hRadius ) {
        radius = vRadius;
        }

    doublePair centerOffsetVector = mNextBranchDirection;
    centerOffsetVector = forceLength( centerOffsetVector, radius - 1 );
    

    GridPos center = add( inIncludePos, centerOffsetVector );
    
    // box arect circle
    int lowX = center.x - hRadius;
    int highX = center.x + hRadius;

    int lowY = center.y - vRadius;
    int highY = center.y + vRadius;

    // cut off at edges of allowed region
    if( lowX < mLow.x ) {
        lowX = mLow.x;
        }
    if( lowY < mLow.y ) {
        lowY = mLow.y;
        }
    if( highX > mHigh.x ) {
        highX = mHigh.x;
        }
    if( highY > mHigh.y ) {
        highY = mHigh.y;
        }
    
    // track edges for next branch point
    SimpleVector<GridPos> edgePoints;
    
    for( int y=lowY; y<=highY; y++ ) {
        for( int x=lowX; x<=highX; x++ ) {
            
            GridPos p = { x, y };
            
            double d = distance( p, center );
            
            mCurrentPodPoints.push_back( p );

            if( d > radius - 1 ) {
                edgePoints.push_back( p );
                }
            }
        }

    double angle = randSource.getRandomBoundedDouble( -0.5, 0.5 );
    
    centerOffsetVector = rotate( centerOffsetVector, angle );    
    
    mNextBranchPoint = add( center, centerOffsetVector );

    // make sure in bounds

    if( mNextBranchPoint.x < mLow.x ||
        mNextBranchPoint.y < mLow.y ||
        mNextBranchPoint.x > mHigh.x ||
        mNextBranchPoint.y > mHigh.y ) {
        
        // pick random edge point instead
        mNextBranchPoint = 
            *( edgePoints.getElement(
                   randSource.getRandomBoundedInt( 0,
                                                   edgePoints.size() - 1 ) ) );
        }
    


    mNextBranchDirection.x =
        mNextBranchPoint.x - center.x;
    mNextBranchDirection.y =
        mNextBranchPoint.y - center.y;
    

    mNextPointIndex = 0;
    }



RectPodRandomWalker::RectPodRandomWalker( int inLowX, int inLowY, 
                                            int inHighX, int inHighY )
        : PodRandomWalker( inLowX, inLowY, inHighX, inHighY ) {
    }    
    
