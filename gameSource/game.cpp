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
#include "fixedSpriteBank.h"



// globals


// used for picking a "slice" from various noise functions
double globalRandomSeed = 0;





















// position of view in world
doublePair playerPos = {0, 0};
doublePair lastScreenViewCenter = playerPos;


// world with of one view
double viewWidth = 20;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;

int screenW, screenH;

float mouseSpeed;



double velocityX = 0;
double velocityY = 0;

double moveSpeed = 0.25;


unsigned int randSeed = 1285702441;//time( NULL );
CustomRandomSource randSource(randSeed);


char shooting = false;
int stepsTilNextBullet = 0;
int stepsBetweenBullets = 5;
double bulletSpeed = 0.5;


char entering = false;



const char *getWindowTitle() {
    return "Game 10";
    }

Level *currentLevel;

SimpleVector<Level *> levelRiseStack;


typedef struct LevelPositionInfo {
        doublePair playerPos;
        doublePair lastScreenViewCenter;
        doublePair entryPosition;
        double lastMouseX;
        double lastMouseY;
    } LevelPositionInfo;

SimpleVector<LevelPositionInfo> levelRisePositionInfoStack;


// for zooming into new level
Level *lastLevel = NULL;
LevelPositionInfo lastLevelPosition;
doublePair lastLevelCurrentViewCenter;
double lastLevelCurrentViewSize;


double zoomProgress = 0;
double zoomSpeed = 0.02;
double zoomDirection = 1;




void initFrameDrawer( int inWidth, int inHeight ) {
    screenW = inWidth;
    screenH = inHeight;
    
    printf( "Rand seed = %d\n", randSeed );
    

    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );
    setViewSize( viewWidth );

    viewHeightFraction = inHeight / (double)inWidth;

    
    mouseSpeed = viewWidth / inWidth;
    
    setCursorVisible( false );
    //grabInput( true );
    
    // raw screen coordinates
    setMouseReportingMode( false );
    
    int x,y;
    warpMouseToCenter( &x, &y );
    
    
    initSpriteBank();

    currentLevel = new Level();
    }


void freeFrameDrawer() {
    delete currentLevel;
    
    freeSpriteBank();

    for( int i=0; i<levelRiseStack.size(); i++ ) {
        delete *( levelRiseStack.getElement( i ) );
        }
    levelRiseStack.deleteAll();    
    levelRisePositionInfoStack.deleteAll();
    }




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


    char stencilDrawn = false;
    
    double zoomFactor = 1;
    double viewSize = viewWidth;
    
    if( lastLevel != NULL ) {
        //zoomFactor = ( 1 + 50 * pow( zoomProgress, 2 ) );
        zoomFactor = 
            (sin( (zoomProgress * 2 - 1) * M_PI/2 ) * 0.5 + 0.5 ) 
            * 70 + 1;
        
        

        viewSize = viewWidth / zoomFactor;

        setViewSize( viewSize );
        lastLevelCurrentViewSize = viewSize;
        
        // move toward entry point as we zoom in
        double moveFraction = 1 - 1/zoomFactor + ( zoomProgress * 1/ 71 );
        doublePair center = lastLevelPosition.lastScreenViewCenter;
        
        center.x *= ( 1 - moveFraction );
        center.y *= ( 1 - moveFraction );
        
        center.x += moveFraction * lastLevelPosition.entryPosition.x;
        center.y += moveFraction * lastLevelPosition.entryPosition.y;
        
        
        
        setViewCenterPosition( center.x, center.y );
    
        setDrawColor( 0, 0, 0, 1 );
    
        drawSquare( center, viewSize );

        lastLevel->setItemWindowPosition( lastLevelPosition.entryPosition );
        lastLevel->drawLevel( center, viewSize );
        stencilDrawn = true;
        
        lastLevelCurrentViewCenter = center;
        

        // now draw current level
        

        center = sub( lastLevelPosition.entryPosition,
                      lastLevelPosition.lastScreenViewCenter );
        
        center.x *= 1 - moveFraction;
        center.y *= 1 - moveFraction;
        center.x *= -1;
        center.y *= -1;
        center.x *= 71;
        center.y *= 71;
        
        center = add( center, lastScreenViewCenter );
        /*
        setViewCenterPosition( lastScreenViewCenter.x, 
                               lastScreenViewCenter.y );
        */
        setViewCenterPosition( center.x, 
                               center.y );

        setDrawColor( 0, 0, 0, 1 );
    
        setViewSize( viewWidth );
        drawSquare( center, viewWidth );


        viewSize = 71 * viewWidth / zoomFactor;        
        setViewSize( viewSize );
        }
    else {
        setDrawColor( 0, 0, 0, 1 );
    
        drawSquare( lastScreenViewCenter, viewWidth );
        }

    
    doublePair mousePos = { lastMouseX, lastMouseY };
    currentLevel->setMousePos( mousePos );
    currentLevel->setPlayerPos( playerPos );
    currentLevel->setEnteringMouse( entering );
    currentLevel->drawLevel( lastScreenViewCenter, viewSize );


    if( stencilDrawn ) {
        
        
        if( lastLevel != NULL ) {
            setViewSize( lastLevelCurrentViewSize );
            setViewCenterPosition( lastLevelCurrentViewCenter.x,
                                   lastLevelCurrentViewCenter.y );
            lastLevel->drawWindowShade( 1 - zoomProgress );
            }

        // step zoom and check for zoom end

        zoomProgress += zoomSpeed * zoomDirection;
        
        if( zoomProgress >= 1 && zoomDirection == 1) {
            lastLevel = NULL;
            // go with current level
            setViewSize( viewWidth );
            setViewCenterPosition( lastScreenViewCenter.x, 
                                   lastScreenViewCenter.y );
            
            currentLevel->drawFloorEdges( true );
            }
        else if( zoomProgress <= 0 && zoomDirection == -1 ) {
            
            // done with current level
            delete currentLevel;
            
            // switch to last level (zooming out)
            currentLevel = lastLevel;
            currentLevel->freezeLevel( false );
            currentLevel->forgetItemWindow();
            playerPos = lastLevelPosition.playerPos;
            lastScreenViewCenter = lastLevelPosition.lastScreenViewCenter;
            lastMouseX = lastLevelPosition.lastMouseX;
            lastMouseY = lastLevelPosition.lastMouseY;
            
            mousePos.x = lastMouseX;
            mousePos.y = lastMouseY;
            

            lastLevel = NULL;
        
            setViewSize( viewWidth );
            setViewCenterPosition( lastScreenViewCenter.x, 
                                   lastScreenViewCenter.y );
            }    
        }
    

    
    



    doublePair velocity = { velocityX, velocityY };
    
    if( velocityX != 0 && velocityY != 0 ) {
        // diagonal... slow it down so it's not faster than H or V move
        
        // this creates aliasing glitches in player position
        //double componentVelocity = sqrt( (moveSpeed * moveSpeed)/2 );
        
        // use closest fraction of 16 pixels:  3/16
        double componentVelocity = 0.1875;

        velocity.x = velocity.x / moveSpeed * componentVelocity;
        velocity.y = velocity.y / moveSpeed * componentVelocity;
        }
    


    doublePair newPlayerPos = currentLevel->stopMoveWithWall( playerPos,
                                                              velocity );
    
    doublePair viewDelta = sub( newPlayerPos, playerPos );
    
    
    //printf( "Player pos = %f, %f\n", newPlayerPos.x, newPlayerPos.y );
    
    

    playerPos = newPlayerPos;

    
    // between player and reticle, closer to player (to keep player on screen)
    doublePair posToCenterOnScreen = add( playerPos, mousePos );
    // more weight to player pos
    posToCenterOnScreen = add( playerPos, posToCenterOnScreen );
        
    posToCenterOnScreen.x /= 3;
    posToCenterOnScreen.y /= 3;
    

    double screenCenterDistanceFromPlayer = 
        distance( posToCenterOnScreen, lastScreenViewCenter );
    double minDistanceToMoveScreen = 
        0.2 * ( viewWidth * viewHeightFraction ) / 2;

    // stop move screen whenever position to center is inside the center square
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
            double mouseDist = distance( mousePos, playerPos );
            doublePair bulletVelocity = sub( mousePos, playerPos );
            
            // normalize
            bulletVelocity.x /= mouseDist;
            bulletVelocity.y /= mouseDist;
            
            // set speed
            bulletVelocity.x *= bulletSpeed;
            bulletVelocity.y *= bulletSpeed;
            
            
            currentLevel->addBullet( playerPos, bulletVelocity, true );
            

            stepsTilNextBullet = stepsBetweenBullets;
            }
        }
    
    // always decrement, even when mouse not held down
    if( stepsTilNextBullet > 0 ) {
        stepsTilNextBullet --;
        }


    if( entering && lastLevel == NULL ) {
        doublePair mousePos = { lastMouseX, lastMouseY };

        int enemyIndex;
        doublePair enteringPos;
        char rising = false;
        
        if( currentLevel->isEnemy( mousePos, &enemyIndex ) ) {
            
            enteringPos = currentLevel->getEnemyCenter( enemyIndex );
            rising = true;
            }
        else if( currentLevel->isPlayer( mousePos ) ) {
            enteringPos = playerPos;
            rising = true;
            }
        

        if( rising ) {
            levelRiseStack.push_back( currentLevel );
            // enemy is entry position
            LevelPositionInfo info = 
                { playerPos, lastScreenViewCenter, 
                  enteringPos,
                  lastMouseX, lastMouseY };
            levelRisePositionInfoStack.push_back( info );

            lastLevel = currentLevel;
            lastLevel->freezeLevel( true );

            lastLevelPosition = info;
            zoomProgress = 0;
            zoomDirection = 1;
            
            currentLevel = new Level();
            playerPos.x = 0;
            playerPos.y = 0;
            lastMouseX = 0;
            lastMouseY = 0;
            lastScreenViewCenter.x = 0;
            lastScreenViewCenter.y = 0;
            setViewCenterPosition( 0, 0 );
            currentLevel->drawFloorEdges( false );
            }
        
        }
    
    if( currentLevel->isRiseSpot( playerPos ) && lastLevel == NULL ) {
        
        if( levelRiseStack.size() == 0 ) {
            // push one on to rise into
            levelRiseStack.push_back( new Level() );
            
            LevelPositionInfo info = 
                { {0,0}, {0,0}, {0,0}, 0, 0 };
            levelRisePositionInfoStack.push_back( info );
            }
        
        // rise up to last level on stack
        lastLevel = 
            *( levelRiseStack.getElement( levelRiseStack.size() - 1 ) );
        
        levelRiseStack.deleteElement( levelRiseStack.size() - 1 );
        
        lastLevelPosition =
            *( levelRisePositionInfoStack.getElement( 
                   levelRisePositionInfoStack.size() - 1 ) );
        levelRisePositionInfoStack.deleteElement( 
            levelRisePositionInfoStack.size() - 1 );
        
        lastLevel->freezeLevel( true );
        zoomProgress = 1;
        zoomDirection = -1;
        currentLevel->drawFloorEdges( false );
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
        case ' ':
            entering = true;
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
        case ' ':
            entering = false;
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






    



