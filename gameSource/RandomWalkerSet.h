#ifndef RANDOM_WALKER_SET_INCLUDED
#define RANDOM_WALKER_SET_INCLUDED


#include "RandomWalker.h"
#include "fixedSpriteBank.h"


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
      
        // set specific to a power up token type
        RandomWalkerSet( spriteID inPowerType );
        
        
        // picks a walker from set at random
        // destroyed by caller
        RandomWalker *pickWalker( int inLowX, int inLowY, 
                                  int inHighX, int inHighY );
        

    protected:
        
        walkerType mWalkers[ WALKER_SET_SIZE ];

    };


#endif
