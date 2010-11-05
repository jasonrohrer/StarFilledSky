#include "RandomWalker.h"

class BasicRandomWalker : public RandomWalker {
    public:
        
        // boundaries for walk
        BasicRandomWalker( int inLowX, int inLowY, 
                           int inHighX, int inHighY );
                
        
        virtual ~BasicRandomWalker();
        
        
        virtual GridPos getNextStep( GridPos inCurrentPos );
        
    };


          
