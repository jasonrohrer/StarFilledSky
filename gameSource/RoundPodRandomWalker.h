#include "RandomWalker.h"

#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


class RoundPodRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        RoundPodRandomWalker( int inLowX, int inLowY, 
                              int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        
        virtual int getStepsLeftInBatch() {
            return mCurrentPodPoints.size() - mNextPointIndex;
            }


    protected:

        void startNewPod( GridPos inIncludePos );
        
        SimpleVector<GridPos> mCurrentPodPoints;
        
        int mNextPointIndex;

        GridPos mNextBranchPoint;
        doublePair mNextBranchDirection;
                
    };


          
