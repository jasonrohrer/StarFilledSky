#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


#include "PlayerSprite.h"
#include "TileSet.h"
#include "ColorScheme.h"


#define MAX_LEVEL_W  400
#define MAX_LEVEL_H  400

#define MAX_FLOOR_SQUARES 1000
// 1000 max floor squares, plus max of 2 wall tiles per floor square, plus 6
// (worst case)
#define MAX_LEVEL_SQUARES 3006

typedef struct Bullet {
        doublePair position;
        doublePair velocity;
        char playerFlag;
    } Bullet;


typedef struct Enemy {
        doublePair position;
        doublePair velocity;
        doublePair accel;
        int stepsBetweenBullets;
        int stepsTilNextBullet;
    } Enemy;



typedef struct WindowPosition {
        int index;
        char isPlayer;
    } WindowPosition;





class Level {
        

    public:
        
        Level();


        ~Level();

        
        void setPlayerPos( doublePair inPos );
        void setMousePos( doublePair inPos );
        
        void setEnteringMouse( char inEntering );
        


        // must be called before each invokation of drawLevel to have effect
        
        // position of item that should be drawn as a transparent window
        // on the next call to drawLevel
        // this will be set up as a pass-through stencil that will filter
        // all drawing after drawLevel until stopStencil is called
        void setItemWindowPosition( doublePair inPosition );
        
        // after drawing through stencil, call this to draw item body shade 
        // on top.
        void drawWindowShade( double inFade );

        void forgetItemWindow();


        void drawLevel( doublePair inViewCenter, double inViewSize );
        

        char isWall( doublePair inPos );
        
        char isEnemy( doublePair inPos, int *outEnemyIndex = NULL );
        char isPlayer( doublePair inPos  );
        
        char isRiseSpot( doublePair inPos );
        

        doublePair getEnemyCenter( int inEnemyIndex );
        

        // freeze level step updates during drawLevel
        void freezeLevel( char inFrozen );
        

        void drawFloorEdges( char inDraw );
        
        
        doublePair stopMoveWithWall( doublePair inStart,
                                     doublePair inMoveDelta );
        

        void addBullet( doublePair inPosition,
                        doublePair inVelocity, char inPlayerBullet );


    protected:
        
        
        void step();
        
        void drawPlayer( double inFade );
        void drawMouse( double inFade );
        

        // need these for quick wall collision detection
        char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];

        // save ram, because grid is a sparse matrix
        // map each grid space to an index
        short mSquareIndices[MAX_LEVEL_H][MAX_LEVEL_W];
        
        char mFloorEdgeFlags[MAX_LEVEL_SQUARES];
        
        Color mGridColors[MAX_LEVEL_SQUARES];
        

        // static to save RAM
        // maps each grid spot to world coordinates
        static char sGridWorldSpotsComputed;
        static doublePair sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];


        SimpleVector<Bullet> mBullets;

        SimpleVector<Enemy> mEnemies;
        
        doublePair mRisePosition;
        
        char mFrozen;
        
        char mDrawFloorEdges;
        float mEdgeFadeIn;
        

        char mWindowSet;
        WindowPosition mWindowPosition;
        

        doublePair mMousePos;
        char mEnteringMouse;
        doublePair mPlayerPos;


        PlayerSprite mPlayerSprite;

        //TileSet mTileSet;
        ColorScheme mColors;
        
    };

        
