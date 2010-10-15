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
        default: {
            // make new one (should never happen)
            ColorScheme c;
            
            return c;
            }
        }
    }

    


PowerUpSprite::PowerUpSprite( PowerUp inPower )
        : mPower( inPower ), 
          mColors( getScheme( inPower.powerType ) )  {
    

    }

        

void PowerUpSprite::drawBorder( doublePair inPosition, double inFade ) {
    drawPowerUpBorder( inPosition, inFade );
    }


void PowerUpSprite::drawCenter( doublePair inPosition, double inFade ) {
    drawPowerUpCenter( mPower, inPosition, inFade );
    }

        
void PowerUpSprite::draw( doublePair inPosition, double inFade ) {
    drawPowerUp( mPower, inPosition, inFade );
    }





ColorScheme PowerUpSprite::getColors() {
    return mColors;
    }
