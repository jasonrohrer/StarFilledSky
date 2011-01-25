#include "tutorial.h"

#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"

#include "Font.h"
#include "drawUtils.h"


#include <math.h>



extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;

extern Font *mainFont2;

extern char *tutorialMoveKeys;



#define numTut 8

static const char *tutorialKeys[ numTut ] = 
{ "tutorial_move", "tutorial_shoot", "tutorial_structure", 
  "tutorial_tokens", "tutorial_useTokens", 
  "tutorial_enter1", "tutorial_enter2", "tutorial_gather" };

#define numEnterTut 3

static const char *tutorialEnterKeys[ numEnterTut ] =
{ "tutorial_enter2_self", "tutorial_enter2_enemy", "tutorial_enter2_token" };

static const char *tutorialGatherKeys[ numEnterTut ] =
{ "tutorial_gather_self", "tutorial_gather_enemy", "tutorial_gather_token" };

int currentEnter2TutorialType = -1;


// init all to false
static char tutorialsDone[ numTut ] = { false, false, false, false,
                                        false, false, false, false };

// init first to true, rest to false
static char tutorialsReady[ numTut ] = { true, false, false, false,
                                         false, false, false, false };

static char shouldSkipTutorial6 = false;
static char showOneMoreGatherTutorial = false;

static const char *nextGatherKey = NULL;



int finalStepFrameCount = 0;


static char moveKeysPressed[ 4 ] = { false, false, false, false };

static char enteredTypes[ 3 ] =  { false, false, false };


static double tutorialOffset = 0;
static double tutorialFade = 0;

static int currentTut = -1;

static int tutorialCompletedCount = 0;


static char *modifiedMoveTutorial = NULL;

static char rigPowerUpsForTeaching = true;

static char blockEnterForTeaching = true;


static char forceFreshStart = false;
static char forceEnd = false;


static char tutorialBriefMode = false;


void forceTutorialEnd() {
    forceEnd = true;
    
    /*
    */
    }



void forceTutorialFreshStart() {
    forceFreshStart = true;
    /*
    resetTutorial();
    
    // full fresh start
    for( int i=0; i<numTut; i++ ) {
        tutorialsReady[i] = false;
        }
    */    
    }


void checkTutorial() {
    currentTut = 0;

    char countFound = false;
    int readCount = SettingsManager::getIntSetting( "tutorialCompletedCount", 
                                                    &countFound );
    
    if( countFound && readCount > 0 ) {
        tutorialCompletedCount = readCount;
        }

    if( ( tutorialCompletedCount >= 1 && ! forceFreshStart )
        ||
        forceEnd ) {

        currentTut = -1;
        rigPowerUpsForTeaching = false;
        blockEnterForTeaching = false;
        
        // don't space out tutorial across levels if player
        // asks for tutorial to be replayed later
        
        // just show one tutorial after another
        for( int i=0; i<numTut; i++ ) {
            tutorialsReady[i] = true;
            }        
        }
    }



void initTutorial() {

    const char *tutMessage = translate( tutorialKeys[ 0 ] );
    
    char found;
    modifiedMoveTutorial = replaceOnce( tutMessage, "W A S D",
                                        tutorialMoveKeys,
                                        &found );
    checkTutorial();
    }




void freeTutorial() {
    if( modifiedMoveTutorial != NULL ) {
        delete [] modifiedMoveTutorial;
        modifiedMoveTutorial = NULL;
        }
    }


char shouldPowerUpsBeRigged() {
    return rigPowerUpsForTeaching;
    }


char shouldEnterBeBlocked() {
    return blockEnterForTeaching;
    }



void resetTutorial() {
    if( currentTut == -1 ) {
        // not running;
        
        // skip all but control tutorials
        tutorialBriefMode = true;
        
        for( int i=0; i<numTut; i++ ) {
            tutorialsDone[i] = true;
            tutorialsReady[i] = true;
            }

        
        tutorialsDone[0] = false;
        tutorialsDone[1] = false;
        tutorialsDone[5] = false;
        
        
        

        finalStepFrameCount = 0;


        for( int i=0; i<4; i++ ) {
            moveKeysPressed[i] = false;
            }
        
        // skip these tutorials
        for( int i=0; i<3; i++ ) {
            enteredTypes[i] = true;
            }

        tutorialOffset = 0;
        tutorialFade = 0;

        currentTut = 0;
        }
    }




static void drawBracket( doublePair inPos, float inFade,
                         const char *inLabelTransKey ) {

    setDrawColor( 1, 1, 1, 0.5 * inFade );
    drawSprite( bracket,  inPos );
    

    if( strcmp( inLabelTransKey, "" ) != 0 ) {
        
        doublePair markerPos = inPos;
        markerPos.y -= 0.625;
        
        // one sub-pixel tweak
        markerPos.x += 0.03125;
        

        doublePair markerShadowPos = markerPos;
    
        markerShadowPos.x += 0.0625;
        markerShadowPos.y -= 0.0625;
    
        setDrawColor( 0, 0, 0, 0.5 * inFade );
    
        const char *markerString = translate( inLabelTransKey );
    
        if( false )mainFont2->drawString( markerString,
                               markerShadowPos, alignCenter );
    

        setDrawColor( 1, 1, 1, 0.5 * inFade );
    
        mainFont2->drawString( markerString,
                               markerPos, alignCenter );
        }
    
    }




void drawTutorial( doublePair inScreenCenter ) {


    if( currentTut != -1 && tutorialsReady[ currentTut ] ) {
        
        // tutorial text
        const char *tutMessage = translate( tutorialKeys[ currentTut ] );
    
        if( currentTut == 0 ) {
            // override
            tutMessage = modifiedMoveTutorial;
            }
        

        double offsetLimit = viewWidth * viewHeightFraction / 2 - 0.5;


        doublePair tutorialPos = inScreenCenter;
        double sineSmooth = 0.5 * sin( ( tutorialOffset - 0.5 ) * M_PI ) + 0.5;
        
        tutorialPos.y -= sineSmooth * offsetLimit;

        double messageWidth = mainFont2->measureString( tutMessage );
    
        setDrawColor( 0, 0, 0, 0.5 * tutorialFade );
        drawRect( tutorialPos.x - messageWidth / 2 - 0.25, 
                  tutorialPos.y - 0.4, 
                  tutorialPos.x + messageWidth / 2 + 0.25, 
                  tutorialPos.y + 0.5 );
        setDrawColor( 1, 1, 1, tutorialFade );
        mainFont2->drawString( tutMessage, 
                               tutorialPos, alignCenter );

        if( currentTut == 2 || currentTut == 3 || currentTut == 4 ||
            currentTut == 7 ) {
            // draw brackets
            
            doublePair bracketPos = inScreenCenter;
            bracketPos.y += 6.1875;
            bracketPos.x -= 0.4375;
            
            if( currentTut == 2 || currentTut == 4 ) {
                const char *firstBracketMarker = "";

                if( currentTut == 2 ) {
                    firstBracketMarker = "tutorial_youMarker";
                    }
                
                drawBracket( bracketPos, tutorialFade, firstBracketMarker );
                }
                        
            
            if( currentTut == 2 || currentTut == 3 || currentTut == 7 ) {
                
                // second bracket for 2, 3 and 7
                bracketPos.x -= 6.25;
                
                const char *secondBracketMarker = "";
                
                if( currentTut == 2 ) {
                    // only mark it in 2
                    secondBracketMarker = "tutorial_insideMarker";
                    }

                drawBracket( bracketPos, tutorialFade, secondBracketMarker );
                }
            
            }

        

        if( tutorialOffset < 1 ) {
        
            tutorialFade += 0.01 * frameRateFactor;
    
            if( tutorialFade > 1 ) {
                tutorialFade = 1;
                }
            }
        else if( tutorialsDone[ currentTut ] ) {
            
            tutorialFade -= 0.01 * frameRateFactor;
        
            if( tutorialFade < 0 ) {
                tutorialFade = 0;

                // move on
                // skip done ones
                while( currentTut < numTut && 
                       tutorialsDone[ currentTut ] ) {
                    currentTut ++;
                    }
                printf( "Moving on to tut %d\n", currentTut );
                
                if( currentTut < numTut ) {
                    
                    tutorialFade = 0;
                    tutorialOffset = 0;
                    }
                else {
                    // just finished entering2

                    // check if we're really, really done with it
                    
                    if( enteredTypes[0] && enteredTypes[1] &&
                        enteredTypes[2] &&
                        ! showOneMoreGatherTutorial ) {
                        // really done
                        
                        currentTut = -1;
                        
                        tutorialCompletedCount ++;
                    
                        SettingsManager::setSetting( "tutorialCompletedCount",
                                                     tutorialCompletedCount );
                        }
                    else {
                        // still some left to enter
                        currentTut = 6;
                        //tutorialsReady[6] = false;
                        tutorialsDone[6] = shouldSkipTutorial6;
                        
                
                        tutorialsDone[7] = false;
                        //tutorialsReady[7] = true;
                        tutorialsReady[7] = showOneMoreGatherTutorial;
                        
                        tutorialKeys[7] = nextGatherKey;

                        if( shouldSkipTutorial6 ) {
                            showOneMoreGatherTutorial = false;
                            currentTut = 7;
                            }


                        tutorialFade = 0;
                        tutorialOffset = 0;
                        }
                    }
                }
            }
    
    
        if( tutorialFade == 1 ) {
        
            tutorialOffset += 0.0125 * frameRateFactor;
        
            if( tutorialOffset > 1 ) {
                tutorialOffset = 1;
                }
            }
        }
    

    /*
    // show final until 2 things entered, or until enough time passes
    if( currentTut == numTut - 1 ) {
        finalStepFrameCount ++;
    
        if( finalStepFrameCount > 600 / frameRateFactor ) {
            tutorialsDone[ currentTut ] = true;
            }
        }
    */

    }





// report movement keys pressed
void tutorialKeyPressed( int inKeyNum ) {
    if( ! tutorialsDone[ 0 ] ) {
        
        moveKeysPressed[ inKeyNum ] = true;
        
        int allPressed = true;
        for( int i=0; i<4; i++ ) {
            allPressed = allPressed & moveKeysPressed[i];
            }
        if( allPressed ) {
            tutorialsDone[0] = true;
            }
        }
    }


// start at level 0, so it is already visited
static char levelVisited[7] = { true, false, false,
                                false, false, false };
                                


void tutorialRiseHappened( int inLevelRisenTo ) {

    if( tutorialBriefMode ) {
        return;
        }
    

    if( inLevelRisenTo >=0 && inLevelRisenTo < 7 ) {
        levelVisited[ inLevelRisenTo ] = true;
        }


    // force these to end, and the next to start, based on level progression
    // even if required actions (moving NSEW or shooting enemy) not performed
    // by player.
    // Avoid falling behind.
    if( inLevelRisenTo > 0 && inLevelRisenTo < 6 ) {
        tutorialsDone[ inLevelRisenTo - 1 ] = true;
        if( inLevelRisenTo < 5 ) {
            tutorialsReady[ inLevelRisenTo ] = true;
            }
        }
    
    

    switch( inLevelRisenTo ) {
        case 6:
            rigPowerUpsForTeaching = false;
            break;
        case 8:
            tutorialsReady[5] = true;
            blockEnterForTeaching = false;
            break;
        }

    if( tutorialsDone[5] && inLevelRisenTo < 8 ) {
        tutorialsReady[6] = false;
        }
    
    if( tutorialsDone[5] && inLevelRisenTo >= 8 ) {
        // risen out of whatever was entered,
        // or at least in a good spot to other enter options 
        
        if( ! enteredTypes[0] ||
            ! enteredTypes[1] ||
            ! enteredTypes[2] ) {
            
            
            tutorialsReady[6] = true;
            shouldSkipTutorial6 = false;
            tutorialsDone[6] = false;
            
            if( !enteredTypes[0] ) {
                tutorialKeys[6] = tutorialEnterKeys[0];
                currentEnter2TutorialType = player;
                }
            else if( !enteredTypes[1] ) {
                tutorialKeys[6] = tutorialEnterKeys[1];
                currentEnter2TutorialType = enemy;
                }
            else if( !enteredTypes[2] ) {
                tutorialKeys[6] = tutorialEnterKeys[2];
                currentEnter2TutorialType = power;
                }
            }
        else {
            // totally done
            if( currentTut != -1 &&
                !tutorialsReady[currentTut] ) {
                currentTut = -1;

                tutorialCompletedCount ++;
                
                SettingsManager::setSetting( "tutorialCompletedCount",
                                             tutorialCompletedCount );
                }
            
            tutorialsReady[6] = true;
            tutorialsDone[6] = true;
            shouldSkipTutorial6 = true;

            tutorialsReady[7] = true;
            tutorialsDone[7] = true;
            }    
        }

    if( tutorialsReady[7] ) {
        tutorialsDone[7] = true;
        shouldSkipTutorial6 = false;
        }
    

    }



char levelAlreadyVisited( int inLevelNumber ) {
    if( inLevelNumber < 7 ) {
        return levelVisited[ inLevelNumber ];
        }
    else {
        return false;
        }
    }




// report mouse shooting used
void tutorialEnemyHit() {
    tutorialsDone[1] = true;
    }



// report enter function used
void tutorialSomethingEntered( itemType inType ) {
    if( tutorialBriefMode ) {
        tutorialsDone[5] = true;
        }
    

    if( tutorialsDone[4] ) {        
        
        if( tutorialsReady[5] ) {

            tutorialsDone[5] = true;

            if( tutorialsReady[6] && 
                ( currentEnter2TutorialType == -1 ||
                  inType == currentEnter2TutorialType ) ) {
                // entered something else
                
                }

            tutorialsDone[6] = true;
            shouldSkipTutorial6 = false;

            showOneMoreGatherTutorial = false;

            if( tutorialsReady[7] ) {
                // already showing a Gather tutorial
                // end it
                tutorialsDone[7] = true;
                tutorialsReady[6] = false;
                }
            if( !enteredTypes[ inType ] ) {
                // entered something new
                tutorialsReady[7] = true;
                
                shouldSkipTutorial6 = true;

                if( tutorialsDone[7] ) {
                    // wait before changing key
                    nextGatherKey = 
                        tutorialGatherKeys[inType];
                    }
                else {
                    // change key immediately
                    tutorialKeys[7] = 
                        tutorialGatherKeys[inType];
                    }
                
                showOneMoreGatherTutorial = true;
                }

            
            enteredTypes[ (int)inType ] = true;
            }
        }
    }



void tutorialPlayerKnockedDown() {
    if( tutorialBriefMode ) {
        return;
        }
    

    if( tutorialsReady[7] ) {
        // already showing a Gather tutorial
        // end it
        tutorialsDone[7] = true;
        showOneMoreGatherTutorial = false;
        shouldSkipTutorial6 = false;
        tutorialsReady[6] = false;
        }
    
    if( tutorialsReady[4] ) {
        tutorialsDone[4] = true;
        }
    }


