#include "PodRandomWalker.h"

#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


class RectPodRandomWalker : public PodRandomWalker {
    public:
        
        // boundaries for walk
        RectPodRandomWalker( int inLowX, int inLowY, 
                              int inHighX, int inHighY );

    protected:

        virtual void startNewPod( GridPos inIncludePos );
                        
    };


          
