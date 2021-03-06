#include "PowerUpSet.h"
#include "numerals.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"

#include "setTipDisplay.h"
#include "tutorial.h"


#include "minorGems/util/random/CustomRandomSource.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/log/AppLog.h"


#include <limits.h>



extern CustomRandomSource randSource;

extern double frameRateFactor;



char defaultDrawToggleArray[3] = { true, true, true };


char PowerUpSet::sPauseAllSets = false;



void drawPowerUpBorder( doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( powerUpBorder, inPosition );
    }

    

void drawPowerUpCenter( PowerUp inPower,
                        doublePair inPosition, double inFade ) {
    setDrawColor( 1, 1, 1, inFade );
    drawSprite( inPower.powerType, inPosition );
    if( ! inPower.behavior && inPower.powerType != powerUpEmpty ) {

        
        int levelToDraw = inPower.level;
        
        if( levelToDraw > 999 ) {
            // overflows our 3 digit display!
            
            // display all but most significant three digits as bars on side

            int numDigits = 0;
            
            int leftOver = levelToDraw;
            
            while( leftOver > 0 ) {
                numDigits++;
                leftOver /= 10;
                }

            int numBars = numDigits - 3;
            
            spriteID barSprite = (spriteID)( bar1 + (numBars-1) );
            
            
            drawSprite( barSprite, inPosition );

            int barRepresents = 1;
            for( int i=0; i<numBars; i++ ) {
                barRepresents *= 10;
                }
            // just draw most significant as true digits
            levelToDraw /= barRepresents;
            }
        
        
        setDrawColor( 0.5, 0.5, 0.5, inFade );


        inPosition.x += 0.5 - 0.3125;
        inPosition.y -= 0.5 - 0.3125;
        drawNumber( levelToDraw, inPosition );
        }
    
    }



void drawPowerUp( PowerUp inPower,
                  doublePair inPosition, double inFade ) {

    setDrawColor( 1, 1, 1, inFade );
    if( inPower.behavior ) {
        drawSprite( enemyBehaviorBorder, inPosition );
        }
    else {
        drawSprite( powerUpBorder, inPosition );
        }
    

    drawPowerUpCenter( inPower, inPosition, inFade );
    }



Color getBlurredColor( PowerUp inPower ) {
    return getBlurredColor( inPower.powerType );
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
    mPushProgress = 0;
    
    mPowerToPush.powerType = powerUpEmpty;
    mPowerToPush.level = 0;
    mPowerToPush.behavior = false;
    
    mPushStartPos.x = 0;
    mPushStartPos.y = 0;

    mPushStack = NULL;
    mPushStackSize = 0;
    
    mDropping = false;
    mDropProgress = 0;
    mPowersBeingDropped = NULL;
    
    mDropDrawToggles[0] = true;
    mDropDrawToggles[1] = true;
    mDropDrawToggles[2] = true;
    

    mDroppingHearts = false;
    

    mDimMinority = false;

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i].powerType = powerUpEmpty;
        mPowers[i].behavior = false;
        mPowers[i].level = 0;

        mDimFlags[i] = false;
        }
    }



#define MIN_FOLLOW_LEVEL 3
#define MIN_DODGE_LEVEL 4
#define MIN_RANDOM_LEVEL 3
#define MIN_FAST_LEVEL 5
#define MIN_CIRCLE_LEVEL 3



void PowerUpSet::fillRandomSet( int inTotalLevel, char inIsEnemy,
                                char inAllowFollow ) {

    // ensure enemy behaviors are inserted at most once
    char dodge = false;
    char fast = false;
    
    // enemy move styles block each other
    char moveStyle = false;
    

    // fill first FIFO slot first
    for( int i=POWER_SET_SIZE-1; i>=0 && inTotalLevel > 0; i-- ) {
        
        if( inTotalLevel / POWER_SET_SIZE > 0 ) {
            mPowers[i] = getRandomPowerUp( inTotalLevel / POWER_SET_SIZE );
            }
        else {
            // total level cannot be spread evenly over slots 
            // (smaller than 3, for example)
            // try sticking it in one spot, and treat rest as remainder
            mPowers[i] = getRandomPowerUp( inTotalLevel );
            
            inTotalLevel -= mPowers[i].level;
            }
        


        if( inIsEnemy ) {
        
            // special behaviors?
            
            char behaviorPicked = false;
            
            if( inAllowFollow && !behaviorPicked && ! moveStyle &&
                mPowers[ i ].level >= MIN_FOLLOW_LEVEL ) {
            
                if( randSource.getRandomBoundedInt( 0, 100 ) > 86 ) {
                
                    // stick a follow in this spot
                    mPowers[ i ].powerType = enemyBehaviorFollow;
                    mPowers[ i ].behavior = true;
                    // keep existing level number

                    behaviorPicked = true;
                    moveStyle = true;
                    }
                }

            if( !behaviorPicked && ! dodge &&
                mPowers[ i ].level >= MIN_DODGE_LEVEL ) {
            
                if( randSource.getRandomBoundedInt( 0, 100 ) > 86 ) {
                
                    // stick a dodge in this spot
                    mPowers[ i ].powerType = enemyBehaviorDodge;
                    mPowers[ i ].behavior = true;
                    // keep existing level number
                    
                    behaviorPicked = true;
                    dodge = true;
                    }
                }

            if( !behaviorPicked && ! fast &&
                mPowers[ i ].level >= MIN_FAST_LEVEL ) {
            
                if( randSource.getRandomBoundedInt( 0, 100 ) > 90 ) {
                                
                    // stick a fast in this spot
                    mPowers[ i ].powerType = enemyBehaviorFast;
                    mPowers[ i ].behavior = true;
                    // keep existing level number
                    
                    behaviorPicked = true;
                    fast = true;
                    }
                }

            if( !behaviorPicked && ! moveStyle &&
                mPowers[ i ].level >= MIN_RANDOM_LEVEL ) {
            
                if( randSource.getRandomBoundedInt( 0, 100 ) > 86 ) {
                
                    // stick a random move token in this spot
                    mPowers[ i ].powerType = enemyBehaviorRandom;
                    mPowers[ i ].behavior = true;
                    // keep existing level number

                    behaviorPicked = true;
                    moveStyle = true;
                    }
                }

            if( !behaviorPicked && ! moveStyle &&
                mPowers[ i ].level >= MIN_CIRCLE_LEVEL ) {
            
                if( randSource.getRandomBoundedInt( 0, 100 ) > 86 ) {
                
                    // stick a random move token in this spot
                    mPowers[ i ].powerType = enemyBehaviorCircle;
                    mPowers[ i ].behavior = true;
                    // keep existing level number

                    behaviorPicked = true;
                    moveStyle = true;
                    }
                }

            }
        }
    }




PowerUpSet::PowerUpSet() {
    fillDefaultSet();
    }




PowerUpSet::PowerUpSet( int inTotalLevel, char inIsEnemy, 
                        char inAllowFollow ) {
    fillDefaultSet();
    
    fillRandomSet( inTotalLevel, inIsEnemy, inAllowFollow );
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



PowerUpSet::PowerUpSet( PowerUpSet *inSetToCopy ) {
    fillDefaultSet();

    copySet( inSetToCopy );
    }



PowerUpSet::PowerUpSet( const char *inStringEncoding ) {
    fillDefaultSet();

    int numParts;
    char **results = split( inStringEncoding, "#", &numParts );

    if( numParts != POWER_SET_SIZE ) {
        AppLog::error( 
            "Failed to parse expected number of powers from string encoding" );
        }
    else {
        for( int i=0; i<POWER_SET_SIZE; i++ ) {
        
            char name[100];
            int level;
            
            int numRead = sscanf( results[i], "[%d]%99s", &level, name );
            
            if( numRead != 2 ) {
                AppLog::error( 
                    "Failed to parse power from string encoding" );
                AppLog::error( results[i] );
                }
            else {
                spriteID powerType = mapNameToID( name );
                
                if( powerType != endSpriteID ) {
                    mPowers[i].level = level;
                    mPowers[i].powerType = powerType;
                    }
                else {
                    AppLog::error( 
                        "Unknown power type in string encoding" );
                    
                    mPowers[i].level = 0;
                    mPowers[i].powerType = powerUpEmpty;
                    }                
                }
            }
        
        }
    
    for( int i=0; i<numParts; i++ ) {
        delete [] results[i];
        }
    delete [] results;    
    }

        

char *PowerUpSet::getStringEncoding() {
    char *results[POWER_SET_SIZE];
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        results[i] = autoSprintf( "[%d]%s",
                                  mPowers[i].level,
                                  spriteIDNames[ mPowers[i].powerType ] );
        }

    char *fullSetResult =
        join( results, POWER_SET_SIZE, "#" );

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        delete [] results[i];
        }

    return fullSetResult;
    }





void PowerUpSet::copySet( PowerUpSet *inSetToCopy ) {
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i] = inSetToCopy->mPowers[i];
        }


    // now push any pending powers (including push stack) from the copied set

    if( inSetToCopy->mPushing ) {
        for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
            mPowers[i] = mPowers[i+1];
            }
    
        mPowers[ POWER_SET_SIZE - 1 ] = inSetToCopy->mPowerToPush;
        
    
        powerPushRecord *nextStackRecord = inSetToCopy->mPushStack;

        while( nextStackRecord != NULL ) {
            for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
                mPowers[i] = mPowers[i+1];
                }
    
            mPowers[ POWER_SET_SIZE - 1 ] = nextStackRecord->powerToPush;

        
            nextStackRecord = nextStackRecord->next;
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
    
    if( mPowersBeingDropped != NULL ) {
        delete mPowersBeingDropped;
        mPowersBeingDropped = NULL;
        }

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



int PowerUpSet::knockOffHearts( int inNumToKnock, char inInstant ) {
    
    // what we should be reduced to as a result of this knock
    PowerUpSet newSet;

    // DON'T take pending pushes into account when copying set to make newSet
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        newSet.mPowers[i] = mPowers[i];
        }


    
    int numLeftToKnock = inNumToKnock;
    
    for( int i=0; i<POWER_SET_SIZE && numLeftToKnock > 0; i++ ) {
        
        if( newSet.mPowers[i].powerType == powerUpHeart ) {
            
            if( newSet.mPowers[i].level > numLeftToKnock ) {
                // partially exhausts this heart slot

                newSet.mPowers[i].level -= numLeftToKnock;
                
                numLeftToKnock = 0;
                }
            else {
                // exhausts this heart slot completely

                numLeftToKnock -= newSet.mPowers[i].level;
                
                newSet.mPowers[i].powerType = powerUpEmpty;
                newSet.mPowers[i].level = 0;
                }
            }
        }
    
    if( ! equals( &newSet ) ) {
        // show difference between two sets dropping off

        
        if( ! mDroppingHearts ) {
            

            if( mPowersBeingDropped != NULL ) {
                delete mPowersBeingDropped;
                mPowersBeingDropped = NULL;
                }
        
            mDropping = true;
            mDroppingHearts = true;
            mDropProgress = 0;

            // base is empty set
            mPowersBeingDropped = new PowerUpSet();
            }
        else {
            // already dropping hearts, keep existing drop set and add to it
        
            // don't  update drop progress either
            }
        


        for( int i=0; i<POWER_SET_SIZE; i++ ) {
            
            if( mPowers[i].powerType == powerUpHeart &&
                mPowers[i].level != newSet.mPowers[i].level ) {
                
                // hearts dropping off this slot

                mPowersBeingDropped->mPowers[i].powerType = powerUpHeart;
                
                // add to it (works both for fresh heart drop and augmentation
                // of a heart drop already in progress)
                mPowersBeingDropped->mPowers[i].level += 
                    mPowers[i].level - newSet.mPowers[i].level;
                }
            if( mPowersBeingDropped->mPowers[i].powerType == powerUpHeart ) {
                mDropDrawToggles[i] = true;
                }
            else {
                mDropDrawToggles[i] = false;
                }
            }

        }

    // switch to new set underneath dropping-off tokens, or if drop-off is
    // instant
    // don't use copySet, though, because it screws up a pending push
    // copySet( &newSet );

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i] = newSet.mPowers[i];
        }


    
    return inNumToKnock - numLeftToKnock;
    }


        
char PowerUpSet::knockOffHeart() {
    int numKnocked = knockOffHearts( 1, true );

    return (numKnocked == 1);
    }



void PowerUpSet::dropDownToSet( PowerUpSet *inNewSet ) {
    if( mPowersBeingDropped != NULL ) {
        delete mPowersBeingDropped;
        mPowersBeingDropped = NULL;
        }
    
    mDropping = true;
    mDropProgress = 0;
    
    // copy our powers into the drop animation set
    mPowersBeingDropped = new PowerUpSet( this );


    // skip drawingtokens in places where set is the same as the one
    // we already have

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType == inNewSet->mPowers[i].powerType
            &&
            mPowers[i].behavior == inNewSet->mPowers[i].behavior
            &&
            mPowers[i].level == inNewSet->mPowers[i].level ) {
            
            mDropDrawToggles[i] = false;
            }
        else {
            mDropDrawToggles[i] = true;
            }
        }
    

    // replace our powers with these new ones
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        mPowers[i] = inNewSet->mPowers[i];
        }
    }




void PowerUpSet::sortPowersRight() {
    char foundFlip = true;
    
    while( foundFlip ) {
        foundFlip = false;
        
        for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
            
            if( mPowers[i].level > mPowers[i+1].level 
                ||
                // move hearts to the left of any powers that have equal
                // level to the hearts
                ( mPowers[i].level == mPowers[i+1].level &&
                  mPowers[i].powerType != powerUpHeart &&
                  mPowers[i+1].powerType == powerUpHeart ) ) {
                foundFlip = true;
                
                PowerUp p = mPowers[i];
                
                mPowers[i] = mPowers[i+1];
                
                mPowers[i+1] = p;                
                }
            }        
        }
    }



void decayOnePower( PowerUp *inPower ) {
    // never decay hearts
    if( inPower->powerType == powerUpHeart ) {
        return;
        }
    

    if( inPower->level >= 2 ) {

        // variable decay rate, higher for higher powers
            
        int decayRate = 1;
        
        if( inPower->level > 5 ) {
            decayRate +=
                ( inPower->level - 5 ) / 5;
            }
        

        inPower->level -= decayRate;
        }
    }




void PowerUpSet::decayPowers() {
    
    // skip any empty slots
    int i;
    for( i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType != powerUpEmpty ) {
            break;
            }
        }
    
    if( i < POWER_SET_SIZE ) {
        decayOnePower( &( mPowers[i] ) );
        
        
        // decay a second-weakest power too
        i++;

        if( i < POWER_SET_SIZE ) {
            decayOnePower( &( mPowers[i] ) );
            }
        // else rest of set is empty or hearts
        
        }
    
    // else entire set is empty or hearts

    }



void PowerUpSet::setDimMinority( char inDim ) {
    mDimMinority = inDim;
    
    if( mDimMinority ) {
        recomputeDimFlags();
        }
    }



void PowerUpSet::recomputeDimFlags() {
    spriteID majorityType = getMajorityType();
    
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        char inMinority = ( mPowers[i].powerType != majorityType );
        
        mDimFlags[i] = inMinority;
        }
    }


            
    

char PowerUpSet::equals( PowerUpSet *inOtherSet ) {
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType != inOtherSet->mPowers[i].powerType
            ||
            mPowers[i].behavior != inOtherSet->mPowers[i].behavior
            ||
            mPowers[i].level != inOtherSet->mPowers[i].level ) {
            
            return false;
            }
        }
    // else all match
    return true;
    }




void PowerUpSet::drawSet( doublePair inPosition, float inFade, 
                          char inDrawSlots, const char inDrawToggles[3] ) {
    
    int centerIndex = POWER_SET_CENTERED_INDEX;
    
    double slotSize = 1.125;

    if( inDrawSlots ) {    
        // draw slots first, under everything
        for( int i=0; i<POWER_SET_SIZE; i++ ) {
            doublePair drawPos = inPosition;
            drawPos.x += ( i - centerIndex ) * slotSize;
        
            setDrawColor( 1, 1, 1, inFade );
            
            if( i==0 ) {
                drawSprite( powerUpSlotLeft, drawPos );
                }
            else if( i == POWER_SET_SIZE - 1 ) {
                drawSprite( powerUpSlotRight, drawPos );
                }
            else {
                drawSprite( powerUpSlot, drawPos );
                } 
            }
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
        
        if( ! inDrawToggles[i] ) {
            // skip drawing this power
            continue;
            }

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

        if( mDimMinority && mDimFlags[i] && 
            mPowers[i].powerType != powerUpEmpty ) {

            // leave 71% of color in place, to match darkness of empty power
            setDrawColor( 0, 0, 0, 0.29 * fadeFactor);
        
            // don't cover power border or slot markers
            drawSquare( drawPos, 0.375 );
            }
        
        } 
    

    if( mDropping ) {
        // draw dropped stuff on top, dropping down and fading out
        
        
        inPosition.y -= mDropProgress;
        
        // don't draw slots for the set that's dropping off
        // don't draw empties (which mark slots that are not changing)
        mPowersBeingDropped->drawSet( inPosition, 
                                      inFade * (1 - mDropProgress), 
                                      false,
                                      mDropDrawToggles );
        }
    
    

    if( mPushing && ! sPauseAllSets ) {
        // speed up as stack of waiting power-ups gets taller
        mPushProgress += ( 0.025 + 0.025 * mPushStackSize ) * frameRateFactor;
        if( mPushProgress >= 1 ) {
            mPushing = false;

            // actually stick into our last slot and discard first slot
            for( int i=0; i<POWER_SET_SIZE - 1; i++ ) {
                mPowers[i] = mPowers[i+1];
                }
            
            mPowers[ POWER_SET_SIZE - 1 ] = mPowerToPush;

            if( shouldSetTipsBeShown() ) {    
                // show tip for this new combo
                triggerSetTip( this );
                }
            

            recomputeDimFlags();

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
    
    
    if( mDropping && ! sPauseAllSets ) {
        mDropProgress += 0.025 * frameRateFactor;
        if( mDropProgress >= 1 ) {
            mDropping = false;
            mDroppingHearts = false;
            
            delete mPowersBeingDropped;
            mPowersBeingDropped = NULL;
            }
        }
    

    }



int PowerUpSet::getLevelSum( spriteID inPowerUpType ) {
    int sum = 0;

    // watch for overflow
    char overflow = false;

    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType == inPowerUpType ) {
            
            int tokenLevel = mPowers[i].level;

            if( sum <= INT_MAX - tokenLevel ) {
                sum += tokenLevel;
                }
            else {
                overflow = true;
                }
            }
        }
    
    if( overflow ) {
        sum = INT_MAX;
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
    
    // don't consider all types, in the arbitrary order they occur in 
    // the sums array
    
    // instead, only consider the types in our set, in that order
    // (in case of tie, last power token trumps others)
    for( int i=POWER_SET_SIZE-1; i>=0; i-- ) {
        if( mPowers[i].powerType != powerUpEmpty ) {
            int thisSum = sums[ mPowers[i].powerType - firstPowerUpID ];

            if( thisSum > maxSum ) {
                maxSum = thisSum;
                maxType = mPowers[i].powerType;
                }
            }
        }

    
    return (spriteID)maxType;
    }




char PowerUpSet::containsFollow() {
    for( int i=0; i<POWER_SET_SIZE; i++ ) {
        if( mPowers[i].powerType == enemyBehaviorFollow ) {
            return true;
            }
        }
    
    return false;
    }
