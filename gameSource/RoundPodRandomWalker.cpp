#include "RoundPodRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;



void RoundPodRandomWalker::startNewPod( GridPos inIncludePos ) {
    
    int radius = randSource.getRandomBoundedInt( 3, 7 );
    
    double centerOffset = randSource.getRandomBoundedDouble( 0, radius - 1 );
    
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
    mNextBranchPoint = 
        *( edgePoints.getElement(
               randSource.getRandomBoundedInt( 0,
                                               edgePoints.size() - 1 ) ) );
    */

    double angle = randSource.getRandomBoundedDouble( -0.5, 0.5 );
    
    centerOffsetVector = rotate( centerOffsetVector, angle );    
    
    mNextBranchPoint = add( center, centerOffsetVector );

    mNextBranchDirection.x =
        mNextBranchPoint.x - center.x;
    mNextBranchDirection.y =
        mNextBranchPoint.y - center.y;
    

    mNextPointIndex = 0;
    }



RoundPodRandomWalker::RoundPodRandomWalker( int inLowX, int inLowY, 
                                            int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {
    
    mNextPointIndex = 0;
    
    mNextBranchDirection.x = randSource.getRandomBoundedDouble( -1, 1 );
    mNextBranchDirection.y = randSource.getRandomBoundedDouble( -1, 1 );
    }

        
        
GridPos RoundPodRandomWalker::getNextStep( GridPos inCurrentPos ) {
    
    if( mCurrentPodPoints.size() == 0 ) {
        startNewPod( inCurrentPos );
        }
    

    GridPos p = *( mCurrentPodPoints.getElement( mNextPointIndex ) );

    mNextPointIndex ++;
    
    if( mNextPointIndex >= mCurrentPodPoints.size() ) {
        mCurrentPodPoints.deleteAll();

        // branch off existing pod
        startNewPod( mNextBranchPoint );
        }
    
    return p;
    }

