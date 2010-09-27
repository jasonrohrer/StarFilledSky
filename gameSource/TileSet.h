#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"


class TileSet {

    public:

        TileSet();
        
        ~TileSet();

        void drawWall( doublePair inPosition );
        
        void drawFloor( doublePair inPosition );


    protected:
        
        SpriteHandle mWall;
        SpriteHandle mFloor;
        

    };
