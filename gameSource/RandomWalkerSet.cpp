#include "RandomWalkerSet.h"

#include "BasicRandomWalker.h"
#include "StraightRandomWalker.h"
#include "CurvedRandomWalker.h"
#include "RoundPodRandomWalker.h"
#include "RectPodRandomWalker.h"
#include "DiagRandomWalker.h"


#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;


RandomWalkerSet::RandomWalkerSet() {
    
    for( int i=0; i<WALKER_SET_SIZE; i++ ) {
        
        mWalkers[ i ] = 
            (walkerType)
            randSource.getRandomBoundedInt( 0, endWalkerType - 1 );
        }
    
    }



RandomWalker *RandomWalkerSet::pickWalker( int inLowX, int inLowY, 
                                           int inHighX, int inHighY ) {
    int pick = randSource.getRandomBoundedInt( 0, WALKER_SET_SIZE - 1 );
    
    switch( mWalkers[pick] ) {
        case straight:
            return new StraightRandomWalker( inLowX, inLowY, 
                                             inHighX, inHighY );
        case curved:
            return new CurvedRandomWalker( inLowX, inLowY, 
                                           inHighX, inHighY );
        case diagonal:
            return new DiagRandomWalker( inLowX, inLowY, 
                                         inHighX, inHighY );
        case roundPod:
            return new RoundPodRandomWalker( inLowX, inLowY, 
                                             inHighX, inHighY );
        case rectPod:
            return new RectPodRandomWalker( inLowX, inLowY, 
                                            inHighX, inHighY );
        default:
            return new BasicRandomWalker( inLowX, inLowY, 
                                          inHighX, inHighY );
        }
    }
