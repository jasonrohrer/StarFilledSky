#include "powerUpProperties.h"
#include "bulletSizeSet.h"


extern double frameRateFactor;


// used for test-toggling a bullet property level with external (game.cpp)
// keyboard input
int testBulletValue = 0;




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
    int heartTotal = getTotalLevel( inSet, powerUpHeart );

    int numBehaviors = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( inSet->mPowers[i].behavior ) {
            numBehaviors++;
            }
        }    
    
    // extra hearts for each behavior, to make up for fact that they take
    // up a slot
    
    return heartTotal + 2 * numBehaviors;
    }





static float bulletCurve( int inLevel ) {
    
    // linear up to level 10, where it reaches 0.5

    if( inLevel <= 10 ) {
        return inLevel / 20.0f;
        }
    else {
        // a steep curve that meets line at level (10, 0.5)
        // and approaches 1 as inLevel -> infinity

        int adjustedLevel = inLevel - 10;

        return 
            0.5 * 
            adjustedLevel / ( adjustedLevel + 3.0f ) 
            + 0.5;
        }
    }




float getBulletSize( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletSize );

    // first bound to 0:1
    float boundedSize = bulletCurve( totalLevel );

    // bound to 1:maxBulletSize
    boundedSize *= ( maxBulletSize - 1 );
    
    boundedSize += 1;
    
    return boundedSize;
    }


float bulletStepParam = 2;



int getStepsBetweenBullets( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpRapidFire );

    // max = 40, min = 4
    return (int)( 
        ( 40 - 36 * totalLevel / ( totalLevel + bulletStepParam ) ) 
        / frameRateFactor );
    }


float bulletSpeedParam = 2;

float getBulletSpeed( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletSpeed );

    // first bound to 0:1
    float boundedSpeed = totalLevel / ( totalLevel + bulletSpeedParam );

    // bound to 0.15 : 0.4
    boundedSpeed *= ( 0.25 );
    
    boundedSpeed += 0.15;
    
    boundedSpeed *= frameRateFactor;

    return boundedSpeed;
    }



/*
float accuracyParam = 2;


float getAccuracy( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpAccuracy );

    // first bound to 0:1
    float boundedAccuracy = 1 - totalLevel / ( totalLevel + accuracyParam );

    // bound to 0:1.5
    boundedAccuracy *= 1.5;
    
    
    return boundedAccuracy;
    }
*/



float spreadD1 = 1.5;
float spreadD2 = 0.06;


float spreadParam = 15;

float spreadCurveOffset = ( - 1 / (1 + spreadParam) ) + 0.01;


float getSpread( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpSpread );
    
    // skip calculation for level 0 to avoid returning negative, given
    // offset that is used
    if( totalLevel == 0 ) {
        return 0;
        }


    // first bound to 0:1    
    float boundedSpread = totalLevel / ( totalLevel + spreadParam );

    // now push level 1 right above 0
    // to extract most variety out of low-end of curve
    boundedSpread += spreadCurveOffset;

    // bound to 0:10
    boundedSpread *= 10;
    
    return boundedSpread;
    }



float getHeatSeek( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpHeatSeek );
    
    // first bound to 0:1
    float boundedHeatSeek = bulletCurve( totalLevel );

    // bound to 0:0.25
    boundedHeatSeek *= 0.25 * frameRateFactor;
    
    return boundedHeatSeek;
    }



float getBulletDistance( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBulletDistance );

    // first bound to 0:1
    float boundedDistance = bulletCurve( totalLevel );

    // bound to 5:30
    boundedDistance *= 25;
    
    boundedDistance += 5;
    
    return boundedDistance;
    }


float bounceParam = 5;

int getBounce( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpBounce );

    // try unlimited bounce (limited by range, anyway)
    return totalLevel;
    

    // first bound to 0:1
    float boundedBounce = totalLevel / ( totalLevel + bounceParam );

    // bound to 0:10
    boundedBounce *= 10;
    
    return (int)boundedBounce;
    }



float explodeParam = 15;

float explodeCurveOffset = ( - 1 / (1 + explodeParam) ) + 0.02;


float getExplode( PowerUpSet *inSet ) {
    int totalLevel = getTotalLevel( inSet, powerUpExplode );

    
    // skip calculation for level 0 to avoid returning negative, given
    // offset that is used
    if( totalLevel == 0 ) {
        return 0;
        }


    // first bound to 0:1
    //float boundedExplode = bulletCurve( totalLevel );
    
    float boundedExplode = totalLevel / ( totalLevel + explodeParam );

    // now push level 1 right above 0
    // to extract most variety out of low-end of curve
    boundedExplode += explodeCurveOffset;

    // bound to 0:13
    boundedExplode *= 13;
    
    return boundedExplode;
    }


