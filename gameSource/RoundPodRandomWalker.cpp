#include "RoundPodRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;



void RoundPodRandomWalker::startNewPod( GridPos inIncludePos ) {
    
    int radius = randSource.getRandomBoundedInt( 3, 7 );
    
    doublePair centerOffsetVector = mNextBranchDirection;
    centerOffsetVector = forceLength( centerOffsetVector, radius - 1 );
    

    GridPos center = add( inIncludePos, centerOffsetVector );
    
    // box around circle
    int lowX = center.x - radius;
    int highX = center.x + radius;

    int lowY = center.y - radius;
    int highY = center.y + radius;

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
            
            if( d <= radius ) {
                mCurrentPodPoints.push_back( p );

                if( d > radius - 1 ) {
                    edgePoints.push_back( p );
                    }
                }
            }
        }

    /*
    */

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



RoundPodRandomWalker::RoundPodRandomWalker( int inLowX, int inLowY, 
                                            int inHighX, int inHighY )
        : PodRandomWalker( inLowX, inLowY, inHighX, inHighY ) {
    }    
    
