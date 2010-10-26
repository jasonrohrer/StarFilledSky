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
    if( ! inPower.behavior && inPower.powerType != powerUpEmpty ) {

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
        PowerUp p = { powerUpEmpty, 0, false };
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
        
        PowerUp p = { powerUpType, level, false };
        
        return p;
        }        
    }




void PowerUpSet::fillDefaultSet() {
    mPushing = false;
    
    mPushStack = NULL;
    mPushStackSize = 0;
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i].powerType = powerUpEmpty;
        mPowers[i].level = 0;
        }
    }



void PowerUpSet::fillRandomSet( int inTotalLevel ) {
    
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




PowerUpSet::PowerUpSet() {
    fillDefaultSet();
    }



PowerUpSet::PowerUpSet( int inTotalLevel, char inIsEnemy ) {
    fillDefaultSet();
    
    fillRandomSet( inTotalLevel );

    if( inIsEnemy ) {
        // stick a follow in one spot
        mPowers[ 0 ].powerType = enemyBehaviorFollow;
        mPowers[ 0 ].behavior = true;
        }
    
    }



PowerUpSet::PowerUpSet( int inTotalLevel, spriteID inType ) {
    fillDefaultSet();
    
    // fill with random first

    fillRandomSet( inTotalLevel );
    

    // spread inType tokens evenly throughout set
    int typeSum = 0;
        
    for( int i=POWER_SET_SIZE-1; i>=0; i-- ) {
        
        if( mPowers[i].powerType == powerUpEmpty ) {
            // use any empty slots too, but only if needed
            }
        else {
            // change all non-empty tokens to inType
            mPowers[i].powerType = inType;
            typeSum += mPowers[i].level;
            }
        }
    if( typeSum < inTotalLevel ) {
        int extra = inTotalLevel - typeSum;
        
        
        int extraPerSlot = extra / POWER_SET_SIZE;
        int extraLastSlot = extra % POWER_SET_SIZE;
            
        for( int i=POWER_SET_SIZE-1; i>=0; i-- ) {
            
            // only change empty tokens if we have level to add to them
            // (so we don't end up with level-0 inType tokens)
            if( mPowers[i].powerType != powerUpEmpty 
                ||
                extraPerSlot > 0 ) {
                
                mPowers[i].powerType = inType;
                mPowers[i].level += extraPerSlot;
                }

            if( i == 0 ) {
                if( extraLastSlot > 0 ) {
                    mPowers[i].powerType = inType;
                    mPowers[i].level += extraLastSlot;
                    }
                }
            }
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
    mPushStackSize = 0;
    
    }




    

void PowerUpSet::pushPower( PowerUp inPower, doublePair inPowerPos ) {

    if( !mPushing ) {    
        mPushing = true;
        mPushProgress = 0;
        mPowerToPush = inPower;
        mPushStartPos = inPowerPos;
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
        p->pushStartPos = inPowerPos;
        p->next = NULL;
        

        *nextInStack = p;        
        mPushStackSize++;
        }
    }




void PowerUpSet::drawSet( doublePair inPosition, float inFade ) {
    int centerIndex = POWER_SET_CENTERED_INDEX;
    
    double slotSize = 1.125;

    // draw slots first, under everything
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = inPosition;
        drawPos.x += ( i - centerIndex ) * slotSize;
        
        setDrawColor( 1, 1, 1, inFade );
        drawSprite( powerUpSlot, drawPos );
        } 



    doublePair slotContentsPos = inPosition;
    
    if( mPushing ) {        

        doublePair destPos = inPosition;
        // push into last slot
        destPos.x += slotSize;
        
        doublePair curPos;

        curPos.x = destPos.x * mPushProgress + 
            mPushStartPos.x * (1-mPushProgress);
        
        curPos.y = destPos.y * mPushProgress + 
            mPushStartPos.y * (1-mPushProgress);
        
        // start faded out so it doesn't pop in over player
        double fadeFactor = inFade;
        if( mPushProgress < 0.2 ) {
            fadeFactor *= mPushProgress / 0.2;
            }
        
        drawPowerUp( mPowerToPush, curPos, fadeFactor );


        

        // scoot rest over
        slotContentsPos.x -= mPushProgress * slotSize;        
        }
    

    // draw existing slot contents
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        doublePair drawPos = slotContentsPos;
        drawPos.x += ( i - centerIndex ) * slotSize;

        double fadeFactor = inFade;
        if( mPushing && i == 0 ) {
            // fade discared one out
            fadeFactor *= 1 - mPushProgress;
            // move it differently
            drawPos = inPosition;
            drawPos.x += ( i - centerIndex ) * slotSize;
            drawPos.y -= mPushProgress;
            }
        
        drawPowerUp( mPowers[i], drawPos, fadeFactor );
        } 

    

    if( mPushing ) {
        // speed up as stack of waiting power-ups gets taller
        mPushProgress += 0.05 + 0.05 * mPushStackSize;
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
                mPushStartPos = mPushStack->pushStartPos;

                powerPushRecord *recordToDiscard = mPushStack;
                
                mPushStack = mPushStack->next;
                mPushStackSize --;
                
                delete recordToDiscard;
                }
            }
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



spriteID PowerUpSet::getMajorityType() {
    #define NUM_POWER_TYPES  lastPowerUpID - firstPowerUpID + 1
    
    int sums[ NUM_POWER_TYPES ];
    for( int i=0; i<NUM_POWER_TYPES; i++ ) {
        sums[i] = 0;
        }
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType != powerUpEmpty ) {
            sums[ mPowers[i].powerType - firstPowerUpID ] += mPowers[i].level;
            }
        }

    int maxType = powerUpEmpty;
    int maxSum = 0;
    
    for( int i=0; i<NUM_POWER_TYPES; i++ ) {
        if( sums[i] > maxSum ) {
            maxSum = sums[i];
            maxType = i + firstPowerUpID;
            }
        }
    
    return (spriteID)maxType;
    }
