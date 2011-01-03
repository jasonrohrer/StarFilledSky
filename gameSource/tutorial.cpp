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



#define numTut 4

static const char *tutorialKeys[ numTut ] = 
{ "tutorial_move", "tutorial_shoot", "tutorial_enter1", "tutorial_enter2" };

static char tutorialsDone[ numTut ] = { false, false, false, false };

static char tutorialsReady[ numTut ] = { true, false, false, false };



int finalStepFrameCount = 0;


static char moveKeysPressed[ 4 ] = { false, false, false, false };

static char enteredTypes[ 3 ] =  { false, false, false };


static double tutorialOffset = 0;
static double tutorialFade = 0;

static int currentTut = -1;

static int tutorialCompletedCount = 0;


static char *modifiedMoveTutorial = NULL;

static char rigPowerUpsForTeaching = true;



void initTutorial() {

    const char *tutMessage = translate( tutorialKeys[ 0 ] );
    
    char found;
    modifiedMoveTutorial = replaceOnce( tutMessage, "W A S D",
                                        tutorialMoveKeys,
                                        &found );

    currentTut = 0;

    char countFound = false;
    int readCount = SettingsManager::getIntSetting( "tutorialCompletedCount", 
                                                    &countFound );
    
    if( countFound && readCount > 0 ) {
        tutorialCompletedCount = readCount;
        }

    if( tutorialCompletedCount >= 1 ) {
        currentTut = -1;
        rigPowerUpsForTeaching = false;
        
        // don't space out tutorial across levels if player
        // asks for tutorial to be replayed later
        
        // just show one tutorial after another
        for( int i=0; i<numTut; i++ ) {
            tutorialsReady[i] = true;
            }
        }
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



void resetTutorial() {
    if( currentTut == -1 ) {
        // not running;
        

        for( int i=0; i<numTut; i++ ) {
            tutorialsDone[i] = false;
            }
        tutorialsReady[0] = true;


        finalStepFrameCount = 0;


        for( int i=0; i<4; i++ ) {
            moveKeysPressed[i] = false;
            }
        
        for( int i=0; i<3; i++ ) {
            enteredTypes[i] = false;
            }

        tutorialOffset = 0;
        tutorialFade = 0;

        currentTut = 0;
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
                    // totally done
                    currentTut = -1;

                    tutorialCompletedCount ++;
                    
                    SettingsManager::setSetting( "tutorialCompletedCount",
                                                 tutorialCompletedCount );
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

    if( inLevelRisenTo >=0 && inLevelRisenTo < 7 ) {
        levelVisited[ inLevelRisenTo ] = true;
        }

    switch( inLevelRisenTo ) {
        case 1:
            tutorialsReady[1] = true;
            break;
        case 7:
            rigPowerUpsForTeaching = false;
            break;
        case 8:
            tutorialsReady[2] = true;
            break;
        }

    if( tutorialsDone[2] && inLevelRisenTo >= 8 ) {
        // risen out of whatever was entered,
        // or at least in a good spot to suggest "entering anything..."
        tutorialsReady[3] = true;
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
    if( tutorialsDone[1] ) {
        
        
        
        if( tutorialsReady[2] ) {
            enteredTypes[ (int)inType ] = true;

            tutorialsDone[2] = true;

            if( enteredTypes[0] && enteredTypes[1] && enteredTypes[2] ) {
                // entered all three types after basic enter tutorial shown
                // maybe don't need to show "enter anything..."
                tutorialsDone[3] = true;
                }
            }
        }
    }

