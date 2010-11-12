#include "tutorial.h"
#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "Font.h"
#include "drawUtils.h"


double tutorialOffset = 0;
double tutorialFade = 0;



extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;

extern Font *mainFont2;



void initTutorial() {
    
    }



void freeTutorial() {
    }



void drawTutorial( doublePair inScreenCenter ) {

    // tutorial text
    const char *tutMessage = translate( "tutorial_move" );
    
    double offsetLimit = viewWidth * viewHeightFraction / 2 - 0.5;


    doublePair tutorialPos = inScreenCenter;
    tutorialPos.y -= tutorialOffset;

    double messageWidth = mainFont2->measureString( tutMessage );
    
    setDrawColor( 0, 0, 0, 0.5 * tutorialFade );
    drawRect( tutorialPos.x - messageWidth / 2 - 0.25, 
              tutorialPos.y - 0.5, 
              tutorialPos.x + messageWidth / 2 + 0.25, 
              tutorialPos.y + 0.5 );
    setDrawColor( 1, 1, 1, tutorialFade );
    mainFont2->drawString( translate( "tutorial_move" ), 
                          tutorialPos, alignCenter );

    if( tutorialOffset < offsetLimit ) {
        
        tutorialFade += 0.01 * frameRateFactor;
    
        if( tutorialFade > 1 ) {
            tutorialFade = 1;
            }
        }
    else {
        tutorialFade -= 0.01 * frameRateFactor;
        
        if( tutorialFade < 0 ) {
            tutorialFade = 0;
            }
        }
    
    
    if( tutorialFade == 1 ) {
        
        tutorialOffset += 0.1 * frameRateFactor;
        
        if( tutorialOffset > offsetLimit ) {
            tutorialOffset = offsetLimit;
            }
        }

    }





// report movement keys pressed
void tutorialKeyPressed( int inKeyNum ) {
    }


// report mouse shooting used
void tutorialEnemyDestroyed() {
    }



// report enter function used
void tutorialSomethingEntered() {
    }

