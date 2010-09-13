#include "doublePair.h"
#include "minorGems/util/SimpleVector.h"

#define MAX_LEVEL_W  400
#define MAX_LEVEL_H  400


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





class Level {
        

    public:
        
        Level();


        ~Level();

        
        // must be called before each invokation of drawLevel to have effect
        
        // position of item that should be drawn as a transparent window
        // on the next call to drawLevel
        // this will be set up as a pass-through stencil that will filter
        // all drawing after drawLevel until stopStencil is called
        void setItemWindowPosition( doublePair inPosition );
        
        // after drawing through stencil, call this to draw item body shade 
        // on top.
        void drawWindowShade( double inFade );


        void drawLevel( doublePair inViewCenter );
        

        char isWall( doublePair inPos );
        
        char isEnemy( doublePair inPos, int *outEnemyIndex = NULL );
        
        char isRiseSpot( doublePair inPos );
        

        doublePair getEnemyCenter( int inEnemyIndex );
        

        // freeze level step updates during drawLevel
        void freezeLevel( char inFrozen );
        
        
        doublePair stopMoveWithWall( doublePair inStart,
                                     doublePair inMoveDelta );
        

        void addBullet( doublePair inPosition,
                        doublePair inVelocity, char inPlayerBullet );


    protected:
        
        
        void step( doublePair inViewCenter );
        

        char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
        
        SimpleVector<Bullet> mBullets;

        SimpleVector<Enemy> mEnemies;
        
        doublePair mRisePosition;
        
        char mFrozen;
        

        char mWindowSet;
        int mWindowItemIndex;
        
    };

        
