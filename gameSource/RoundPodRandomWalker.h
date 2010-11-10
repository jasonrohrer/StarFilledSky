#include "PodRandomWalker.h"

#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


class RoundPodRandomWalker : public PodRandomWalker {
    public:
        
        // boundaries for walk
        RoundPodRandomWalker( int inLowX, int inLowY, 
                              int inHighX, int inHighY );

    protected:

        virtual void startNewPod( GridPos inIncludePos );
                        
    };


          
