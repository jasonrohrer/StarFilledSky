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


#define POWER_SET_SIZE 3
#define POWER_SET_CENTERED_INDEX 1

class PowerUpSet {
        
    public:
        
        // constructs a random set with sum level at max inTotalLevel
        PowerUpSet( int inTotalLevel );


        void pushPower( PowerUp inPower );
        

        void drawSet( doublePair inPosition );
        


        PowerUp mPowers[ POWER_SET_SIZE ];

    protected:
        
        
    };

