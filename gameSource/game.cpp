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
#include <malloc.h>


#include "minorGems/graphics/Color.h"




#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/random/CustomRandomSource.h"


#include "minorGems/util/log/AppLog.h"

#include "minorGems/system/Time.h"


#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"



#include "drawUtils.h"
#include "Level.h"
#include "fixedSpriteBank.h"
#include "Font.h"
#include "PowerUpSet.h"
#include "numerals.h"


// globals


// used for picking a "slice" from various noise functions
double globalRandomSeed = 0;


















// height of dashboard at top of screen
double dashHeight = 1;


// position of view in world
doublePair playerPos = {-0.5, 0};
// up a bit to account for dashboard
doublePair lastScreenViewCenter = {-0.5, 0.5 * dashHeight };


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


int levelNumber = 1;

Level *currentLevel;

SimpleVector<Level *> levelRiseStack;


typedef struct LevelPositionInfo {
        doublePair playerPos;
        doublePair lastScreenViewCenter;
        doublePair entryPosition;
        itemType entryType;
        doublePair mousePos;
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


Font *mainFont;



static void populateLevelRiseStack() {
    if( levelRiseStack.size() == 0 ) {
        // push one on to rise into
        ColorScheme c = currentLevel->getLevelColors();
        ColorScheme freshColors;
        levelRiseStack.push_back( new Level( &c, &freshColors,
                                             levelNumber + 1 ) );
        
        // center player in symmetrical level
        LevelPositionInfo info = 
            { {-0.5,0}, {-0.5,0}, {-0.5,0}, player, {-0.5, 0} };
        levelRisePositionInfoStack.push_back( info );
        }
    }




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

    mainFont = new Font( "font_8_16.tga", -2, 4, true );
    
    initNumerals( "numerals.tga" );
    

    currentLevel = new Level( NULL, NULL, levelNumber );
    
    populateLevelRiseStack();
    

    // for level construction optimization
    if( false ) {
        
        double msTime = Time::getCurrentTime();
        
        for( int i=0; i<100; i++ ) {
            Level *l = new Level();
            delete l;
            }
        printf( "Contstructing levels took %f s\n",
                (Time::getCurrentTime() - msTime) );
    
        exit(0);
        }
    
    }


void freeFrameDrawer() {
    delete currentLevel;
    
    freeSpriteBank();

    freeNumerals();
    

    delete mainFont;
    

    for( int i=0; i<levelRiseStack.size(); i++ ) {
        delete *( levelRiseStack.getElement( i ) );
        }
    levelRiseStack.deleteAll();    
    levelRisePositionInfoStack.deleteAll();
    }




char haveFirstScreenMouse = false;
float lastScreenMouseX, lastScreenMouseY;



doublePair mousePos = { -0.5, 0 };



static void confineMouseOnScreen() {
    double halfViewWidth = viewWidth / 2;
    
    if( mousePos.x > lastScreenViewCenter.x + halfViewWidth ) {
        mousePos.x = lastScreenViewCenter.x + halfViewWidth;
        }
    else if( mousePos.x < lastScreenViewCenter.x - halfViewWidth ) {
        mousePos.x = lastScreenViewCenter.x - halfViewWidth;
        }

    double halfViewHeight = ( viewWidth * viewHeightFraction ) / 2;
    
    if( mousePos.y > lastScreenViewCenter.y + halfViewHeight - dashHeight ) {
        mousePos.y = lastScreenViewCenter.y + halfViewHeight - dashHeight;
        }
    else if( mousePos.y < lastScreenViewCenter.y - halfViewHeight ) {
        mousePos.y = lastScreenViewCenter.y - halfViewHeight;
        }

    }




static Level *getNextAbove() {
    Level *nextAbove;

    if( lastLevel != NULL ) {
        
        if( zoomDirection == 1 && levelRiseStack.size() > 1 ) {
            nextAbove = 
                *( levelRiseStack.getElement( levelRiseStack.size() - 2 ) );
            }
        else {
            nextAbove = lastLevel;
            }
        }
    else if( levelRiseStack.size() > 0 ) {    
        nextAbove = 
            *( levelRiseStack.getElement( levelRiseStack.size() - 1 ) );
        }
    else {
        // draw SOMETHING for now while we wait for next level to be 
        // populated
        nextAbove = currentLevel;
        }

    return nextAbove;
    }



// used to keep level rise stack populated without a visible frame hiccup
// hide the hiccup right after the final freeze frame of the level
// we're rising into

// turns out there are two final zoom frames at end of rise out
static char secondToLastRiseFreezeFrameDrawn = false;
static char lastRiseFreezeFrameDrawn = false;



void drawFrame() {

    if( lastRiseFreezeFrameDrawn ) {
        levelNumber = currentLevel->getLevelNumber();
        
        // populate stack here, in case we rise back out further
        // this prevents frame hiccups, because this happens
        // at the tail end of lastLevel's freeze right before
        // it becomes the current level, so the hiccup is hidden
        populateLevelRiseStack();
        }
    



    // update all movement and detect special conditions



    // do this here, before drawing anything, to avoid final frame hiccups
    // when entering something (due to level construction time, which varies)

    // force entry into self when player health reduced to 0
    int playerHealth, playerMax;
    currentLevel->getPlayerHealth( &playerHealth, &playerMax );
    
    if( ( entering || playerHealth == 0 ) && lastLevel == NULL ) {
        
        int itemIndex;
        doublePair enteringPos;
        char enteringHit = false;
        itemType enteringType = player;
        
        if( playerHealth > 0 && 
            currentLevel->isEnemy( mousePos, &itemIndex ) ) {
            
            enteringPos = currentLevel->getEnemyCenter( itemIndex );
            enteringHit = true;
            enteringType = enemy;
            }
        else if( playerHealth == 0 ||
                 currentLevel->isPlayer( mousePos ) ) {
            enteringPos = playerPos;
            enteringHit = true;
            enteringType = player;
            }
        else if( currentLevel->isPowerUp( mousePos, &itemIndex ) ) {
            enteringPos = currentLevel->getPowerUpCenter( itemIndex );
            enteringHit = true;
            enteringType = power;
            }
        

        if( enteringHit ) {
            levelRiseStack.push_back( currentLevel );
            // enemy or player is entry position
            LevelPositionInfo info = 
                { playerPos, lastScreenViewCenter, 
                  enteringPos, enteringType, mousePos };
            levelRisePositionInfoStack.push_back( info );

            lastLevel = currentLevel;
            lastLevel->freezeLevel( true );

            lastLevelPosition = info;
            zoomProgress = 0;
            zoomDirection = 1;
            
            struct mallinfo meminfo = mallinfo();
    
            int oldAllocedBytes = meminfo.uordblks;
            
            ColorScheme c = 
                currentLevel->getEnteringPointColors( mousePos, enteringType );

            int subLevelNumber =
                currentLevel->getEnteringPointSubLevel( mousePos, 
                                                        enteringType );

            char symmetrical = ( enteringType == player || 
                                 enteringType == power );
            
            currentLevel = new Level( NULL, &c, subLevelNumber,
                                      symmetrical );
            
            meminfo = mallinfo();
            printf( "Level construction used %d kbytes (%d tot)\n",
                    (meminfo.uordblks - oldAllocedBytes ) / 1024,
                    meminfo.uordblks / 1024 );

            if( symmetrical ) {
                playerPos.x = -0.5;
                playerPos.y = 0;
                mousePos.x = -0.5;
                mousePos.y = 0;

                lastScreenViewCenter.x = -0.5;
                lastScreenViewCenter.y = 0;
                }
            else {
                // safe, since -0.5 might be out of bounds
                playerPos.x = 0;
                playerPos.y = 0;
                mousePos.x = 0;
                mousePos.y = 0;

                lastScreenViewCenter.x = 0;
                lastScreenViewCenter.y = 0;
                }
            
            setViewCenterPosition( 0, 0 );
            currentLevel->drawFloorEdges( false );
            }
        
        }
    
    if( currentLevel->isRiseSpot( playerPos ) && lastLevel == NULL ) {
        
        if( levelRiseStack.size() == 0 ) {
            printf( "WARNING:  level stack empty unexpectedly\n" );            
            populateLevelRiseStack();
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
        
        lastLevel->decompactLevel();

        lastLevel->freezeLevel( true );
        lastLevel->restorePlayerHealth();
        zoomProgress = 1;
        zoomDirection = -1;
        currentLevel->drawFloorEdges( false );
        }





    if( currentLevel->isPowerUp( playerPos ) ) {
        
        PowerUp p = currentLevel->getPowerUp( playerPos );
        
        Level *nextAbove = getNextAbove();
        
        PowerUpSet *s = nextAbove->getLastEnterPointPowers();

        s->pushPower( p );
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





    // move screen to follow player and mouse


    // tweak screen center to account for dashboard
    doublePair tweakedScreenViewCenter = lastScreenViewCenter;
    tweakedScreenViewCenter.y -= dashHeight / 2;

    double tweakedViewHeightFraction = 
        (viewHeightFraction * viewWidth - dashHeight) / viewWidth;

    
    // between player and reticle, closer to player (to keep player on screen)
    
    doublePair posToCenterOnScreen = mousePos;
    
    // increase player weight as player moves farther from screen center 
    double playerPosWeightX = 
        1 + 0.4 * sqrt( fabs( playerPos.x - tweakedScreenViewCenter.x ) );
    // closer to player in vertical direction, because screen is shorter
    double playerPosWeightY = 
        1 + 0.4 
        * sqrt( fabs( playerPos.y - tweakedScreenViewCenter.y ) 
                / tweakedViewHeightFraction);
    
    posToCenterOnScreen.x += playerPosWeightX * playerPos.x;
    posToCenterOnScreen.y += playerPosWeightY * playerPos.y;
    
    posToCenterOnScreen.x /= 1 + playerPosWeightX;
    posToCenterOnScreen.y /= 1 + playerPosWeightY;





    double screenCenterDistanceFromPlayerX = 
        fabs( posToCenterOnScreen.x - tweakedScreenViewCenter.x );
    double screenCenterDistanceFromPlayerY = 
        fabs( posToCenterOnScreen.y - tweakedScreenViewCenter.y );

    double minDistanceToMoveScreenX = 
        0.2 * ( viewWidth ) / 2;
    double minDistanceToMoveScreenY = 
        0.2 * ( viewWidth * tweakedViewHeightFraction ) / 2;

    // stop move screen whenever position to center is inside the center 
    // rectangle (separate threshold for x and y)
    if( screenCenterDistanceFromPlayerX > 
        minDistanceToMoveScreenX ||
        screenCenterDistanceFromPlayerY > 
        minDistanceToMoveScreenY ) {

        doublePair screenMoveDelta = sub( posToCenterOnScreen, 
                                          tweakedScreenViewCenter );
        
        // set correction speed based on how far off we are from VERY CENTER
        // since we stop moving when player inside center box, this eliminates
        // jerky micro-movements.
        double correctionSpeedX = 
            0.005 * 
            pow(
                (screenCenterDistanceFromPlayerX - 0),
                2 );
        double correctionSpeedY = 
            0.005 *
            pow(
                (screenCenterDistanceFromPlayerY - 0) 
                / tweakedViewHeightFraction,
                2 );
        
        screenMoveDelta.x *= correctionSpeedX;
        screenMoveDelta.y *= correctionSpeedY;

        /*
          // not actually seeing any round-off errors.
          // hold off on doing this for now
        
        // round to closest 1/32 (16 pixels per world square, double size)
        screenMoveDelta.y *= 32;
        screenMoveDelta.y = round( screenMoveDelta.y ) / 32;
        screenMoveDelta.x *= 32;
        screenMoveDelta.x = round( screenMoveDelta.x ) / 32;
        */

        lastScreenViewCenter = add( lastScreenViewCenter, screenMoveDelta );
        
        

        setViewCenterPosition( lastScreenViewCenter.x, 
                               lastScreenViewCenter.y );

        if( ! shooting ) {
            // move mouse with screen
            mousePos.x += screenMoveDelta.x;
            mousePos.y += screenMoveDelta.y;
            }
        }

    
    if( viewDelta.x != 0 || viewDelta.y != 0 ) {
        confineMouseOnScreen();
        }
    

    if( shooting ) {
        if( stepsTilNextBullet == 0 ) {
            // fire bullet

            double mouseDist = distance( mousePos, playerPos );
            doublePair bulletVelocity = sub( mousePos, playerPos );
            
            if( mouseDist > 0 ) {                
                // normalize
                bulletVelocity.x /= mouseDist;
                bulletVelocity.y /= mouseDist;
                }
            else {
                bulletVelocity.x = 0;
                bulletVelocity.y = 1;
                }            

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





    // now draw stuff AFTER all updates


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

        lastLevel->setItemWindowPosition( lastLevelPosition.entryPosition,
                                          lastLevelPosition.entryType );
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

    
    if( lastRiseFreezeFrameDrawn ) {
        lastRiseFreezeFrameDrawn = false;
        currentLevel->freezeLevel( false );
        }
    else if( secondToLastRiseFreezeFrameDrawn ) {
        secondToLastRiseFreezeFrameDrawn = false;
        lastRiseFreezeFrameDrawn = true;
        }
    else {
        // okay to pass player movement to level
        currentLevel->setMousePos( mousePos );
        currentLevel->setPlayerPos( playerPos );
        currentLevel->setEnteringMouse( entering );
        }
    currentLevel->drawLevel( lastScreenViewCenter, viewSize );

    if( stencilDrawn ) {
        
        
        if( lastLevel != NULL ) {
            setViewSize( lastLevelCurrentViewSize );
            setViewCenterPosition( lastLevelCurrentViewCenter.x,
                                   lastLevelCurrentViewCenter.y );
            
            // fade frame just at tail end of zoom (to ensure that any
            // visible parts of frame don't pop out at end)
            double frameFade = ( 1 - zoomProgress ) / 0.25;
            if( frameFade > 1 ) {
                frameFade = 1;
                }
            
            double centerFade = 1 - zoomProgress;
            
            lastLevel->drawWindowShade( centerFade, frameFade );
            }

        // step zoom and check for zoom end

        zoomProgress += zoomSpeed * zoomDirection;
        
        if( zoomProgress >= 1 && zoomDirection == 1) {
            
            struct mallinfo meminfo = mallinfo();
    
            int oldAllocedBytes = meminfo.uordblks;
            
            // going down, compact it first
            lastLevel->compactLevel();

            meminfo = mallinfo();
            printf( "Level compaction used %d kbytes (%d tot)\n",
                    (meminfo.uordblks - oldAllocedBytes ) / 1024,
                    meminfo.uordblks / 1024 );

            lastLevel = NULL;

            // go with current level
            setViewSize( viewWidth );
            setViewCenterPosition( lastScreenViewCenter.x, 
                                   lastScreenViewCenter.y );
            
            currentLevel->drawFloorEdges( true );
            
            levelNumber = currentLevel->getLevelNumber();
            }
        else if( zoomProgress <= 0 && zoomDirection == -1 ) {
            
            // done with current level
            delete currentLevel;
            
            // switch to last level (zooming out)
            currentLevel = lastLevel;
            
            // don't unfreeze yet, still drawing final zoom-out frames
            //currentLevel->freezeLevel( false );
            currentLevel->forgetItemWindow();
            playerPos = lastLevelPosition.playerPos;
            lastScreenViewCenter = lastLevelPosition.lastScreenViewCenter;
            mousePos = lastLevelPosition.mousePos;
            

            lastLevel = NULL;
        
            setViewSize( viewWidth );
            setViewCenterPosition( lastScreenViewCenter.x, 
                                   lastScreenViewCenter.y );

            // wait until end of freeze to do this, so that it
            // syncs with dashboard sprite change
            //levelNumber += 1;


            // take time to populate level rise stack after this frame is drawn
            // to hide hiccup
            secondToLastRiseFreezeFrameDrawn = true;            
            }    
        }



    // draw dashboard
    setViewSize( viewWidth );
    setViewCenterPosition( lastScreenViewCenter.x, 
                           lastScreenViewCenter.y );
    
    // border around panel
    setDrawColor( 0.3, 0.3, 0.3, 1 );
    
    

    drawRect( lastScreenViewCenter.x - viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 ,
              lastScreenViewCenter.x + viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 - dashHeight - 0.0625 );

    // body of panel
    setDrawColor( 0, 0, 0, 1 );
    
    

    drawRect( lastScreenViewCenter.x - viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 ,
              lastScreenViewCenter.x + viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 - dashHeight );


    // level number display on dash
    setDrawColor( 1, 1, 1, 0.5 );
    
    doublePair levelNumberPos = { lastScreenViewCenter.x +
                                  viewWidth /2,
                                  lastScreenViewCenter.y +
                                  viewHeightFraction * viewWidth /2 - 0.625 };
    
    
    char *levelString = autoSprintf( "%d", levelNumber );
    
    mainFont->drawString( levelString, levelNumberPos, alignRight );
    
    delete [] levelString;


    Level *nextAbove = getNextAbove();
    
        
        

    BorderSprite *weAreInsideSprite = nextAbove->getLastEnterPointSprite();
    
    
    doublePair spritePos = levelNumberPos;
    spritePos.x = lastScreenViewCenter.x - viewWidth/2 + 1.25;
    spritePos.y += 0.125;
    
    weAreInsideSprite->draw( spritePos );

    doublePair markerPos = spritePos;
    markerPos.x -= 0.875;
    
    drawSprite( riseIcon, markerPos );
    
    
    PowerUpSet *p = nextAbove->getLastEnterPointPowers();
    doublePair setPos = spritePos;
    setPos.x += 2.25;

    p->drawSet( setPos );
    

    
    Level *levelToGetCurrentFrom;
    if( lastLevel != NULL && zoomDirection == 1 ) {
        levelToGetCurrentFrom = lastLevel;
        }
    else {
        levelToGetCurrentFrom = currentLevel;
        }
    
    
    PowerUpSet playerPowers = levelToGetCurrentFrom->getPlayerPowers();
    setPos = spritePos;
    setPos.x = lastScreenViewCenter.x;

    playerPowers.drawSet( setPos );

    PlayerSprite *playerSprite = levelToGetCurrentFrom->getPlayerSprite(); 

    spritePos = setPos;
    spritePos.x += 2.25;
    
    playerSprite->draw( spritePos );
    

    // health bar
    
    setDrawColor( 0.25, 0.25, 0.25, 1 );
    drawRect( spritePos.x + 0.75, spritePos.y - 0.25, 
              spritePos.x + 2.75, spritePos.y + 0.25 );

    levelToGetCurrentFrom->getPlayerHealth( &playerHealth, &playerMax );
    float playerHealthFraction = playerHealth / (float)playerMax;
    //printf( "h, m = %d, %d\n", playerHealth, playerMax );

    setDrawColor( 0, 0, 0, 1 );
    drawRect( spritePos.x + 0.875, spritePos.y - 0.125, 
              spritePos.x + 0.875 + 1.75, 
              spritePos.y + 0.125 );

    
    setDrawColor( 1, 0, 0, 1 );
    drawRect( spritePos.x + 0.875, spritePos.y - 0.125, 
              spritePos.x + 0.875 + 1.75 * playerHealthFraction, 
              spritePos.y + 0.125 );


    
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


        mousePos.x += mouseSpeed * deltaX;
        mousePos.y -= mouseSpeed * deltaY;
        

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
        case 'm':
        case 'M': {
            struct mallinfo meminfo = mallinfo();
            printf( "Mem alloc: %d\n",
                    meminfo.uordblks / 1024 );
            }
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






    



