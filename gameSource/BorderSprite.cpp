#include "BorderSprite.h"



BorderSprite::BorderSprite()
        : mBorderSprite( NULL ), mCenterSprite( NULL ) {
    }



BorderSprite::~BorderSprite() {
    if( mBorderSprite != NULL ) {
        freeSprite( mBorderSprite );
        }
    if( mCenterSprite != NULL ) {
        freeSprite( mCenterSprite );
        }
    }

static double scaleFactor = 1.0 / 16;


        
void BorderSprite::drawBorder( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( mBorderSprite, inPosition, scaleFactor );
    }


void BorderSprite::drawCenter( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 0, 1, inFade );
    drawSprite( mCenterSprite, inPosition, scaleFactor );
    }


void BorderSprite::draw( doublePair inPosition, double inFade ) {
    drawBorder( inPosition, inFade );
    drawCenter( inPosition, inFade );
    }


