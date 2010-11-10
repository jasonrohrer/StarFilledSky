#include "PodRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;


PodRandomWalker::PodRandomWalker( int inLowX, int inLowY, 
                                  int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {
    
    mNextPointIndex = 0;
    
    mNextBranchDirection.x = randSource.getRandomBoundedDouble( -1, 1 );
    mNextBranchDirection.y = randSource.getRandomBoundedDouble( -1, 1 );
    }

        
        
GridPos PodRandomWalker::getNextStep( GridPos inCurrentPos ) {
    
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

