#include "minorGems/game/doublePair.h"

#include "Level.h"


// must be called before init to have an effect
void forceTutorialEnd();
void forceTutorialFreshStart();


// check if tutorial should run, even if we're not ready to init it yet
void checkTutorial();



void initTutorial();


void freeTutorial();


void drawTutorial( doublePair inScreenCenter );


// true if we're still teaching about power-ups, where they are revealed
// in a certain order on various levels
char shouldPowerUpsBeRigged();

char shouldEnterBeBlocked();


// check if we're revisiting a level in tutorial mode
// allows us to stick some hearts there to prevent user from getting stuck
// only tracks levels 0 through 6
char levelAlreadyVisited( int inLevelNumber );



// report movement keys pressed
void tutorialKeyPressed( int inKeyNum );

// report that rise marker discovered and operated
void tutorialRiseHappened( int inLevelRisenTo );


// report mouse shooting used
void tutorialEnemyHit();


// report enter function used
void tutorialSomethingEntered( itemType inType );

void tutorialPlayerKnockedDown();



// has no effect in middle of running tutorial
void resetTutorial();
