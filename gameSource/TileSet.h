#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"


class TileSet {

    public:

        TileSet();
        
        ~TileSet();

        void drawWall( doublePair inPosition );


        // edges to draw packed into lower 4 bits
        // inEdgeFlags & 1  == top on
        // inEdgeFlags & 2  == right on
        // inEdgeFlags & 4  == bottom on
        // inEdgeFlags & 8  == left on
        void drawWallEdges( doublePair inPosition, char inEdgeFlags );
        
        
        void drawFloor( doublePair inPosition );


    protected:
        
        SpriteHandle mWall;
        
        SpriteHandle mWallEdges[4];

        SpriteHandle mFloor;
        

    };
