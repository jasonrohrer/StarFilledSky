#include "RandomWalker.h"

class StraightRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        StraightRandomWalker( int inLowX, int inLowY, 
                              int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        
        virtual int getStepsLeftInBatch() {
            return mStepsLeft;
            }
        

    protected:
        
        void pickNewDir();
        

        int mXDir;
        int mYDir;
        
        int mStepsLeft;
        
    };


          
