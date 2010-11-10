#include "DiagRandomWalker.h"

#include "minorGems/util/random/CustomRandomSource.h"

#include <math.h>
#include <stdio.h>

extern CustomRandomSource randSource;



void DiagRandomWalker::pickNewDir() {
    mVelocity.x = randSource.getRandomBoundedDouble( -1, 1 );
    mVelocity.y = randSource.getRandomBoundedDouble( -1, 1 );
    
    mVelocity = mult( normalize( mVelocity ), 0.5 );

    mStepsLeft = randSource.getRandomBoundedInt( 5, 30 );
    }


DiagRandomWalker::DiagRandomWalker( int inLowX, int inLowY, 
                                      int inHighX, int inHighY )
        : RandomWalker( inLowX, inLowY, inHighX, inHighY ) {

    mPos.x = 0;
    mPos.y = 0;
    mStartPosSet = false;
    
    pickNewDir();
    }
        
        
GridPos DiagRandomWalker::getNextStep( GridPos inCurrentPos ) {
    if( !mStartPosSet ) {
        mPos.x = inCurrentPos.x;
        mPos.y = inCurrentPos.y;
        mStartPosSet = true;
        }

    mPos = add( mPos, mVelocity );    
    


    int x = (int)rint( mPos.x );
    int y = (int)rint( mPos.y );
    
    // pure diagonal moves are a problem
    int xDelta = x - inCurrentPos.x;
    int yDelta = y - inCurrentPos.y;
    
    if( yDelta != 0 && xDelta != 0 ) {
    
        // ignore smaller non-rounded component of move 
        // to prevent pure diagonal move

        double xDeltaSize = fabs( mPos.x - inCurrentPos.x );
        double yDeltaSize = fabs( mPos.y - inCurrentPos.y );
        
        if( xDeltaSize > yDeltaSize ) {
            y = inCurrentPos.y;
            }
        else {
            x = inCurrentPos.x;
            }
        }
    
        

    
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

    if( hitEdge ) {
        mVelocity = mult( mVelocity, -1 );
        
        mPos.x = x;
        mPos.y = y;
        }
    
    
    mStepsLeft --;
    
    if( mStepsLeft <= 0 ) {
        pickNewDir();
        }
    


    GridPos p = { x, y };

    return p;
    }

