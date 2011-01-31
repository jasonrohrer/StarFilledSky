#include "tipDisplay.h"

#include "minorGems/game/game.h"
#include "Font.h"
#include "drawUtils.h"


extern Font *tinyFont;

extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;
extern double dashHeight;



static char tipShowing = false;

static float tipProgress = 0;

const char *currentTipString = NULL;

spriteID currentTipSpriteID = endSpriteID;


static int timesToShowEachTip = 5;


// delay counter before showing a tip after it has been shown the base
// number of times
int delayStepCount = 0;

int delaySteps;




// use fixedSpriteBank's macro 

// redefine F so that it expands each name into a 0 counter init
// constant
#undef F
#define F(inName) 0

static int tipShownCount[] = {
	FIXED_SPRITE_NAMES
    };





void initTipDisplay() {

    delaySteps = (int)( 60 / frameRateFactor );
    }



void freeTipDisplay() {
    }




void drawTipDisplay( doublePair inScreenCenter ) {
    
    if( tipShowing ) {
        

        float fade = 1;
        
        if( tipProgress > 0.5 ) {
            fade = 1 - ( tipProgress - 0.5 ) / 0.5;
            }
        
        doublePair tipPos = inScreenCenter;
        
        tipPos.y += (viewWidth * viewHeightFraction) /  2;
        
        tipPos.y -= dashHeight;
        
        tipPos.y -= 0.4375;
        
        
        double messageWidth = tinyFont->measureString( currentTipString );
            
        
        setDrawColor( 0, 0, 0, 0.5 * fade );
        drawRect( tipPos.x - messageWidth / 2 - 0.125, 
                  tipPos.y - 0.25, 
                  tipPos.x + messageWidth / 2 + 0.125, 
                  tipPos.y + 0.25 );
        setDrawColor( 1, 1, 1, fade );

        // shift by one sub-pixel, font drawing code is currently a bit off
        tipPos.x += 0.03125;
        tinyFont->drawString( currentTipString, 
                              tipPos, alignCenter );
        
        tipProgress += 0.01 * frameRateFactor;
        
        if( tipProgress > 1 ) {
            tipShowing = false;
            tipProgress = 0;

            currentTipSpriteID = endSpriteID;
            currentTipString = NULL;
            }
        }
    }




void triggerTip( spriteID inPowerType, char inShowAfterQuota, 
                 float inStartingProgress ) {

    char freshShow = false;
    
    if( !tipShowing || currentTipSpriteID != inPowerType ) {
        freshShow = true;
        }
    

    if( freshShow && tipShownCount[ inPowerType ] > timesToShowEachTip ) {
    
        if( !inShowAfterQuota ) {
            // never show this tip again
            return;
            }
        
        // else consider showing it
        delayStepCount ++;
        
        if( delayStepCount < delaySteps ) {
            return;
            }
        
        // else show it, and reset delay counter for next time
        delayStepCount = 0;
        }
    

    if( freshShow ) {
        // tip being shown fresh
        tipShownCount[ inPowerType ] ++;
        }
    
    


    tipShowing = true;
    
    // reset progress (instant jump)
    tipProgress = inStartingProgress;
    
    currentTipSpriteID = inPowerType;
    currentTipString = translate( spriteIDTipTranslateKeys[ inPowerType ] );
    }

