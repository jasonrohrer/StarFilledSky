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


void initTipDisplay() {
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
            
        printf( "message width = %f pixels\n", messageWidth * 16 );
        
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
            }
        }
    }




void triggerTip( spriteID inPowerType, float inStartingProgress ) {
    tipShowing = true;
    
    // reset progress (instant jump)
    tipProgress = inStartingProgress;
    

    currentTipString = translate( spriteIDTipTranslateKeys[ inPowerType ] );
    }

