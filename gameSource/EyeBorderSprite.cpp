#include "EyeBorderSprite.h"
#include "fixedSpriteBank.h"

#include <math.h>


extern double frameRateFactor;


EyeBorderSprite::EyeBorderSprite() {
    mEyeOffset.x = 0;
    mEyeOffset.y = 0;
    
    for( int y=0; y<16; y++ ) {
        for( int x=0; x<16; x++ ) {
            mFillMap[y][x] = false;
            }
        }
    
    mSquintTimeLeft = 0;
    }


ColorScheme EyeBorderSprite::getColors() {
    return mColors;
    }



static double scaleFactor = 1.0 / 16;

void EyeBorderSprite::drawCenter( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( mCenterSprite, inPosition, scaleFactor );
    
    setDrawColor( mColors.special.r,
                  mColors.special.g,
                  mColors.special.b, inFade );

    // round to single-pixel move
    doublePair roundedOffset = mult( mEyeOffset, 1 / scaleFactor );
    roundedOffset.x = rint( roundedOffset.x );
    roundedOffset.y = rint( roundedOffset.y );
    roundedOffset = mult( roundedOffset, scaleFactor );
    
    
    doublePair eyePos = add( inPosition, roundedOffset );
    
    if( mSquintTimeLeft > 0 ) {
        drawSprite( eyeSquint, eyePos );
        }
    else {
        drawSprite( eye, eyePos );
        }
    
    }



#define eyeLow 6
#define eyeHigh 9

void EyeBorderSprite::setLookVector( doublePair inLookDir ) {
    doublePair oldEyeOffset = mEyeOffset;
    
    // half-pixel steps to avoid jerkiness caused by round-off errors
    // as we step
    inLookDir = mult( inLookDir, scaleFactor / 2 );
    
    // walk from center out along look dir until we hit edge
    double yD = 0;
    double xD = 0;
    double lastXD = xD;
    double lastYD = yD;

    int y = (int)( (-yD + 0.5) / scaleFactor );
    int x = (int)( (xD + 0.5) / scaleFactor );
    
    while( y >=eyeLow && y <=eyeHigh && x >= eyeLow && x <= eyeHigh &&
           mFillMap[y][x] ) {
        
        lastXD = xD;
        lastYD = yD;
        
        xD += inLookDir.x / 2;
        yD += inLookDir.y / 2;
        
        y = (int)( (-yD + 0.5) / scaleFactor );
        x = (int)( (xD + 0.5) / scaleFactor );
        }
    
    doublePair desiredOffset = { lastXD, lastYD };
    
    doublePair deltaOffset = sub( desiredOffset, mEyeOffset );
    
    double stepSize = scaleFactor * frameRateFactor * 0.25;
    
    if( length( deltaOffset ) >  stepSize ) {
        
        deltaOffset = mult( normalize( deltaOffset ), 
                            stepSize );
    
        mEyeOffset = add( mEyeOffset, deltaOffset );
        }
    else {
        // too close for one more step
        mEyeOffset = desiredOffset;
        }

    mSquintTimeLeft -= 0.01 * frameRateFactor;
    if( mSquintTimeLeft < 0 ) {
        mSquintTimeLeft = 0;
        }
    }


void EyeBorderSprite::startSquint() {
    mSquintTimeLeft = 0.5;
    }
