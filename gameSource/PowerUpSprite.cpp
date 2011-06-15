#include "PowerUpSprite.h"
#include "PowerUpSet.h"



static ColorScheme getScheme( spriteID inPowerType ) {
    
    switch( inPowerType ) {
        case powerUpEmpty: {
            ColorScheme c( 0.125f, 0.125f );
            return c;
            }
        case powerUpHeatSeek:
        case powerUpHeart: {
            ColorScheme c( 0.0f, 0.125f );
            return c;
            }
        // most power ups have same scheme
        default: {
            ColorScheme c( 0.625f, 0.125f );
            return c;
            }
        }
    }

    


PowerUpSprite::PowerUpSprite( PowerUp inPower, PowerUpSet *inSubPowers )
        : mKeepUpdated( false ), mPower( inPower ), mSubPowers( inSubPowers ),
          mScheme( getScheme( inPower.powerType ) ) {
    

    }

        

void PowerUpSprite::drawBorder( doublePair inPosition, double inFade ) {
    drawPowerUpBorder( inPosition, inFade );
    }


void PowerUpSprite::drawCenter( doublePair inPosition, double inFade ) {
    mPower.level = mSubPowers->getLevelSum( mPower.powerType );

    drawPowerUpCenter( mPower, inPosition, inFade );
    }

        
void PowerUpSprite::draw( doublePair inPosition, double inFade ) {
    if( mKeepUpdated ) {    
        mPower.level = mSubPowers->getLevelSum( mPower.powerType );
        mPower.powerType = mSubPowers->getMajorityType();
        }
    
    drawPowerUp( mPower, inPosition, inFade );
    }



ColorScheme PowerUpSprite::getColors() {
    return mScheme;
    }
