#include "powerUpProperties.h"
#include "bulletSizeSet.h"


static int getTotalLevel( PowerUpSet *inSet, 
                          spriteID inPowerType ) {
    
    int totalLevel = 0;
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == inPowerType ) {
            totalLevel += inSet->mPowers[i].level;
            }
        }
    return totalLevel;
    }




int getMaxHealth( PowerUpSet *inSet ) {
    return getTotalLevel( inSet, powerUpHeart );
    }




// determines steepness of bullet size curve (higher makes a flatter curve) 
static float bulletParam = 20;



float getBulletSize( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletSize );

    // first bound to 0:1
    float boundedSize = totalLevel / ( totalLevel + bulletParam );

    // bound to 1:10
    boundedSize *= ( maxBulletSize - 1 );
    
    boundedSize += 1;
    
    return boundedSize;
    }


float bulletStepParam = 5;



int getStepsBetweenBullets( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpRapidFire );

    // max = 20, min = 2
    return (int)( 20 - 18 * totalLevel / ( totalLevel + bulletStepParam ) );
    }



float getBulletSpeed( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletSpeed );

    // first bound to 0:1
    float boundedSpeed = totalLevel / ( totalLevel + bulletParam );

    // bound to 0.3 : 0.9
    boundedSpeed *= ( 0.6 );
    
    boundedSpeed += 0.3;
    
    return boundedSpeed;
    }



float accuracyParam = 2;


float getAccuracy( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpAccuracy );

    // first bound to 0:1
    float boundedAccuracy = 1 - totalLevel / ( totalLevel + accuracyParam );

    // bound to 0:1.5
    boundedAccuracy *= 1.5;
    
    
    return boundedAccuracy;
    }

