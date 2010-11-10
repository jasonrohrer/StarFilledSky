#include "RandomWalker.h"

class BasicRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        BasicRandomWalker( int inLowX, int inLowY, 
                           int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        

        virtual int getStepsLeftInBatch() {
            return mStepsLeft;
            }

    protected:
        int mStepsLeft;

    };


          
