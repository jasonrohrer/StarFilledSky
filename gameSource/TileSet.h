#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"

#include "ColorScheme.h"


#define NUM_TILE_PATTERNS 16

class TileSet {

    public:


        // NEXT:  pass allowed color set into constructor 
        TileSet();
        
        ~TileSet();

        // called before each batch of walls
        void startDrawingWalls();
        

        void drawWall( doublePair inPosition );


        // edges to draw packed into lower 4 bits
        // inEdgeFlags & 1  == top on
        // inEdgeFlags & 2  == right on
        // inEdgeFlags & 4  == bottom on
        // inEdgeFlags & 8  == left on
        void drawWallEdges( doublePair inPosition, char inEdgeFlags );
        
        
        void drawFloor( doublePair inPosition );


    protected:
        
        int mNextTileToDraw;

        SpriteHandle makeWallTile( ColorScheme inColors );
        

        SpriteHandle mWall[ NUM_TILE_PATTERNS ];
        //SpriteHandle mWall;
        
        SpriteHandle mWallEdges[4];

        SpriteHandle mFloor[ NUM_TILE_PATTERNS ];
        //SpriteHandle mFloor;
        

    };
