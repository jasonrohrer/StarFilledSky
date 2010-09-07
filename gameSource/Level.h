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

        
        void drawLevel( doublePair inViewCenter );
        

        char isWall( doublePair inPos );
        
        
        doublePair stopMoveWithWall( doublePair inStart,
                                     doublePair inMoveDelta );
        

        void addBullet( doublePair inPosition,
                        doublePair inVelocity, char inPlayerBullet );


    protected:
        

        char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
        
        SimpleVector<Bullet> mBullets;

        SimpleVector<Enemy> mEnemies;
        

    };

        
