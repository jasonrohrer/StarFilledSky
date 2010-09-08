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

SimpleVector<doublePair> hitWallSpots;



static void confineMouseOnScreen() {
    double halfViewWidth = viewWidth / 2;
    
    if( lastMouseX > viewCenter.x + halfViewWidth ) {
        lastMouseX = viewCenter.x + halfViewWidth;
        }
    else if( lastMouseX < viewCenter.x - halfViewWidth ) {
        lastMouseX = viewCenter.x - halfViewWidth;
        }

    double halfViewHeight = ( viewWidth * viewHeightFraction ) / 2;
    
    if( lastMouseY > viewCenter.y + halfViewHeight ) {
        lastMouseY = viewCenter.y + halfViewHeight;
        }
    else if( lastMouseY < viewCenter.y - halfViewHeight ) {
        lastMouseY = viewCenter.y - halfViewHeight;
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

    // reticle
    setDrawColor( 0, 1, 0, 0.5 );
    
    doublePair p = { lastMouseX, lastMouseY };
    
    drawSquare( p, 0.125 );

    setDrawColor( 0, 0, 0, 0.5 );

    drawSquare( p, 0.025 );

    
    // player
    setDrawColor( 1, 0, 0, 1 );
    p = viewCenter;
    drawSquare( p, 0.125 );

    setDrawColor( 0, 1, 0, 0.5 );
    
    for( int i=0; i<hitWallSpots.size(); i++ ) {
        p = *( hitWallSpots.getElement( i ) );
        //drawSquare( p, 0.06 );
        }


    doublePair velocity = { velocityX, velocityY };
    
    doublePair newViewPos = currentLevel->stopMoveWithWall( viewCenter,
                                                            velocity );
    
    doublePair viewDelta = sub( newViewPos, viewCenter );
    
    // move mouse with screen
    //lastMouseX += viewDelta.x;
    //lastMouseY += viewDelta.y;

    
    
    

    viewCenter = newViewPos;
    setViewCenterPosition( viewCenter.x, viewCenter.y );

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






    



