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
        
        if( tipProgress > 0.75 ) {
            fade = 1 - ( tipProgress - 0.75 ) / 0.25;
            }
        
        doublePair tipPos = inScreenCenter;
        
        tipPos.x -= viewWidth / 2;
        tipPos.x += 3.3125;
        

        tipPos.y += (viewWidth * viewHeightFraction) /  2;
        
        tipPos.y -= dashHeight;
        
        tipPos.y -= 0.4375;
        
        // scoot down lower to avoid tutorial bracket
        tipPos.y -= 0.375;

        
        for( int i=0; i<numFilledStrings; i++ ) {
            
            drawTip( currentTipStrings[i], tipPos, fade );

            tipPos.y -= 0.5;
            }
        
        tipProgress += 0.005 * frameRateFactor;
        
        if( tipProgress > 1 ) {
            tipShowing = false;
            tipProgress = 0;
            }
        }
    }




void triggerSetTip( PowerUpSet *inSet ) {

    
    tipShowing = true;
    
    // reset progress (instant jump)
    tipProgress = 0;
    
    
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

