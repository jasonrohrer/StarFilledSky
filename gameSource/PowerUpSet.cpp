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
    
    mPushStack = NULL;
    

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



PowerUpSet::~PowerUpSet() {


    // clear stack
    powerPushRecord *nextInStack = mPushStack;
        
    while( nextInStack != NULL ) {
        
        powerPushRecord *p = nextInStack;

        nextInStack = nextInStack->next;

        delete p;
        }
    mPushStack = NULL;
    
    }




    

void PowerUpSet::pushPower( PowerUp inPower, doublePair inPowerPos ) {

    if( !mPushing ) {    
        mPushing = true;
        mPushProgress = 0;
        mPowerToPush = inPower;
        mPushStartOffset = sub( inPowerPos, mLastDrawPos );
        }
    else {
        // already pushing, add to stack
        
        powerPushRecord **nextInStack = &( mPushStack );
        
        while( *nextInStack != NULL ) {
        
            nextInStack = &( (*nextInStack)->next );
            }
        
        powerPushRecord *p = new powerPushRecord;
        p->pushProgress = 0;
        p->powerToPush = inPower;
        p->pushStartOffset = sub( inPowerPos, mLastDrawPos );
        p->next = NULL;
        

        *nextInStack = p;        
        }
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



    doublePair slotContentsPos = inPosition;
    
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
        
        // start faded out so it doesn't pop in over player
        double fadeFactor = 1;
        if( mPushProgress < 0.2 ) {
            fadeFactor = mPushProgress / 0.2;
            }
        
        drawPowerUp( mPowerToPush, curPos, fadeFactor );


        

        // scoot rest over
        slotContentsPos.x -= mPushProgress * slotSize;        
        }
    

    // draw existing slot contents
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = slotContentsPos;
        drawPos.x += ( i - centerIndex ) * slotSize;

        double fadeFactor = 1;
        if( mPushing && i == 0 ) {
            // fade discared one out
            fadeFactor = 1 - mPushProgress;
            // move it differently
            drawPos = inPosition;
            drawPos.x += ( i - centerIndex ) * slotSize;
            drawPos.y -= mPushProgress;
            }
        
        drawPowerUp( mPowers[i], drawPos, fadeFactor );
        } 

    

    if( mPushing ) {
        mPushProgress += 0.05;
        if( mPushProgress >= 1 ) {
            mPushing = false;

            // actually stick into our last slot and discard first slot
            for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
                mPowers[i] = mPowers[i+1];
                }
            
            mPowers[ POWER_SET_SIZE - 1 ] = mPowerToPush;

            if( mPushStack != NULL ) {
                // start next from stack
                mPushing = true;
                
                mPushProgress = 0;
                mPowerToPush = mPushStack->powerToPush;
                mPushStartOffset = mPushStack->pushStartOffset;

                powerPushRecord *recordToDiscard = mPushStack;
                
                mPushStack = mPushStack->next;

                delete recordToDiscard;
                }
            }
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


