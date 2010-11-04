#include "powerUpProperties.h"
#include "bulletSizeSet.h"


extern double frameRateFactor;



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

    // bound to 1:maxBulletSize
    boundedSize *= ( maxBulletSize - 1 );
    
    boundedSize += 1;
    
    return boundedSize;
    }


float bulletStepParam = 5;



int getStepsBetweenBullets( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpRapidFire );

    // max = 40, min = 4
    return (int)( 
        ( 40 - 36 * totalLevel / ( totalLevel + bulletStepParam ) ) 
        / frameRateFactor );
    }



float getBulletSpeed( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletSpeed );

    // first bound to 0:1
    float boundedSpeed = totalLevel / ( totalLevel + bulletParam );

    // bound to 0.15 : 0.4
    boundedSpeed *= ( 0.25 );
    
    boundedSpeed += 0.15;
    
    boundedSpeed *= frameRateFactor;

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


float spreadD1 = 0.5;
float spreadD2 = 0.06;



float getSpread( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpSpread );

    // first bound to 0:1
    float boundedSpread = totalLevel / ( totalLevel + bulletParam );

    // bound to 0:10
    boundedSpread *= 10;
    
    return boundedSpread;
    }



float heatSeekParam = 3;


float getHeatSeek( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpHeatSeek );

    // first bound to 0:1
    float boundedHeatSeek = totalLevel / ( totalLevel + heatSeekParam );

    // bound to 0:0.1
    boundedHeatSeek *= 0.1;

    
    return boundedHeatSeek;
    }



float getBulletDistance( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletDistance );

    // first bound to 0:1
    float boundedDistance = totalLevel / ( totalLevel + bulletParam );

    // bound to 5:30
    boundedDistance *= 25;
    
    boundedDistance += 5;
    
    return boundedDistance;
    }


float bounceParam = 5;

int getBounce( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBounce );

    // first bound to 0:1
    float boundedBounce = totalLevel / ( totalLevel + bounceParam );

    // bound to 0:10
    boundedBounce *= 10;
    
    return (int)boundedBounce;
    }



float getExplode( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpExplode );

    // first bound to 0:1
    float boundedExplode = totalLevel / ( totalLevel + bulletParam );

    // bound to 0:5
    boundedExplode *= 10;
    
    return boundedExplode;
    }


