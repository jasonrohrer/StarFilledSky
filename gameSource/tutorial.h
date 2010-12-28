#include "minorGems/game/doublePair.h"

#include "Level.h"

void initTutorial();


void freeTutorial();


void drawTutorial( doublePair inScreenCenter );



// report movement keys pressed
void tutorialKeyPressed( int inKeyNum );

// report that rise marker discovered and operated
void tutorialRiseHappened( int inLevelRisenTo );


// report mouse shooting used
void tutorialEnemyHit();


// report enter function used
void tutorialSomethingEntered( itemType inType );



// has no effect in middle of running tutorial
void resetTutorial();
