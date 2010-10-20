#include "PowerUpSprite.h"
#include "PowerUpSet.h"



static ColorScheme getScheme( spriteID inPowerType ) {
    
    switch( inPowerType ) {
        case powerUpEmpty: {
            ColorScheme c( 0.1667f, 0.1667f );
            return c;
            }
        case powerUpHeart: {
            ColorScheme c( 0.0f, 0.1667f );
            return c;
            }
        case powerUpBulletSize:
        case powerUpRapidFire:
        case powerUpBulletSpeed: {
            ColorScheme c( 0.6667f, 0.1667f );
            return c;
            }
        default: {
            // make new one (should never happen)
            ColorScheme c;
            
            return c;
            }
        }
    }

    


PowerUpSprite::PowerUpSprite( PowerUp inPower, PowerUpSet *inSubPowers,
                              char inStartedEmpty )
        : mStartedEmpty( inStartedEmpty ),
          mPower( inPower ), mSubPowers( inSubPowers ),
          mColors( getScheme( inPower.powerType ) )  {
    

    }

        

void PowerUpSprite::drawBorder( doublePair inPosition, double inFade ) {
    drawPowerUpBorder( inPosition, inFade );
    }


void PowerUpSprite::drawCenter( doublePair inPosition, double inFade ) {
    mPower.level = mSubPowers->getLevelSum( mPower.powerType );

    drawPowerUpCenter( mPower, inPosition, inFade );
    }

        
void PowerUpSprite::draw( doublePair inPosition, double inFade ) {
    mPower.level = mSubPowers->getLevelSum( mPower.powerType );

    if( mStartedEmpty ) {
        mPower.powerType = mSubPowers->getMajorityType();
        }
    
    drawPowerUp( mPower, inPosition, inFade );
    }





ColorScheme PowerUpSprite::getColors() {
    return mColors;
    }
