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




RandomWalkerSet::RandomWalkerSet( spriteID inPowerType ) {
    switch( inPowerType ) {
        case powerUpEmpty:
        case powerUpBulletSize:
        case powerUpRapidFire:
        case powerUpBulletDistance:
        case powerUpCornering:
        case powerUpSticky:
            mWalkers[0] = straight;
            mWalkers[1] = rectPod;
            mWalkers[2] = straight;
            break;
        case powerUpHeart:
            mWalkers[0] = curved;
            mWalkers[1] = rectPod;
            mWalkers[2] = roundPod;
            break;
        // case powerUpAccuracy:
        case powerUpBulletSpeed:
        case powerUpBounce:
            mWalkers[0] = diagonal;
            mWalkers[1] = rectPod;
            mWalkers[2] = diagonal;
            break;
        case powerUpSpread:
        case powerUpExplode:
            mWalkers[0] = diagonal;
            mWalkers[1] = rectPod;
            mWalkers[2] = straight;
            break;
        case powerUpHeatSeek:
            mWalkers[0] = curved;
            mWalkers[1] = rectPod;
            mWalkers[2] = diagonal;
            break;
        default:
            mWalkers[0] = straight;
            mWalkers[1] = rectPod;
            mWalkers[2] = straight;
            break;
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
