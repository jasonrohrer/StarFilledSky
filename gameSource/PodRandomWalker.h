#ifndef POD_RANDOM_WALKER_INCLUDED
#define POD_RANDOM_WALKER_INCLUDED


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
            if( mCurrentPodPoints.size() > 0 ) {
                // one extra point to return last in batch:
                // the next branch point
                return mCurrentPodPoints.size() - mNextPointIndex + 1;
                }
            else {
                return 0;
                }
            }


    protected:

        // implemented by subclasses
        virtual void startNewPod( GridPos inIncludePos ) = 0;
        
        SimpleVector<GridPos> mCurrentPodPoints;
        
        int mNextPointIndex;

        GridPos mNextBranchPoint;
        doublePair mNextBranchDirection;
                
    };


#endif

          
