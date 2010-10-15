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
    
    mLastDrawPos.x = 0;
    mLastDrawPos.y = 0;
    
    mPushing = false;
    

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




    

void PowerUpSet::pushPower( PowerUp inPower, doublePair inPowerPos ) {
    
    mPushing = true;
    mPushProgress = 0;
    mPowerToPush = inPower;
    mPushStartOffset = sub( inPowerPos, mLastDrawPos );
    
    /*
    for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
        mPowers[i] = mPowers[i+1];
        }

    mPowers[ POWER_SET_SIZE - 1 ] = inPower;
    */
    }




void PowerUpSet::drawSet( doublePair inPosition ) {
    int centerIndex = POWER_SET_CENTERED_INDEX;
    
    double slotSize = 1.125;

    // draw slots first, under everything
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = inPosition;
        drawPos.x += ( i - centerIndex ) * slotSize;
        
        setDrawColor( 1, 1, 1, 1 );
        drawSprite( powerUpSlot, drawPos );
        } 


    
    
    if( mPushing ) {

        doublePair startPos = add( inPosition, mPushStartOffset );
        

        doublePair destPos = inPosition;
        // push into last slot
        destPos.x += slotSize;
        
        doublePair curPos;

        curPos.x = destPos.x * mPushProgress + 
            startPos.x * (1-mPushProgress);
        
        curPos.y = destPos.y * mPushProgress + 
            startPos.y * (1-mPushProgress);
        
        drawPowerUp( mPowerToPush, curPos );


        

        // scoot rest over
        inPosition.x -= mPushProgress * slotSize;

        mPushProgress += 0.1;
        if( mPushProgress >= 1 ) {
            mPushing = false;

            // actually stick into our last slot and discard first slot
            for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
                mPowers[i] = mPowers[i+1];
                }
            
            mPowers[ POWER_SET_SIZE - 1 ] = mPowerToPush;

            }
        
        }
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = inPosition;
        drawPos.x += ( i - centerIndex ) * slotSize;
        
        setDrawColor( 1, 1, 1, 1 );

        drawPowerUp( mPowers[i], drawPos );
        } 

    
    


    mLastDrawPos = inPosition;

    
    


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


