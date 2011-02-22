#include "tipDisplay.h"

#include "minorGems/game/game.h"
#include "minorGems/util/stringUtils.h"
#include "Font.h"
#include "drawUtils.h"


#include <limits.h>



extern Font *tinyFont;

extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;
extern double dashHeight;



static char tipShowing = false;

static float tipProgress = 0;

const char *currentTipString = NULL;

spriteID currentTipSpriteID = endSpriteID;

int currentTipDifficultyModifier = 0;



//static int timesToShowEachTip = 3;
// effectively disable quotas.  Always show tips
static int timesToShowEachTip = INT_MAX;


// delay counter before showing a tip after it has been shown the base
// number of times
int delayStepCount = 0;

int delaySteps;


double maxTipStringWidth = 0;



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


    for( int p=powerUpHeart; p<=powerUpExplode; p++ ) {
        
        const char *tipString = 
            translate( spriteIDTipTranslateKeys[ p ] );

        double width = tinyFont->measureString( tipString );
        
        if( width > maxTipStringWidth ) {
            maxTipStringWidth = width;
            }
        }
    }




void freeTipDisplay() {
    }



void drawTip( const char *inMessage, doublePair inPos, float inFade,
              char inRedText ) {
    double messageWidth = tinyFont->measureString( inMessage );
            
        
    setDrawColor( 0, 0, 0, 0.5 * inFade );
    drawRect( inPos.x - messageWidth / 2 - 0.125, 
              inPos.y - 0.25, 
              inPos.x + messageWidth / 2 + 0.125, 
              inPos.y + 0.25 );

    if( inRedText ) {
        setDrawColor( 1, 0, 0, inFade );
        }
    else {
        setDrawColor( 1, 1, 1, inFade );
        }
    
    // shift by one sub-pixel, font drawing code is currently a bit off
    inPos.x += 0.03125;
    tinyFont->drawString( inMessage, 
                          inPos, alignCenter );
    }



void drawTipDisplay( doublePair inScreenCenter ) {
    
    if( tipShowing ) {
        

        float fade = 1;
        
        if( tipProgress > 0.75 ) {
            fade = 1 - ( tipProgress - 0.75 ) / 0.25;
            }
        
        doublePair tipPos = inScreenCenter;

        tipPos.x += viewWidth / 2;
        
        tipPos.x -= maxTipStringWidth / 2 + 0.25;


        
        tipPos.y += (viewWidth * viewHeightFraction) /  2;
        
        tipPos.y -= dashHeight;
        
        tipPos.y -= 0.4375;
        
        
        drawTip( currentTipString, tipPos, fade );
        
        // don't display +1 modifiers, because they're not important
        if( currentTipDifficultyModifier > 1 ) {
            tipPos.y -= 0.5;
            
            char *difficultyModTipString = 
                autoSprintf( "+%d %s",
                             currentTipDifficultyModifier,
                             translate( "difficultyTag" ) );
        

            // scoot over if it is going off the edge of the screen
            double messageWidth = 
                tinyFont->measureString( difficultyModTipString );

            double overhang = 
                ( tipPos.x + messageWidth / 2 + 0.125 ) -
                ( inScreenCenter.x + viewWidth / 2 );
            
            if( overhang > 0 ) {
                tipPos.x -= overhang;
                }
            
            drawTip( difficultyModTipString, tipPos, fade, true );
            
            delete [] difficultyModTipString;
            }
        

        
        tipProgress += 0.00417 * frameRateFactor;
        
        if( tipProgress > 1 ) {
            tipShowing = false;
            tipProgress = 0;

            currentTipSpriteID = endSpriteID;
            currentTipString = NULL;
            }
        }
    }




void forceTipEnd() {
    if( tipProgress < 0.75 ) {
        // jump right to end fade-out
        tipProgress = 0.75;
        }
    }



void triggerTip( spriteID inPowerType, char inShowAfterQuota, 
                 float inStartingProgress, int inDifficultyModifier ) {

    char freshShow = false;
    
    if( !tipShowing || currentTipSpriteID != inPowerType ) {
        freshShow = true;
        }
    

    if( freshShow && tipShownCount[ inPowerType ] >= timesToShowEachTip ) {
    
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

    currentTipDifficultyModifier = inDifficultyModifier;
    }

