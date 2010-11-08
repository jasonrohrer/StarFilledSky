#include "RandomWalker.h"

#include "minorGems/game/doublePair.h"


class CurvedRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        CurvedRandomWalker( int inLowX, int inLowY, 
                            int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        

    protected:
        
        void pickNewCurve();
        

        doublePair mPos;
        
        doublePair mVelocity;
        double mDeltaAngle;
        double mDeltaDeltaAngle;
        double mDeltaDeltaDeltaAngle;
        
        
        int mStepsLeft;
        

        char mStartPosSet;
        
    };


          
