#include "RandomWalker.h"

#include "minorGems/game/doublePair.h"


class DiagRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        DiagRandomWalker( int inLowX, int inLowY, 
                            int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );

        
        virtual int getStepsLeftInBatch() {
            return mStepsLeft;
            }


    protected:
        
        void pickNewDir();
        

        doublePair mPos;
        
        doublePair mVelocity;
                
        
        int mStepsLeft;
        

        char mStartPosSet;
        
    };


          
