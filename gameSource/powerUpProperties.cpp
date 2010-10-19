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
    int size = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].powerType == powerUpBulletSize ) {
            size += inSet->mPowers[i].level;
            }
        }

    // first bound to 0:1
    float boundedSize = size / ( size + bulletParam );

    // bound to 1:10
    boundedSize *= ( maxBulletSize - 1 );
    
    boundedSize += 1;
    
    return boundedSize;
    }
