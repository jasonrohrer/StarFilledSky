#include "minorGems/game/doublePair.h"
#include "fixedSpriteBank.h"


void initTipDisplay();


void freeTipDisplay();



void drawTipDisplay( doublePair inScreenCenter );



// can start tip in a partially-advanced state (to prevent it from lingering
// after it stops being re-triggered)
// 0.75 is the fade-out point, where 1-second fade starts
// Thus, setting inStartingProgress to 0.5 shows tip for one second before fade
// starts.
// Setting it to 0.0 (default) shows tip for 3 seconds before fade starts. 
void triggerTip( spriteID inPowerType, 
                 char inShowAfterQuota = false,
                 float inStartingProgress = 0 );


void forceTipEnd();


// utility function shared by setTipDisplay

void drawTip( const char *inMessage, doublePair inPos, float inFade );
