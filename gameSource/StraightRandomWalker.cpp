#include "StraightRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



void StraightRandomWalker::pickNewDir() {
    mXDir = 0;
    mYDir = 0;

    if( randSource.getRandomBoolean() ) {
        if( randSource.getRandomBoolean() ) {
            mYDir = 1;
            }
        else {
            mYDir = -1;
            }
        }
    else {
        if( randSource.getRandomBoolean() ) {
            mXDir = 1;
            }
        else {
            mXDir = -1;
            }
        }

    mStepsLeft = randSource.getRandomBoundedInt( 1, 20 );
    }


StraightRandomWalker::StraightRandomWalker( int inLowX, int inLowY, 
                                      int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {


    pickNewDir();
    }
        
        
GridPos StraightRandomWalker::getNextStep( GridPos inCurrentPos ) {
    int x = inCurrentPos.x;
    int y = inCurrentPos.y;
    

    x += mXDir;
    y += mYDir;


    char hitEdge = false;
    
    // switch dir when hit boundary
    if( x > mHigh.x ) {
        x = mHigh.x;
        hitEdge = true;
        }
    else if( x < mLow.x ) {
        x = mLow.x;
        hitEdge = true;
        }

    if( y > mHigh.y ) {
        y = mHigh.y;
        hitEdge = true;
        }
    else if( y < mLow.y ) {
        y = mLow.y;
        hitEdge = true;
        }    
    
    mStepsLeft --;
    
    if( mStepsLeft <= 0 || hitEdge ) {
        pickNewDir();
        }
    


    GridPos p = { x, y };
    
    return p;
    }

