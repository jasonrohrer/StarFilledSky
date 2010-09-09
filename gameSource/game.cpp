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
doublePair lastScreenViewCenter = viewCenter;


// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;

int screenW, screenH;

float mouseSpeed;



double velocityX = 0;
double velocityY = 0;

double moveSpeed = 0.25;


CustomRandomSource randSource;


char shooting = false;
int stepsTilNextBullet = 0;
int stepsBetweenBullets = 5;
double bulletSpeed = 0.5;




const char *getWindowTitle() {
    return "Game 10";
    }

Level *currentLevel;



void initFrameDrawer( int inWidth, int inHeight ) {
    screenW = inWidth;
    screenH = inHeight;
    

    setViewCenterPosition( viewCenter.x, viewCenter.y );
    setViewSize( viewWidth );

    viewHeightFraction = inHeight / (double)inWidth;

    
    mouseSpeed = viewWidth / inWidth;
    
    setCursorVisible( false );
    grabInput( true );
    
    // raw screen coordinates
    setMouseReportingMode( false );
    
    int x,y;
    warpMouseToCenter( &x, &y );
    
    
    currentLevel = new Level();
    }

void freeFrameDrawer() {
    }


int numRadii = 100;


char lightingOn = true;


char haveFirstScreenMouse = false;
float lastScreenMouseX, lastScreenMouseY;



float lastMouseX = 0;
float lastMouseY = 0;




static void confineMouseOnScreen() {
    double halfViewWidth = viewWidth / 2;
    
    if( lastMouseX > lastScreenViewCenter.x + halfViewWidth ) {
        lastMouseX = lastScreenViewCenter.x + halfViewWidth;
        }
    else if( lastMouseX < lastScreenViewCenter.x - halfViewWidth ) {
        lastMouseX = lastScreenViewCenter.x - halfViewWidth;
        }

    double halfViewHeight = ( viewWidth * viewHeightFraction ) / 2;
    
    if( lastMouseY > lastScreenViewCenter.y + halfViewHeight ) {
        lastMouseY = lastScreenViewCenter.y + halfViewHeight;
        }
    else if( lastMouseY < lastScreenViewCenter.y - halfViewHeight ) {
        lastMouseY = lastScreenViewCenter.y - halfViewHeight;
        }

    }



void drawFrame() {


    // draw stuff



    /*
    glClearColor( mBackgroundColor->r,
                  mBackgroundColor->g,
                  mBackgroundColor->b,
                  mBackgroundColor->a );
    */

      
        
    
    currentLevel->drawLevel( viewCenter );

    // reticle
    setDrawColor( 0, 1, 0, 0.5 );
    
    doublePair mousePos = { lastMouseX, lastMouseY };
    
    drawSquare( mousePos, 0.125 );

    setDrawColor( 0, 0, 0, 0.5 );

    drawSquare( mousePos, 0.025 );

    
    // player
    setDrawColor( 1, 0, 0, 1 );
    drawSquare( viewCenter, 0.125 );

    setDrawColor( 0, 1, 0, 0.5 );
    
    

    doublePair velocity = { velocityX, velocityY };
    
    if( velocityX != 0 && velocityY != 0 ) {
        // diagonal... slow it down so it's not faster than H or V move
        
        double componentVelocity = sqrt( (moveSpeed * moveSpeed)/2 );
        
        velocity.x = velocity.x / moveSpeed * componentVelocity;
        velocity.y = velocity.y / moveSpeed * componentVelocity;
        }
    


    doublePair newViewPos = currentLevel->stopMoveWithWall( viewCenter,
                                                            velocity );
    
    doublePair viewDelta = sub( newViewPos, viewCenter );
    
    
    
    
    

    viewCenter = newViewPos;

    
    // halfway between player and reticle
    doublePair posToCenterOnScreen = add( viewCenter, mousePos );
    posToCenterOnScreen.x /= 2;
    posToCenterOnScreen.y /= 2;
    

    double screenCenterDistanceFromPlayer = 
        distance( posToCenterOnScreen, lastScreenViewCenter );
    double minDistanceToMoveScreen = 
        0.2 * ( viewWidth * viewHeightFraction ) / 2;

    // stop move screen whenever player is inside the center square
    if( screenCenterDistanceFromPlayer > 
        minDistanceToMoveScreen ) {

        doublePair screenMoveDelta = sub( posToCenterOnScreen, 
                                          lastScreenViewCenter );
        
        // set correction speed based on how far off we are from VERY CENTER
        // since we stop moving when player inside center box, this eliminates
        // jerky micro-movements.
        double correctionSpeed = 
            0.005 * 
            pow(
                (screenCenterDistanceFromPlayer - 0),
                2 );
        
        screenMoveDelta.x *= correctionSpeed;
        screenMoveDelta.y *= correctionSpeed;
        

        lastScreenViewCenter = add( lastScreenViewCenter, screenMoveDelta );
        
        setViewCenterPosition( lastScreenViewCenter.x, 
                               lastScreenViewCenter.y );

        if( ! shooting ) {
            // move mouse with screen
            lastMouseX += screenMoveDelta.x;
            lastMouseY += screenMoveDelta.y;
            }
        }

    
    if( viewDelta.x != 0 || viewDelta.y != 0 ) {
        confineMouseOnScreen();
        }

    if( shooting ) {
        if( stepsTilNextBullet == 0 ) {
            // fire bullet

            doublePair mousePos = { lastMouseX, lastMouseY };
            double mouseDist = distance( mousePos, viewCenter );
            doublePair bulletVelocity = sub( mousePos, viewCenter );
            
            // normalize
            bulletVelocity.x /= mouseDist;
            bulletVelocity.y /= mouseDist;
            
            // set speed
            bulletVelocity.x *= bulletSpeed;
            bulletVelocity.y *= bulletSpeed;
            
            
            currentLevel->addBullet( viewCenter, bulletVelocity, true );
            

            stepsTilNextBullet = stepsBetweenBullets;
            }
        }
    
    // always decrement, even when mouse not held down
    if( stepsTilNextBullet > 0 ) {
        stepsTilNextBullet --;
        }


    
    }


static void mouseMove( float inX, float inY ) {
    if( ! haveFirstScreenMouse ) {
        lastScreenMouseX = inX;
        lastScreenMouseY = inY;
        haveFirstScreenMouse = true;
        }
    else {
        float deltaX = inX - lastScreenMouseX;
        float deltaY = inY - lastScreenMouseY;
        
        lastScreenMouseX = inX;
        lastScreenMouseY = inY;


        lastMouseX += mouseSpeed * deltaX;
        lastMouseY -= mouseSpeed * deltaY;
        

        confineMouseOnScreen();
        }
    
    if( lastScreenMouseX < 1 || lastScreenMouseX > screenW - 2
        ||
        lastScreenMouseY < 1 || lastScreenMouseY > screenH - 2 ) {
    
        // hit edge
        int x, y;
        warpMouseToCenter( &x, &y );
        
        lastScreenMouseX = x;
        lastScreenMouseY = y;
        }
    
    }



void pointerMove( float inX, float inY ) {
    mouseMove( inX, inY );
    }

void pointerDown( float inX, float inY ) {
    mouseMove( inX, inY );
    shooting = true;
    }


void pointerDrag( float inX, float inY ) {
    mouseMove( inX, inY );
    }

void pointerUp( float inX, float inY ) {
    mouseMove( inX, inY );
    shooting = false;
    // keep old step counter going for next mouse press
    }





static char movementKeysDown[4] = { false, false, false, false };
static char lastMovementKeysDown[4] = { false, false, false, false };

static void movementKeyChange() {
    
    // new presses
    if( movementKeysDown[0] && ! lastMovementKeysDown[0] ) {
        velocityY = moveSpeed;
        }
    else if( movementKeysDown[1] && ! lastMovementKeysDown[1] ) {
        velocityY = -moveSpeed;
        }
    // releases?
    else if( movementKeysDown[0] && ! movementKeysDown[1] ) {
        velocityY = moveSpeed;
        }
    else if( movementKeysDown[1] && ! movementKeysDown[0] ) {
        velocityY = -moveSpeed;
        }
    else if( ! movementKeysDown[0] && ! movementKeysDown[1] ) {
        velocityY = 0;
        }
    

    // new presses
    if( movementKeysDown[2] && ! lastMovementKeysDown[2] ) {
        velocityX = moveSpeed;
        }
    else if( movementKeysDown[3] && ! lastMovementKeysDown[3] ) {
        velocityX = -moveSpeed;
        }
    // releases?
    else if( movementKeysDown[2] && ! movementKeysDown[3] ) {
        velocityX = moveSpeed;
        }
    else if( movementKeysDown[3] && ! movementKeysDown[2] ) {
        velocityX = -moveSpeed;
        }
    else if( ! movementKeysDown[2] && ! movementKeysDown[3] ) {
        velocityX = 0;
        }


    memcpy( lastMovementKeysDown, movementKeysDown, 4 );
    }





void keyDown( unsigned char inASCII ) {
    switch( inASCII ) {
        case 'w':
        case 'W':
            movementKeysDown[0] = true;
            movementKeyChange();
            break;
        case 's':
        case 'S':
            movementKeysDown[1] = true;
            movementKeyChange();
            break;
        case 'd':
        case 'D':
            movementKeysDown[2] = true;
            movementKeyChange();
            break;
        case 'a':
        case 'A':
            movementKeysDown[3] = true;
            movementKeyChange();
            break;
        }
    }


void keyUp( unsigned char inASCII ) {
    switch( inASCII ) {
        case 'w':
        case 'W':
            movementKeysDown[0] = false;
            movementKeyChange();
            break;
        case 's':
        case 'S':
            movementKeysDown[1] = false;
            movementKeyChange();
            break;
        case 'd':
        case 'D':
            movementKeysDown[2] = false;
            movementKeyChange();
            break;
        case 'a':
        case 'A':
            movementKeysDown[3] = false;
            movementKeyChange();
            break;
        }
    }







void specialKeyDown( int inKey ) {


    switch( inKey ) {
        case MG_KEY_UP:
            movementKeysDown[0] = true;
            movementKeyChange();
            break;
        case MG_KEY_DOWN:
            movementKeysDown[1] = true;
            movementKeyChange();
            break;
        case MG_KEY_RIGHT:
            movementKeysDown[2] = true;
            movementKeyChange();
            break;
        case MG_KEY_LEFT:
            movementKeysDown[3] = true;
            movementKeyChange();
            break;
        }

	}



void specialKeyUp( int inKey ) {
    switch( inKey ) {
        case MG_KEY_UP:
            movementKeysDown[0] = false;
            movementKeyChange();
            break;
        case MG_KEY_DOWN:
            movementKeysDown[1] = false;
            movementKeyChange();
            break;
        case MG_KEY_RIGHT:
            movementKeysDown[2] = false;
            movementKeyChange();
            break;
        case MG_KEY_LEFT:
            movementKeysDown[3] = false;
            movementKeyChange();
            break;
        }

	} 






    



