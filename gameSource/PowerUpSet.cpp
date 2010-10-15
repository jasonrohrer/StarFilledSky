#include "PowerUpSet.h"
#include "numerals.h"
#include "minorGems/game/gameGraphics.h"


#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



void drawPowerUpBorder( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( powerUpBorder, inPosition );
    }

    

void drawPowerUpCenter( PowerUp inPower,
                        doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( inPower.powerType, inPosition );
    if( inPower.powerType != powerUpEmpty ) {

        setDrawColor( 0.5, 0.5, 0.5, inFade );
        
        inPosition.x += 0.5 - 0.3125;
        inPosition.y -= 0.5 - 0.25;
        drawNumber( inPower.level, inPosition );
        }
    
    }



void drawPowerUp( PowerUp inPower,
                  doublePair inPosition, double inFade ) {

    setDrawColor( 1, 1, 1, inFade );
    drawSprite( powerUpBorder, inPosition );

    drawPowerUpCenter( inPower, inPosition, inFade );
    }



PowerUp getRandomPowerUp( int inMaxLevel ) {
    if( inMaxLevel <= 0 ) {
        PowerUp p = { powerUpEmpty, 0 };
        return p;
        }
    else {
        int level = randSource.getRandomBoundedInt( 0, inMaxLevel );
                
        spriteID powerUpType = powerUpEmpty;
        
        if( level > 0 ) {
        
            // skip empty if we've gotten here
            powerUpType = (spriteID)
                randSource.getRandomBoundedInt( firstPowerUpID + 1, 
                                                lastPowerUpID );
            }
        
        PowerUp p = { powerUpType, level };
        
        return p;
        }        
    }




void PowerUpSet::fillDefaultSet() {
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i].powerType = powerUpEmpty;
        mPowers[i].level = 0;
        }
    }



PowerUpSet::PowerUpSet() {
    fillDefaultSet();
    }



PowerUpSet::PowerUpSet( int inTotalLevel ) {

    fillDefaultSet();
    

    // fill first FIFO slot first
    for( int i=POWER_SET_SIZE-1; i>=0 && inTotalLevel > 0; i-- ) {
        
        if( inTotalLevel / 3 > 0 ) {
            mPowers[i] = getRandomPowerUp( inTotalLevel / 3 );
            }
        else {
            mPowers[i] = getRandomPowerUp( inTotalLevel );
            }
        
        inTotalLevel -= mPowers[i].level;
        }
    }




    

void PowerUpSet::pushPower( PowerUp inPower ) {
    for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
        mPowers[i] = mPowers[i+1];
        }

    mPowers[ POWER_SET_SIZE - 1 ] = inPower;
    }




void PowerUpSet::drawSet( doublePair inPosition ) {
    int centerIndex = POWER_SET_CENTERED_INDEX;
    
    
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = inPosition;
        drawPos.x += ( i - centerIndex ) * 1.125;
        
        setDrawColor( 1, 1, 1, 1 );
        drawSprite( powerUpSlot, drawPos );

        drawPowerUp( mPowers[i], drawPos );
        } 
    }



int PowerUpSet::getLevelSum( spriteID inPowerUpType ) {
    int sum = 0;

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType == inPowerUpType ) {
            
            sum += mPowers[i].level;
            }
        }
    return sum;
    }


