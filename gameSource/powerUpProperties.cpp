#include "powerUpProperties.h"
#include "bulletSizeSet.h"


int getMaxHealth( PowerUpSet *inSet ) {
    int max = 0;
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == powerUpHeart ) {
            max += inSet->mPowers[i].level;
            }
        }
    return max;
    }




// determines steepness of bullet size curve (higher makes a flatter curve) 
static float bulletParam = 20;



float getBulletSize( PowerUpSet *inSet ) {
    int totalLevel = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == powerUpBulletSize ) {
            totalLevel += inSet->mPowers[i].level;
            }
        }

    // first bound to 0:1
    float boundedSize = totalLevel / ( totalLevel + bulletParam );

    // bound to 1:10
    boundedSize *= ( maxBulletSize - 1 );
    
    boundedSize += 1;
    
    return boundedSize;
    }


float bulletStepParam = 5;



int getStepsBetweenBullets( PowerUpSet *inSet ) {
    int totalLevel = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == powerUpRapidFire ) {
            totalLevel += inSet->mPowers[i].level;
            }
        }

    // max = 20, min = 2
    return (int)( 20 - 18 * totalLevel / ( totalLevel + bulletStepParam ) );
    }



float getBulletSpeed( PowerUpSet *inSet ) {
    int totalLevel = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == powerUpBulletSpeed ) {
            totalLevel += inSet->mPowers[i].level;
            }
        }

    // first bound to 0:1
    float boundedSpeed = totalLevel / ( totalLevel + bulletParam );

    // bound to 0.3 : 1.0
    boundedSpeed *= ( 0.7 );
    
    boundedSpeed += 0.3;
    
    return boundedSpeed;
    }

