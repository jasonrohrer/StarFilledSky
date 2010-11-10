#ifndef RANDOM_WALKER_INCLUDED
#define RANDOM_WALKER_INCLUDED


#include "GridPos.h"


class RandomWalker {

    public:
        
        // boundaries for walk
        RandomWalker( int inLowX, int inLowY, 
                      int inHighX, int inHighY ) {
            mLow.x = inLowX;
            mLow.y = inLowY;
            mHigh.x = inHighX;
            mHigh.y = inHighY;
            }
        
        
        virtual ~RandomWalker() {
            };
        
        
        virtual GridPos getNextStep( GridPos inCurrentPos ) = 0;
        

        virtual int getStepsLeftInBatch() = 0;
        

    protected:
        
        GridPos mLow, mHigh;
        
        
    };



#endif
