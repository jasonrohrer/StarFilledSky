#include "RandomWalker.h"

#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


class PodRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        PodRandomWalker( int inLowX, int inLowY, 
                              int inHighX, int inHighY );

        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        
        virtual int getStepsLeftInBatch() {
            return mCurrentPodPoints.size() - mNextPointIndex;
            }


    protected:

        // implemented by subclasses
        virtual void startNewPod( GridPos inIncludePos ) = 0;
        
        SimpleVector<GridPos> mCurrentPodPoints;
        
        int mNextPointIndex;

        GridPos mNextBranchPoint;
        doublePair mNextBranchDirection;
                
    };


          
