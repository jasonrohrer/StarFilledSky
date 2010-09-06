#include "doublePair.h"


#define MAX_LEVEL_W  400
#define MAX_LEVEL_H  400


class Level {
        

    public:
        
        Level();


        ~Level();

        
        void drawLevel( doublePair inViewCenter );
        

    protected:
        

        char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
        
        
    };

        
