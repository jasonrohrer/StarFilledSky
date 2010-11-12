#include "tutorial.h"

#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/util/SettingsManager.h"

#include "Font.h"
#include "drawUtils.h"


#include <math.h>



extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;

extern Font *mainFont2;



#define numTut 4

static const char *tutorialKeys[ numTut ] = 
{ "tutorial_move", "tutorial_shoot", "tutorial_enter1", "tutorial_enter2" };

static char tutorialsDone[ numTut ] = { false, false, false, false };

int finalStepFrameCount = 0;


static char moveKeysPressed[ 4 ] = { false, false, false, false };


static double tutorialOffset = 0;
static double tutorialFade = 0;

static int currentTut = -1;

static int tutorialCompletedCount = 0;



void initTutorial() {
    currentTut = 0;

    char countFound = false;
    int readCount = SettingsManager::getIntSetting( "tutorialCompletedCount", 
                                                    &countFound );
    
    if( countFound && readCount > 0 ) {
        tutorialCompletedCount = readCount;
        }

    if( tutorialCompletedCount >= 2 ) {
        currentTut = -1;
        }
    }


void freeTutorial() {
    }


void resetTutorial() {
    if( currentTut == -1 ) {
        // not running;
        

        for( int i=0; i<numTut; i++ ) {
            tutorialsDone[i] = false;
            }
        

        finalStepFrameCount = 0;


        for( int i=0; i<4; i++ ) {
            moveKeysPressed[i] = false;
            }
        
        tutorialOffset = 0;
        tutorialFade = 0;

        currentTut = 0;
        }
    }



void drawTutorial( doublePair inScreenCenter ) {


    if( currentTut != -1 ) {
        
        // tutorial text
        const char *tutMessage = translate( tutorialKeys[ currentTut ] );
    
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
    

    // show final until 2 things entered, or until enough time passes
    if( currentTut == numTut - 1 ) {
        finalStepFrameCount ++;
    
        if( finalStepFrameCount > 600 / frameRateFactor ) {
            tutorialsDone[ currentTut ] = true;
            }
        }
    

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


// report mouse shooting used
void tutorialEnemyDestroyed() {
    tutorialsDone[1] = true;
    }



// report enter function used
void tutorialSomethingEntered() {
    if( !tutorialsDone[2] ) {
        tutorialsDone[2] = true;
        }
    else {
        tutorialsDone[3] = true;
        }

    if( !tutorialsDone[1] ) {
        // entered before shooting used
        // assume they know how to enter, and don't show dangling final
        // enter tip by itself
        tutorialsDone[3] = true;
        }
    }

