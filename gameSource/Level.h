#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


#include "PlayerSprite.h"
#include "EnemySprite.h"
#include "TileSet.h"
#include "ColorScheme.h"
#include "PowerUpSet.h"
#include "PowerUpSprite.h"


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
        float size;
    } Bullet;


typedef struct Enemy {
        doublePair position;
        doublePair velocity;
        doublePair accel;
        int stepsBetweenBullets;
        int stepsTilNextBullet;
        EnemySprite *sprite;
        PowerUpSet *powers;
        int health;
        float healthBarFade;
    } Enemy;





enum itemType { player = 0,
                enemy,
                power };                




typedef struct WindowPosition {
        int index;
        itemType type;
    } WindowPosition;



typedef struct GridPos {
        int x;
        int y;
    } GridPos;



typedef struct PowerUpToken {
        PowerUp power;
        GridPos gridPosition;
        doublePair position;
        PowerUpSprite *sprite;
        PowerUpSet *subPowers;
    };




class Level {
        

    public:

        // Pass NULL to generate a fresh scheme
        // destroyed by caller
        Level( ColorScheme *inPlayerColors=NULL,
               ColorScheme *inColors=NULL, 
               int inLevelNumber = 0,
               char inSymmetrical=true );


        ~Level();

        
        // compacts level, discarding reproducible data
        // level cannot be drawn or stepped until decompact is called
        void compactLevel();
        
        void decompactLevel();


        
        void setPlayerPos( doublePair inPos );
        void setMousePos( doublePair inPos );
        
        void setEnteringMouse( char inEntering );
        


        // must be called before each invokation of drawLevel to have effect
        
        // position of item that should be drawn as a transparent window
        // on the next call to drawLevel
        // this will be set up as a pass-through stencil that will filter
        // all drawing after drawLevel until stopStencil is called
        void setItemWindowPosition( doublePair inPosition, 
                                    itemType inType );
        
        // after drawing through stencil, call this to draw item body shade 
        // on top.
        void drawWindowShade( double inFade, double inFrameFade );

        void forgetItemWindow();


        void drawLevel( doublePair inViewCenter, double inViewSize );
        

        char isWall( doublePair inPos );
        
        char isEnemy( doublePair inPos, int *outEnemyIndex = NULL );
        char isPlayer( doublePair inPos  );
      
        char isPowerUp( doublePair inPos, int *outPowerUpIndex = NULL );

        // removes it from world
        PowerUp getPowerUp( doublePair inPos );
        


        ColorScheme getLevelColors();
        
        
        // 0 player
        // 1 enemy
        ColorScheme getEnteringPointColors( doublePair inPosition,
                                            itemType inType );
        
        // level number of subLevel if entered here
        int getEnteringPointSubLevel( doublePair inPosition,
                                      itemType inType );
        
        
        int getLevelNumber();
        

        BorderSprite *getLastEnterPointSprite();
        
        PowerUpSet *getLastEnterPointPowers();


        PlayerSprite *getPlayerSprite();
        
        PowerUpSet *getPlayerPowers();
        
        
        void getPlayerHealth( int *outValue, int *outMax );
        void restorePlayerHealth();
        



        char isRiseSpot( doublePair inPos );
        

        doublePair getEnemyCenter( int inEnemyIndex );
        doublePair getPowerUpCenter( int inPowerUpIndex );
        

        // freeze level step updates during drawLevel
        void freezeLevel( char inFrozen );
        

        void drawFloorEdges( char inDraw );
        
        
        doublePair stopMoveWithWall( doublePair inStart,
                                     doublePair inMoveDelta );
        

        void addBullet( doublePair inPosition,
                        doublePair inVelocity, char inPlayerBullet,
                        int inEnemyIndex = -1 );


    protected:
        
        
        void step();
        
        void drawPlayer( double inFade );
        void drawMouse( double inFade );

        GridPos getGridPos( doublePair inWorldPos );


        // generate data that can be reproduced from the seed
        void generateReproducibleData();
        
        // free up memory consumed by reproducible data
        void freeReproducibleData();
        
        char mDataGenerated;
        

        // random generator state that generated this level
        unsigned int mRandSeedState;
        
        
        int mLevelNumber;

        char mSymmetrical;
        

        // dynamically allocate these to make them compactable

        // need these for quick wall collision detection
        //char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
        char **mWallFlags;

        // save ram, because grid is a sparse matrix
        // map each grid space to an index
        //short mSquareIndices[MAX_LEVEL_H][MAX_LEVEL_W];
        short **mSquareIndices;

        //char mFloorEdgeFlags[MAX_LEVEL_SQUARES];
        char *mFloorEdgeFlags;
        

        int mNumFloorSquares;
        int mNumWallSquares;
        int mNumUsedSquares;
        // dynamically allocate to save space based on actual number of
        // squares
        Color *mGridColors;

        Color *mHardGridColors;
        Color *mSoftGridColors;

        float *mColorMix;
        float *mColorMixDelta;
        
        // using square indices
        char *mWallFlagsIndexed;
        GridPos *mIndexToGridMap;
        doublePair **mGridWorldSpots;
        
        

        // static to save RAM
        // maps each grid spot to world coordinates
        static char sGridWorldSpotsComputed;
        static doublePair sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];


        SimpleVector<Bullet> mBullets;

        SimpleVector<Enemy> mEnemies;
        
        SimpleVector<PowerUpToken> mPowerUpTokens;


        GridPos mRisePosition;
        
        char mFrozen;
        
        char mDrawFloorEdges;
        float mEdgeFadeIn;
        

        char mWindowSet;
        WindowPosition mWindowPosition;

        doublePair mMousePos;
        char mEnteringMouse;
        doublePair mPlayerPos;


        PlayerSprite mPlayerSprite;
        PowerUpSet *mPlayerPowers;
        
        int mPlayerHealth;
        


        //TileSet mTileSet;
        ColorScheme mColors;
        

        BorderSprite *mLastEnterPointSprite;
        PowerUpSet *mLastEnterPointPowers;
        
        // -1 if last enter point was not a power token
        int mLastEnterPointPowerTokenIndex;
        

    };

        
