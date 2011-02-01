#include "setTipDisplay.h"

#include "minorGems/game/game.h"
#include "minorGems/util/stringUtils.h"
#include "Font.h"

#include "tipDisplay.h"
#include "powerUpProperties.h"


extern Font *tinyFont;

extern double viewWidth;
extern double viewHeightFraction;
extern double frameRateFactor;
extern double dashHeight;



static char tipShowing = false;

static float tipProgress = 0;

static float tipFadeIn = 1.0f;


static char tipUnderMiddleBracket = false;

static char tipIsBlocking = false;



#define SET_TIP_CAPACITY 4

static int numFilledStrings = 0;

static char *currentTipStrings[SET_TIP_CAPACITY] = { NULL, NULL, NULL, NULL };




void initSetTipDisplay() {
    }


static void clearTipStrings() {
    for( int i=0; i<SET_TIP_CAPACITY; i++ ) {
        if( currentTipStrings[i] != NULL ) {
            delete [] currentTipStrings[i];
            currentTipStrings[i] = NULL;
            }
        }
    numFilledStrings = 0;
    }



void freeSetTipDisplay() {
    clearTipStrings();
    }



void drawSetTipDisplay( doublePair inScreenCenter ) {
    
    if( tipShowing ) {
        

        float fade = 1;
        
        if( tipFadeIn == 1 ) {
            // fade out at end of progress
            if( tipProgress > 0.75 ) {
                fade = 1 - ( tipProgress - 0.75 ) / 0.25;
                }
            }
        else {
            // still fading in
            fade = tipFadeIn;
            }
        
        
        doublePair tipPos = inScreenCenter;
        
        if( tipUnderMiddleBracket ) {
            tipPos.x -= 0.4375;
            }
        else {
            tipPos.x -= viewWidth / 2;
            tipPos.x += 3.3125;
            }
        

        tipPos.y += (viewWidth * viewHeightFraction) /  2;
        
        tipPos.y -= dashHeight;
        
        tipPos.y -= 0.4375;
        
        // scoot down lower to avoid tutorial bracket
        tipPos.y -= 0.375;

        
        for( int i=0; i<numFilledStrings; i++ ) {
            
            drawTip( currentTipStrings[i], tipPos, fade );

            tipPos.y -= 0.5;
            }
        
        if( tipFadeIn < 1 ) {
            tipFadeIn += 0.01 * frameRateFactor;
            
            if( tipFadeIn > 1 ) {
                tipFadeIn = 1;
                }
            }
        else {
            // fade-in complete
            // start progress

            tipProgress += 0.00417 * frameRateFactor;
            
            if( tipProgress > 1 ) {
                tipShowing = false;
                tipProgress = 0;
                tipIsBlocking = false;
                }
            }
        }
    }



void forceSetTipEnd() {
    if( tipProgress < 0.75 ) {
        // jump right to end fade-out
        tipFadeIn = 1;
        tipProgress = 0.75;
        }
    }





void triggerSetTip( PowerUpSet *inSet, char inUnderMiddleBracket,
                    char inFadeIn, char inBlocking ) {

    if( tipIsBlocking ) {
        // already showing a blocking tip
        // ignore this one
        return;
        }
    
    


    tipShowing = true;
    
    // reset progress (instant jump)
    tipProgress = 0;

    tipUnderMiddleBracket = inUnderMiddleBracket;
    
    if( inFadeIn ) {
        tipFadeIn = 0;
        }
    else {
        tipFadeIn = 1;
        }

    tipIsBlocking = inBlocking;
    

    
    clearTipStrings();

    // first string is always health

    currentTipStrings[0] = 
        autoSprintf( "%s %d",
                     translate( "powerUpHeartSetCombo_tip" ),
                     getTotalLevel( inSet, powerUpHeart ) + 1 );
    
    numFilledStrings++;
    
    // walk through all possible powers and create a tip string for
    // any that are non-zero
    for( int p=powerUpBulletSize; p<=powerUpExplode; p++ ) {
        int level = getTotalLevel( inSet, (spriteID)p );
        
        if( level > 0 ) {
            
            currentTipStrings[ numFilledStrings ] =
                autoSprintf( 
                    "%s LEVEL %d",
                    translate( spriteIDTipTranslateKeys[ p ] ), level );
            
            numFilledStrings++;
            }
        }
    }

