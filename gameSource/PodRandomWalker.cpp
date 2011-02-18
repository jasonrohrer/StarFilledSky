#include "PodRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;


PodRandomWalker::PodRandomWalker( int inLowX, int inLowY, 
                                  int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {
    
    mNextBranchPoint.x = 0;
    mNextBranchPoint.y = 0;
    
    mNextPointIndex = 0;
    
    mNextBranchDirection.x = randSource.getRandomBoundedDouble( -1, 1 );
    mNextBranchDirection.y = randSource.getRandomBoundedDouble( -1, 1 );
    }

        
        
GridPos PodRandomWalker::getNextStep( GridPos inCurrentPos ) {
    
    if( mCurrentPodPoints.size() == 0 ) {
        startNewPod( inCurrentPos );
        }

    GridPos p;
    

    if( mNextPointIndex < mCurrentPodPoints.size() ) {
        p = *( mCurrentPodPoints.getElement( mNextPointIndex ) );
        mNextPointIndex ++;
        }
    else {
        // final point, next branch point
        p = mNextBranchPoint;

        mCurrentPodPoints.deleteAll();

        // branch off existing pod
        startNewPod( mNextBranchPoint );
        }
    
    return p;
    }

