#ifndef POWER_UP_SET_INCLUDED
#define POWER_UP_SET_INCLUDED


#include "minorGems/util/SimpleVector.h"


#include "fixedSpriteBank.h"


typedef struct PowerUp {
        spriteID powerType;
        int level;
    } PowerUp;


void drawPowerUpBorder( doublePair inPosition, double inFade=1 );

void drawPowerUpCenter( PowerUp inPower,
                        doublePair inPosition, double inFade=1 );

void drawPowerUp( PowerUp inPower,
                  doublePair inPosition, double inFade=1 );


PowerUp getRandomPowerUp( int inMaxLevel );




#define POWER_SET_SIZE 3
#define POWER_SET_CENTERED_INDEX 1


typedef struct powerPushRecord {
        
        double pushProgress;
        PowerUp powerToPush;
        doublePair pushStartOffset;
    } powerPushRecord;



class PowerUpSet {
        
    public:
    
        // constructs a level 0 set
        PowerUpSet();
        
    
        // constructs a random set with sum level at most inTotalLevel
        PowerUpSet( int inTotalLevel );


        void pushPower( PowerUp inPower, doublePair inPowerPos );
        

        void drawSet( doublePair inPosition );
        
        
        // sum of powers in set that match a type
        int getLevelSum( spriteID inPowerUpType );
        


        PowerUp mPowers[ POWER_SET_SIZE ];

    protected:
        
        void fillDefaultSet();
        
        doublePair mLastDrawPos;


        // FIXME:  do something with this
        SimpleVector< powerPushRecord > mPushStack;
        
        
        char mPushing;
        double mPushProgress;
        PowerUp mPowerToPush;
        doublePair mPushStartOffset;
        
        
        
    };



#endif
