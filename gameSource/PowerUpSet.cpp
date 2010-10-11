#include "PowerUpSet.h"
#include "minorGems/game/gameGraphics.h"



void drawPowerUpBorder( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( powerUpBorder, inPosition );
    }

    

void drawPowerUpCenter( PowerUp inPower,
                        doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( inPower.powerType, inPosition );
    }



void drawPowerUp( PowerUp inPower,
                  doublePair inPosition, double inFade ) {

    setDrawColor( 1, 1, 1, inFade );
    drawSprite( powerUpBorder, inPosition );
    drawSprite( inPower.powerType, inPosition );
    }



PowerUpSet::PowerUpSet() {
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i].powerType = powerUpEmpty;
        mPowers[i].level = 0;
        }
    }



PowerUpSet::PowerUpSet( int inTotalLevel ) {
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i].powerType = powerUpEmpty;
        mPowers[i].level = 0;
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
        
        drawSprite( powerUpSlot, drawPos );

        drawPowerUp( mPowers[i], drawPos );
        } 
    }


