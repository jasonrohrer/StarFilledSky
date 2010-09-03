/*
 * Modification History
 *
 * 2008-September-11  Jason Rohrer
 * Created.  Copied from Cultivation.
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#include "minorGems/graphics/Color.h"




#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/random/CustomRandomSource.h"


#include "minorGems/util/log/AppLog.h"


#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"


// globals


// used for picking a "slice" from various noise functions
double globalRandomSeed = 0;





















// position of view in world
double viewCenterX = -100;
double viewCenterY = 0;

// world with of one view
double viewWidth = 10;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;


double velocityX = 0;
double velocityY = 0;
double dragX = 0;
double accelY = 0;
double dragY = 0;

double thrustX = 0;

double thrustValue = 16;

char attached = true;

// barge is heavier, thrust and drag don't affect it as much
double bargeMassFactor = 50;

double shipMassFactor = 10;


CustomRandomSource randSource;



double blackOutProgress = 0.0;
double blackOutRate = 0.004;




void initFrameDrawer( int inWidth, int inHeight ) {

    setViewCenterPosition( viewCenterX, viewCenterY );
    setViewSize( viewWidth );

    viewHeightFraction = inHeight / (double)inWidth;
    }

void freeFrameDrawer() {
    }


int numRadii = 100;


char lightingOn = true;




void drawFrame() {


    // draw stuff



    /*
    glClearColor( mBackgroundColor->r,
                  mBackgroundColor->g,
                  mBackgroundColor->b,
                  mBackgroundColor->a );
    */
    }



void pointerMove( float inX, float inY ) {}
void pointerDown( float inX, float inY ) {}
void pointerDrag( float inX, float inY ) {}
void pointerUp( float inX, float inY ) {}










void keyDown( unsigned char inASCII ) {

    }


void keyUp( unsigned char inASCII ) {}







void specialKeyDown( int inKey ) {


    switch( inKey ) {
        case MG_KEY_UP:
            break;
        case MG_KEY_DOWN:
            break;
        case MG_KEY_RIGHT:
            break;
        case MG_KEY_LEFT:
            break;
        }

	}



void specialKeyUp( int inKey ) {
    switch( inKey ) {
        case MG_KEY_UP:
            break;
        case MG_KEY_DOWN:
            break;
        case MG_KEY_RIGHT:
            break;
        case MG_KEY_LEFT:
            break;
        }

	} 






    



