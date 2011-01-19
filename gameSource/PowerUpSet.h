#ifndef POWER_UP_SET_INCLUDED
#define POWER_UP_SET_INCLUDED


#include "minorGems/util/SimpleVector.h"


#include "fixedSpriteBank.h"


typedef struct PowerUp {
        spriteID powerType;
        int level;
        char behavior;
    } PowerUp;


void drawPowerUpBorder( doublePair inPosition, double inFade=1 );

void drawPowerUpCenter( PowerUp inPower,
                        doublePair inPosition, double inFade=1 );

void drawPowerUp( PowerUp inPower,
                  doublePair inPosition, double inFade=1 );


Color getBlurredColor( PowerUp inPower );


PowerUp getRandomPowerUp( int inMaxLevel );




#define POWER_SET_SIZE 3
#define POWER_SET_CENTERED_INDEX 1


typedef struct powerPushRecord {
        
        double pushProgress;
        PowerUp powerToPush;
        doublePair pushStartPos;
        powerPushRecord *next;
    } powerPushRecord;



class PowerUpSet {
        
    public:
    
        // constructs a level 0 set
        PowerUpSet();
        
        ~PowerUpSet();
        
    
        // constructs a random set with sum level at most inTotalLevel
        PowerUpSet( int inTotalLevel, char inIsEnemy = false );

        // constructs a random set with sum level of EXACTLY inTotalLevel
        // for type inType (and maybe more in other types)
        PowerUpSet( int inTotalLevel, spriteID inType );
        
        // construct by copying an existing set
        PowerUpSet( PowerUpSet *inSetToCopy );
        


        void pushPower( PowerUp inPower, doublePair inPowerPos );
        

        void drawSet( doublePair inPosition, float inFade=1, 
                      char inDrawSlots = true );
        

        // whole-sale replace all 3 powers, displaying a "dropping off" 
        // animation
        // Note that only one drop-off animation at a time is supported
        // inNewSet can be destroyed by caller after call returns
        void dropDownToSet( PowerUpSet *inNewSet );
        

        // do sets contain same powers?
        char equals( PowerUpSet *inOtherSet );
        
        
        // sum of powers in set that match a type
        int getLevelSum( spriteID inPowerUpType );
        
        // gets type of heaviest-total-weight power in set
        spriteID getMajorityType();
        

        PowerUp mPowers[ POWER_SET_SIZE ];
        

        // set to disable animations when drawing
        static char sPauseAllSets;


    protected:
        
        void fillDefaultSet();
        void fillRandomSet( int inTotalLevel, char inIsEnemy = false );


        powerPushRecord *mPushStack;
        int mPushStackSize;
        
        
        char mPushing;
        double mPushProgress;
        PowerUp mPowerToPush;
        doublePair mPushStartPos;
        
        
        char mDropping;
        double mDropProgress;
        PowerUpSet *mPowersBeingDropped;
        
        
    };



#endif
