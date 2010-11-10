#include "BasicRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



BasicRandomWalker::BasicRandomWalker( int inLowX, int inLowY, 
                                      int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {

    mStepsLeft = randSource.getRandomBoundedInt( 5, 30 );
    }
        
        
GridPos BasicRandomWalker::getNextStep( GridPos inCurrentPos ) {
    int x = inCurrentPos.x;
    int y = inCurrentPos.y;
    

    // move only in x or y, not both
    if( randSource.getRandomBoolean() ) {
        x += randSource.getRandomBoundedInt( -1, 1 );
        }
    else {
        y += randSource.getRandomBoundedInt( -1, 1 );
        }
        
    if( x > mHigh.x ) {
        x = mHigh.x;
        }
    else if( x < mLow.x ) {
        x = mLow.x;
        }

    if( y > mHigh.y ) {
        y = mHigh.y;
        }
    else if( y < mLow.y ) {
        y = mLow.y;
        }    

    mStepsLeft--;
    
    if( mStepsLeft <= 0 ) {
        mStepsLeft = randSource.getRandomBoundedInt( 5, 30 );
        }

    GridPos p = { x, y };
    
    return p;
    }

