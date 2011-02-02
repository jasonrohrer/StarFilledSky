#ifndef LEVEL_INCLUDED
#define LEVEL_INCLUDED


#include "minorGems/game/doublePair.h"
#include "minorGems/util/SimpleVector.h"


#include "PlayerSprite.h"
#include "EnemySprite.h"
#include "ColorScheme.h"
#include "PowerUpSet.h"
#include "PowerUpSprite.h"
#include "GridPos.h"
#include "RandomWalkerSet.h"
#include "NoteSequence.h"


#define MAX_LEVEL_W  96
#define MAX_LEVEL_H  MAX_LEVEL_W

#define MAX_FLOOR_SQUARES 1000
// 1000 max floor squares, plus max of 6 wall tiles per floor square, plus 30
// as end cap (worst case)
#define MAX_LEVEL_SQUARES 7030

typedef struct Bullet {
        doublePair position;
        doublePair velocity;
        double speed;
        double heatSeek;
        doublePair heatSeekWaypoint;
        double startDistance;
        double distanceLeft;
        int startBounces;
        int bouncesLeft;
        double explode;
        char playerFlag;
        float size;
        // matches firing enemy's bulletMarker
        char enemyMarker;
        // draw one last frame after a bullet hits a non-wall
        char finalFrame;
        // bullet should have no effect at the end of it's life when
        // it is barely visible
        char halfFadedOut;
    } Bullet;


typedef struct HitSmoke {
        doublePair position;
        float progress;
        float maxSize;
        // 0 enemy vs wall
        // 1 player vs wall
        // 2 damage
        // 3 enemy explosion
        char type;
        Color enemyColor;
    } HitSmoke;



typedef struct BloodStain {
        int floorIndex;
        float blendFactor;
    } BloodStain;


typedef struct GlowSpriteTrail {
        doublePair position;
        float fade;
        float progress;
        BorderSprite *sprite;
    } GlowSpriteTrail;


typedef struct RiseMarkerPathStep {
        int floorIndex;
        float blendFactor;
    } RiseMarkerPathStep;






typedef struct Enemy {
        doublePair position;
        doublePair velocity;
        doublePair accel;
        doublePair baseMoveDirection;
        int stepsBetweenBullets;
        int stepsTilNextBullet;
        EnemySprite *sprite;
        PowerUpSet *powers;
        int health;
        int lastMaxHealth;
        float healthBarFade;
        doublePair followNextWaypoint;
        Bullet *dodgeBullet;
        char circleDirection;
        double circleRadiusFactor;
        RandomWalkerSet walkerSet;
        NoteSequence musicNotes;
        int stepsUntilNextGlowTrail;
        int difficultyLevel;
        // int used to mark bullets that were fired by this enemy
        char bulletMarker;
    } Enemy;





enum itemType { player = 0,
                enemy,
                power };                




typedef struct WindowPosition {
        int index;
        itemType type;
    } WindowPosition;






typedef struct PowerUpToken {
        PowerUp power;
        GridPos gridPosition;
        doublePair position;
        PowerUpSprite *sprite;
        PowerUpSet *subPowers;
        NoteSequence musicNotes;
        int stepsUntilNextGlowTrail;
    };




class Level {
        

    public:

        // Pass NULL to generate a fresh scheme and walker set and notes
        // destroyed by caller
        Level( ColorScheme *inPlayerColors=NULL,
               NoteSequence *inPlayerMusicNotes = NULL,
               ColorScheme *inColors=NULL,
               RandomWalkerSet *inWalkerSet=NULL,
               NoteSequence *inMusicNotes=NULL,
               PowerUpSet *inSetPlayerPowers=NULL,
               int inLevelNumber = 0,
               char inSymmetrical=true,
               char inInsideEnemy=false,
               char inInsidePowerUp=false,
               char inIsKnockDown=false,
               int inTokenRecursionDepth=0,
               int inParentEnemyDifficultyLevel=0,
               int inParentTokenLevel=0,
               int inParentFloorTokenLevel=0,
               int inParentLevelDifficulty=0 );


        ~Level();

        
        // compacts level, discarding reproducible data
        // level cannot be drawn or stepped until decompact is called
        void compactLevel();
        
        void decompactLevel();


        
        void setPlayerPos( doublePair inPos );
        void setPlayerVelocity( doublePair inVelocity );
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
        void drawWindowShade( double inFade, double inFrameFade,
                              doublePair inViewCenter, double inViewSize );

        void forgetItemWindow();


        void drawLevel( doublePair inViewCenter, double inViewSize );
        

        char isWall( doublePair inPos );
        
        char isEnemy( doublePair inPos, int *outEnemyIndex = NULL );
        char isPlayer( doublePair inPos  );
      
        char isPowerUp( doublePair inPos, int *outPowerUpIndex = NULL,
                        char inWidePickup = false );


        PowerUp peekPowerUp( doublePair inPos );

        // removes it from world
        PowerUp getPowerUp( doublePair inPos );
        


        ColorScheme getLevelColors();
        
        NoteSequence getLevelNoteSequence();
        
        
        // 0 player
        // 1 enemy
        ColorScheme getEnteringPointColors( doublePair inPosition,
                                            itemType inType );

        RandomWalkerSet getEnteringPointWalkerSet( doublePair inPosition,
                                                   itemType inType );
        
        NoteSequence getEnteringPointNoteSequence( doublePair inPosition,
                                                   itemType inType );

        // level number of subLevel if entered here
        int getEnteringPointSubLevel( doublePair inPosition,
                                      itemType inType );
        
        
        int getLevelNumber();
        

        BorderSprite *getLastEnterPointSprite();
        
        PowerUpSet *getLastEnterPointPowers();


        PlayerSprite *getPlayerSprite();
        
        PowerUpSet *getPlayerPowers();
        
        NoteSequence *getPlayerNoteSequence();
        

        // copied internally
        void setPlayerPowers( PowerUpSet *inPowers );
        

        
        void getPlayerHealth( int *outValue, int *outMax );
        void restorePlayerHealth();
        
        // jitter after player hit
        doublePair getPlayerHealthBarJitter();
        



        char isRiseSpot( doublePair inPos );
        

        doublePair getEnemyCenter( int inEnemyIndex );
        doublePair getPowerUpCenter( int inPowerUpIndex );
        
        int getEnemyDifficultyLevel( int inEnemyIndex );
        


        // freeze level step updates during drawLevel
        void freezeLevel( char inFrozen );
        
        char isFrozen();
        

        // brief period of immortality
        void startPlayerImmortal();
        


        void drawFloorEdges( char inDraw );
        
        
        doublePair stopMoveWithWall( doublePair inStart,
                                     doublePair inMoveDelta );
        

        void addBullet( doublePair inPosition,
                        doublePair inAimPosition,
                        PowerUpSet *inPowers,
                        doublePair inHeatSeekWaypoint,
                        double inSpeed, char inPlayerBullet,
                        char inEnemyBulletMarker = -1 );


        void pushAllMusicIntoPlayer();
        
        

        char isInsideEnemy();
        
        char isInsidePlayer();
        
        char isKnockDown();
        

        int getTokenRecursionDepth();
        
        int getFloorTokenLevel();

        int getDifficultyLevel();
        

    protected:
        
        
        void step( doublePair inViewCenter, double inViewSize );
        
        void drawPlayer( double inFade );
        void drawMouse( double inFade );
        void drawSmoke( double inFade,
                        doublePair inVisStart, doublePair inVisEnd );        

        void drawGlowTrails( double inFade, 
                             doublePair inVisStart, doublePair inVisEnd );
        

        // inLayer is either 0 or 1 (those below or above item window)
        // inLayer has no effect if no current item window is set
        void drawEnemies( double inFade, int inLayer,
                          doublePair inVisStart, doublePair inVisEnd );
        

        GridPos getGridPos( doublePair inWorldPos );

        // if outNumStepsToGoal pointer provided, call stops after
        // this is computed
        // full path destroyed by caller if requested and result not NULL
        // full path contains ( outNumStepsToGoal + 1 ) positions in it
        GridPos pathFind( GridPos inStart, doublePair inStartWorld,
                          GridPos inGoal,
                          double inMoveSpeed, int *outNumStepsToGoal = NULL,
                          GridPos **outFullPath = NULL );
        
        void generateEnemyDestructionSmoke( Enemy *inE );
        

        // find cut vertices in floor using Hopcroft and Tarjan's 1973 alg
        void findCutPointsRecursive( int inCurrentIndex,
                                     int *inDepths,
                                     int *inLowpoints, int inXLowerLimit );
        

        // must lock audio around this call
        void setLoudnessForAllParts();
        


        // generate data that can be reproduced from the seed
        void generateReproducibleData();
        
        // free up memory consumed by reproducible data
        void freeReproducibleData();
        
        char mDataGenerated;
        

        // random generator state that generated this level
        unsigned int mRandSeedState;
        
        
        int mLevelNumber;

        char mSymmetrical;

        char mInsideEnemy;
        
        char mInsidePowerUp;

        char mKnockDown;
        

        int mTokenRecursionDepth;
        
        int mFloorTokenLevel;
        
        int mDifficultyLevel;


        // background notes for this level
        NoteSequence mHarmonyNotes;        

        // drum beat associated with rise marker
        DrumSequence mRiseDrumBeat;


        // dynamically allocate these to make them compactable

        // need these for quick wall collision detection
        //char mWallFlags[MAX_LEVEL_H][MAX_LEVEL_W];
        // 0 = empty, 1 = floor, 2 = wall
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

        // maps floor indices to flags, where true means that floor
        // spot is a cut vertex that disconnects two parts of the graph
        char *mCutVertexFloorFlags;
        
        
        
        // to speed-up drawing of zoom-in,
        // when map doesn't have to be interactive anyway
        SpriteHandle mFullMapSprite;

        // same trick for shadows along wall edges
        SpriteHandle mFullMapWallShadowSprite;
        

        // static to save RAM
        // maps each grid spot to world coordinates
        static char sGridWorldSpotsComputed;
        static doublePair sGridWorldSpots[MAX_LEVEL_H][MAX_LEVEL_W];


        SimpleVector<Bullet> mBullets;
        SimpleVector<HitSmoke> mSmokeClouds;
        
        SimpleVector<Enemy> mEnemies;
        
        SimpleVector<PowerUpToken> mPowerUpTokens;
        
        SimpleVector<BloodStain> mBloodStains;
        
        SimpleVector<GlowSpriteTrail> mGlowTrails;

        
        SimpleVector<RiseMarkerPathStep> mRiseMarkerPathSteps;
        float mRiseMarkerPathStepFadeProgress;


        GridPos mRisePosition, mRisePosition2;
        doublePair mRiseWorldPos, mRiseWorldPos2;
        char mDoubleRisePositions;
        
        char mFrozen;
        
        int mPlayerImmortalSteps;
        

        char mDrawFloorEdges;
        float mEdgeFadeIn;
        

        float mLastComputedEdgeFade;
        float mLastComputedFastWindowFade;
        

        char mWindowSet;
        WindowPosition mWindowPosition;

        doublePair mMousePos;
        char mEnteringMouse;
        doublePair mPlayerStartPos;
        doublePair mPlayerPos;
        doublePair mPlayerVelocity;


        int mStartStepsToRiseMarker;
        int mLastComputedStepsToRiseMarker;
        int getStepsToRiseMarker( doublePair inPos );
        
        int mLastCloseStepsToRiseMarker;
        char mGettingFartherAwayFromRiseMarker;

        

        int mPlayerStepsUntilNextGlowTrail;
        

        PlayerSprite mPlayerSprite;
        PowerUpSet *mPlayerPowers;
        RandomWalkerSet mPlayerWalkerSet;
        NoteSequence mPlayerMusicNotes;
        
        int mPlayerHealth;
        
        char mPlayerHealthBarJittering;
        double mPlayerHealthBarJitterProgress;


        ColorScheme mColors;
        
        RandomWalkerSet mWalkerSet;
        

        BorderSprite *mLastEnterPointSprite;
        PowerUpSet *mLastEnterPointPowers;
        
        // -1 if last enter point was not a power token
        int mLastEnterPointPowerTokenIndex;
        

        // only update path finding for one enemy per timestep 
        // (avoid slowdown)
        int mNextEnemyPathFindIndex;
        

    };

        


#endif
