#include "minorGems/game/doublePair.h"

void initTutorial();


void freeTutorial();


void drawTutorial( doublePair inScreenCenter );



// report movement keys pressed
void tutorialKeyPressed( int inKeyNum );

// report mouse shooting used
void tutorialEnemyDestroyed();


// report enter function used
void tutorialSomethingEntered();



// has no effect in middle of running tutorial
void resetTutorial();
