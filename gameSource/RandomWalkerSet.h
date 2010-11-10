#include "RandomWalker.h"


enum walkerType{ basic = 0,
                 straight,
                 curved,
                 diagonal,
                 roundPod,
                 rectPod,
                 endWalkerType };



#define WALKER_SET_SIZE 3


class RandomWalkerSet {

    public:
        
        // totally random set
        RandomWalkerSet();
        
        
        // picks a walker from set at random
        // destroyed by caller
        RandomWalker *pickWalker( int inLowX, int inLowY, 
                                  int inHighX, int inHighY );
        

    protected:
        
        walkerType mWalkers[ WALKER_SET_SIZE ];

    };

