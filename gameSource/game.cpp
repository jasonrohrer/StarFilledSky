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



#include "drawUtils.h"
#include "Level.h"


// globals


// used for picking a "slice" from various noise functions
double globalRandomSeed = 0;





















// position of view in world
doublePair viewCenter = {0, 0};

// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;


double velocityX = 0;
double velocityY = 0;

double moveSpeed = 0.25;


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



const char *getWindowTitle() {
    return "Game 10";
    }

Level *currentLevel;



void initFrameDrawer( int inWidth, int inHeight ) {

    setViewCenterPosition( viewCenter.x, viewCenter.y );
    setViewSize( viewWidth );

    viewHeightFraction = inHeight / (double)inWidth;

    setCursorVisible( false );

    currentLevel = new Level();
    }

void freeFrameDrawer() {
    }


int numRadii = 100;


char lightingOn = true;



float lastMouseX, lastMouseY;

SimpleVector<doublePair> hitWallSpots;


void drawFrame() {


    // draw stuff



    /*
    glClearColor( mBackgroundColor->r,
                  mBackgroundColor->g,
                  mBackgroundColor->b,
                  mBackgroundColor->a );
    */

      
    /*

    if( !currentLevel->isWall( newViewPos ) ) {
        viewCenter = newViewPos;
        setViewCenterPosition( viewCenter.x, viewCenter.y );
        }
    else {
        hitWallSpots.push_back( newViewPos );
        printf( "hit at %f,%f\n", newViewPos.x, newViewPos.y );
        
        }
    */
        
    
    currentLevel->drawLevel( viewCenter );

    setDrawColor( 1, 0, 0, 1 );
    
    doublePair p = { lastMouseX, lastMouseY };
    
    drawSquare( p, 0.125 );
    
    p = viewCenter;
    drawSquare( p, 0.125 );

    setDrawColor( 0, 1, 0, 0.5 );
    
    for( int i=0; i<hitWallSpots.size(); i++ ) {
        p = *( hitWallSpots.getElement( i ) );
        //drawSquare( p, 0.06 );
        }



        doublePair newViewPos = viewCenter;
    
    newViewPos.x += velocityX;
    newViewPos.y += velocityY;


    if( currentLevel->isWall( newViewPos ) ) {
        doublePair xMoveAlone = viewCenter;
        xMoveAlone.x += velocityX;
        
        if( !currentLevel->isWall( xMoveAlone ) ) {
            
            // push y as close as possible to nearest wall
            
            int intY = (int)rint( xMoveAlone.y );
            if( velocityY > 0 ) {
                xMoveAlone.y = intY + 0.45;
                }
            else {
                xMoveAlone.y = intY - 0.45;
                }
            
            newViewPos = xMoveAlone;
            }
        else {
            // try y move alone
            doublePair yMoveAlone = viewCenter;
            yMoveAlone.y += velocityY;
        
            if( !currentLevel->isWall( yMoveAlone ) ) {
                
                // push x as close as possible to nearest wall
            
                int intX = (int)rint( yMoveAlone.x );
                if( velocityX > 0 ) {
                    yMoveAlone.x = intX + 0.45;
                    }
                else {
                    yMoveAlone.x = intX - 0.45;
                    }


                newViewPos = yMoveAlone;
                }
            else {
                // both hit
                newViewPos = viewCenter;
                }
            }
        }
    
    
    doublePair viewDelta = sub( newViewPos, viewCenter );
    
    // move mouse with screen
    lastMouseX += viewDelta.x;
    lastMouseY += viewDelta.y;
    

    viewCenter = newViewPos;
    setViewCenterPosition( viewCenter.x, viewCenter.y );

    
    }



void pointerMove( float inX, float inY ) {
    lastMouseX = inX;
    lastMouseY = inY;
    }

void pointerDown( float inX, float inY ) {}
void pointerDrag( float inX, float inY ) {
    lastMouseX = inX;
    lastMouseY = inY;
    }

void pointerUp( float inX, float inY ) {}










void keyDown( unsigned char inASCII ) {

    }


void keyUp( unsigned char inASCII ) {}







void specialKeyDown( int inKey ) {


    switch( inKey ) {
        case MG_KEY_UP:
            velocityY = moveSpeed;
            break;
        case MG_KEY_DOWN:
            velocityY = -moveSpeed;
            break;
        case MG_KEY_RIGHT:
            velocityX = moveSpeed;
            break;
        case MG_KEY_LEFT:
            velocityX = -moveSpeed;
            break;
        }

	}



void specialKeyUp( int inKey ) {
    switch( inKey ) {
        case MG_KEY_UP:
            velocityY = 0;
            break;
        case MG_KEY_DOWN:
            velocityY = 0;
            break;
        case MG_KEY_RIGHT:
            velocityX = 0;
            break;
        case MG_KEY_LEFT:
            velocityX = 0;
            break;
        }

	} 






    



