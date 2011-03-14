/*
 * Modification History
 *
 * 2008-September-11  Jason Rohrer
 * Created.  Copied from Cultivation.
 */


int versionNumber = 16;



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//#define USE_MALLINFO

#ifdef USE_MALLINFO
#include <malloc.h>
#endif


#include "minorGems/graphics/Color.h"




#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
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
#include "bulletSizeSet.h"
#include "powerUpProperties.h"
#include "RandomWalkerSet.h"
#include "tutorial.h"
#include "tipDisplay.h"
#include "setTipDisplay.h"
#include "musicPlayer.h"
#include "numerals.h"
#include "beatTracker.h"


// should we output level maps as images?
char outputMapImages = false;

// should player move toward rise marker automatically?
char enableRobotPlayer = false;



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

double moveSpeed = 0.125;

// one true screen pixel (1/32)
double moveAccel = 0.03125;
//double moveAccel = 0.0625;
//double moveAccel = 0.125;

// as frame rate decreases, accel frames decrease, too
// at full frame rate, we use four accel frames to reach speed
// (reach speed on 4th frame after key press)
// but by 1/4 framerate, we revert to instant-accel 
//   (instant-accel means  moveAccel = moveSpeed)
int numAccelFrames = 4;



double accelX = 0;
double accelY = 0;



double frameRateFactor = 1;


char forceRepeatRandSeed = false;
unsigned int randSeed = 1285702442;
CustomRandomSource randSource(randSeed);


char shooting = false;
int stepsTilNextBullet = 0;
int stepsBetweenBullets = 5;

char playerCorneringDir = false;

char entering = false;



char firstDrawFrameCalled = false;


// if sound wasn't opened properly, our music player won't generate beatHit
// calls.
// we must fake them every second so our "path to rise marker" 
// beacon flashes appropriately 
int stepsSinceLastFakeBeat = 0;



const char *getWindowTitle() {
    return "Game 10";
    }


const char *getFontTGAFileName() {
    return "font_8_16.tga";
    }


char isDemoMode() {
    return false;
    }


const char *getDemoCodeSharedSecret() {
    return "green_stone_table";
    }


const char *getDemoCodeServerURL() {
    return "http://insideastarfilledsky.net/demoServer/server.php";
    }



int levelNumber = 1;

int extraDifficultyNumber = 0;

Color levelNumberColor( 1, 1, 1, .5 );


char gamePlayingBack = false;



Level *currentLevel;

SimpleVector<Level *> levelRiseStack;


typedef struct LevelPositionInfo {
        doublePair playerPos;
        doublePair lastScreenViewCenter;
        doublePair entryPosition;
        itemType entryType;
        doublePair mousePos;
        // for rewinding on rise-out of knock-down
        // used only for entering token or enemy
        PowerUpSet enterPointPowerStartState;
    } LevelPositionInfo;

SimpleVector<LevelPositionInfo> levelRisePositionInfoStack;


// for zooming into new level
Level *lastLevel = NULL;
LevelPositionInfo lastLevelPosition;
doublePair lastLevelCurrentViewCenter;
double lastLevelCurrentViewSize;


double zoomProgress = 0;
double zoomSpeed = 0.01;
double zoomDirection = 1;


Font *levelNumberFont;
Font *levelNumberReducedFont;

Font *mainFont2;

Font *tinyFont;


static float pauseScreenFade = 0;

static char *currentUserTypedMessage = NULL;

// for delete key repeat during message typing
static int holdDeleteKeySteps = -1;
static int stepsBetweenDeleteRepeat;



char *tutorialMoveKeys;


static PowerUpSet defaultSet;


static void addFreshLevelToStack( unsigned int inSeed ) {
    Level *levelRightBelow = currentLevel;
    
    if( levelRiseStack.size() > 0 ) {

        levelRightBelow = *( levelRiseStack.getElement( 0 ) );
        }
    
    ColorScheme c = levelRightBelow->getLevelColors();
    NoteSequence s = levelRightBelow->getLevelNoteSequence();

    RandomWalkerSet ourPlayerWalkerSet = levelRightBelow->getLevelWalkerSet();

    ColorScheme freshColors;
    // copy player part's timbre/envelope
    // alternate part length with player part for phase patterns
    int partLength = 16;
    if( s.partLength == partLength ) {
        partLength -= 4;
        }
    NoteSequence freshNotes = generateRandomNoteSequence( PARTS - 2,
                                                          partLength );
        
    int freshLevelNumber = levelRightBelow->getLevelNumber() + 1;
    

    levelRiseStack.push_front( new Level( inSeed, 
                                          &c, &s, &freshColors,
                                          NULL,
                                          &ourPlayerWalkerSet,
                                          &freshNotes,
                                          NULL,
                                          freshLevelNumber ) );
        
        
    // center player in symmetrical level
    LevelPositionInfo info = 
        { {-0.5,0}, {-0.5,0}, {-0.5,0}, player, {-0.5, 0}, defaultSet };
    levelRisePositionInfoStack.push_front( info );    
    }



// non-null seed vector to override default behavior
static void populateLevelRiseStack( 
    SimpleVector<unsigned int> *inSeeds = NULL ) {
    
    if( inSeeds != NULL ) {
        
        int numSeeds = inSeeds->size();
        
        for( int i=0; i<numSeeds; i++ ) {
            addFreshLevelToStack( *( inSeeds->getElement( i ) ) );
            }        

        return;
        }
    


    // else default behavior, make sure stack has at least two levels in it

    if( levelRiseStack.size() == 0 ) {
        // push one on to rise into
        
        // base pre-seed on level below's seed
        
        unsigned int belowSeed = currentLevel->getSeed();

        randSource.restoreFromSavedState( belowSeed );
        
        // draw seed
        int newSeed = randSource.getRandomInt();

        addFreshLevelToStack( newSeed );
        }

    if( levelRiseStack.size() == 1 ) {
        // always have two to rise into
        Level *nextBelow = *( levelRiseStack.getElement( 0 ) );
        
        // base pre-seed on level below's seed
        
        unsigned int belowSeed = nextBelow->getSeed();
        
        randSource.restoreFromSavedState( belowSeed );
                
        // draw seed
        int newSeed = randSource.getRandomInt();

        addFreshLevelToStack( newSeed );
        }        
    }


#define SETTINGS_HASH_SALT  "gazing upward"


int getStartingLevelNumber() {
    
    
    // defaults to 9 if hash fails and tutorial completed

    int defaultLevel = 9;
    
    int tutorialCompletedCount = 
        SettingsManager::getIntSetting( "tutorialCompletedCount", 0 );

    if( tutorialCompletedCount == 0 ) {
        defaultLevel = 1;
        }


    SettingsManager::setHashSalt( SETTINGS_HASH_SALT );
    
    SettingsManager::setHashingOn( true );

    char *bookmarkString = SettingsManager::getStringSetting( "bookmark" );
    
    int startingLevelNumber = defaultLevel;

    if( bookmarkString != NULL ) {
        sscanf( bookmarkString, "level%d_", &startingLevelNumber );
        delete [] bookmarkString;
        }
        
    SettingsManager::setHashingOn( false );    
        
    return startingLevelNumber;    
    }





static const char *customDataFormatString = 
    "version%d_level%d_tutorial%d_tutorialBookmark%d_mouseSpeed%f_%s";


char *getCustomRecordedGameData() {


    /*
    // for faking bookmarks

    SettingsManager::setHashSalt( SETTINGS_HASH_SALT );
    
    SettingsManager::setHashingOn( true );

    SettingsManager::setSetting( "bookmark", 
                                 "level75_[12]powerUpSpread#[12]powerUpBulletDistance#[9]powerUpBounce_levelStack3_3025385482_3451724343_720135969" );

    SettingsManager::setHashingOn( false );
    */



    int completedCount = 
        SettingsManager::getIntSetting( "tutorialCompletedCount", 0 );

    int tutorialOn = 1;

    if( completedCount >= 1 ) {
        tutorialOn = 0;
        }

    // don't force player back through boring level 0, even if player
    // has not completed tutorial
    levelNumber = getStartingLevelNumber();

    int tutorialBookmark = SettingsManager::getIntSetting( 
        "tutorialBookmark", 0 );
    
    
    float mouseSpeedSetting = 
        SettingsManager::getFloatSetting( "mouseSpeed", 1.0f );



    SettingsManager::setHashSalt( SETTINGS_HASH_SALT );
    
    SettingsManager::setHashingOn( true );

    char *fullBookmarkString = SettingsManager::getStringSetting( "bookmark" );
    char *partialBookmarkString = NULL;
    
    if( fullBookmarkString != NULL ) {
        
        // skip level number, since we already have that in our custom data
        // below
        char *skipPointer = strstr( fullBookmarkString, "_" );
        
        if( skipPointer != NULL ) {
            
            skipPointer = &( skipPointer[1] );
            
            partialBookmarkString = stringDuplicate( skipPointer );
            }
                
        delete [] fullBookmarkString;
        }
    
    if( partialBookmarkString == NULL ) {
        partialBookmarkString = stringDuplicate( "default" );
        }

    SettingsManager::setHashingOn( false );


    char * result = autoSprintf(
        customDataFormatString,
        versionNumber, 
        levelNumber, tutorialOn,
        tutorialBookmark, mouseSpeedSetting,
        partialBookmarkString );
    
    delete [] partialBookmarkString;

    return result;
    }



char *getHashSalt() {
    return stringDuplicate( SETTINGS_HASH_SALT );
    }



// pass in NULL to init with default
static void initStartingLevels( SimpleVector<unsigned int> *inSeeds = NULL ) {

    if( inSeeds == NULL ) {
        
        // random seed
        currentLevel = new Level( randSource.getRandomInt(),
                                  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                                  levelNumber );
    
        populateLevelRiseStack();
        }
    else {
        // first seed for current level
        unsigned int firstSeed = *( inSeeds->getElement( 0 ) );

        currentLevel = new Level( firstSeed, 
                                  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                  levelNumber );
    
        // rest of seeds into rise stack
        inSeeds->deleteElement( 0 );
        
        populateLevelRiseStack( inSeeds );
        }
    
    }





void initFrameDrawer( int inWidth, int inHeight, int inTargetFrameRate,
                      const char *inCustomRecordedGameData,
                      char inPlayingBack ) {
    gamePlayingBack = inPlayingBack;
    
    screenW = inWidth;
    screenH = inHeight;
    
    if( inTargetFrameRate != 60 ) {
        frameRateFactor = (double)60 / (double)inTargetFrameRate;
        }
    
    moveSpeed *= frameRateFactor;
    
    numAccelFrames = (int)( numAccelFrames / frameRateFactor );

    if( numAccelFrames <= 0 ) {
        numAccelFrames = 1;
        }

    moveAccel = moveSpeed / numAccelFrames;
    
    printf( "Player move speed = %f, acceleration = %f at framerate %d/s\n",
            moveSpeed, moveAccel, inTargetFrameRate );


    tutorialMoveKeys = stringDuplicate( "W A S D" );


    if( !forceRepeatRandSeed ) {
        randSeed = getRandSeed();
        
        CustomRandomSource newRandSource( randSeed );
        randSource = newRandSource;
        }
    
    
    printf( "Rand seed = %d\n", randSeed );
    

    int outputMapsSetting = 
        SettingsManager::getIntSetting( "outputMapImages", 0 );
    
    if( outputMapsSetting == 1 ) {
        outputMapImages = true;
        }

    int enableRobotPlayerSetting = 
        SettingsManager::getIntSetting( "enableRobotPlayer", 0 );
    
    if( enableRobotPlayerSetting == 1 ) {
        enableRobotPlayer = true;
        }
    


    


    setViewCenterPosition( lastScreenViewCenter.x, lastScreenViewCenter.y );

    viewHeightFraction = inHeight / (double)inWidth;

    // monitors vary in width relative to height
    // keep visible vertical view span constant (15)
    // which is what it would be for a view width of 20 at a 4:3 aspect
    // ratio
    viewWidth = 15 * 1.0 / viewHeightFraction;
    
    
    setViewSize( viewWidth );


    
    

    

    setCursorVisible( false );
    grabInput( true );
    
    // raw screen coordinates
    setMouseReportingMode( false );
    
    int x,y;
    warpMouseToCenter( &x, &y );
    
    
    initSpriteBank();
    initBulletSizeSet();
    
    levelNumberFont = new Font( getFontTGAFileName(), -2, 4, true, 2.0 );
    levelNumberReducedFont = 
        new Font( getFontTGAFileName(), -2, 4, true, 1.0 );

    mainFont2 = new Font( getFontTGAFileName(), 1, 4, false );
    
    tinyFont = new Font( "font_4_8.tga", 1, 2, false );

    initNumerals( "numerals.tga" );
    

    // override default with data from recorded game
    int tutorialOn = 0;
    int tutorialBookmark = 0;
    float mouseSpeedSetting = 1.0f;
    char *partialBookmarkString = 
        new char[ strlen( inCustomRecordedGameData ) + 1 ];
    
    currentLevel = NULL;

    int readVersionNumber;
    
    int numRead = sscanf( inCustomRecordedGameData, 
                          customDataFormatString, 
                          &readVersionNumber,
                          &levelNumber,
                          &tutorialOn, &tutorialBookmark, &mouseSpeedSetting,
                          partialBookmarkString );
    
    PowerUpSet *startingPowers = NULL;

    
    // default to empty (default level generation)
    // unless overridden by bookmark or recorded game
    SimpleVector<unsigned int> levelStackSeeds;
                    


    if( numRead != 6 ) {
        // no recorded game?

        // don't force player back through boring level 0, even if
        // player hasn't finished tutorial yet
        levelNumber = getStartingLevelNumber();
        }
    else {
        // override with passed-in values
        
        if( readVersionNumber != versionNumber ) {
            AppLog::printOutNextMessage();
            AppLog::getLog()->logPrintf( 
                Log::WARNING_LEVEL,
                "WARNING:  version number in playback file is %d "
                "but game version is %d...",
                readVersionNumber, versionNumber );
            }


        // (keep levelNumber that was read from data)

        if( !tutorialOn ) {
            forceTutorialEnd();
            }
        else if( tutorialBookmark == 0 ) {
            forceTutorialFreshStart();
            }
        else {
            forceTutorialBookmark( tutorialBookmark );
            }        

        if( strcmp( partialBookmarkString, "default" ) != 0 ) {
            
            // split at first _
            char *splitPoint = strstr( partialBookmarkString, "_" );
            
            if( splitPoint != NULL ) {
                
                // first terminate at skip point for power string
                splitPoint[0] = '\0';
                startingPowers = new PowerUpSet( partialBookmarkString );

                // then skip that point for stack string
                
                char *levelStackString = &( splitPoint[1] );
            
                int numStackElements = 0;
                
                sscanf( levelStackString, "levelStack%d_", &numStackElements );
                
                if( numStackElements > 0 ) {
                    
                    for( int s=0; s<numStackElements; s++ ) {
                        
                        char *splitPoint = strstr( levelStackString, "_" );
                        
                        if( splitPoint != NULL ) {
                            levelStackString = &( splitPoint[1] );
                            
                        
                            unsigned int scannedNumber;
                            int numRead = sscanf( levelStackString, "%u",
                                                  &scannedNumber );
                            if( numRead == 1 ) {
                                
                                levelStackSeeds.push_back( scannedNumber );
                                }
                            }
                        }
                    
                    }
    
                }
            
            

            }
        }
    
    delete [] partialBookmarkString;


    
    double mouseParam = 0.000976562;

    mouseParam *= mouseSpeedSetting;

    mouseSpeed = mouseParam * inWidth / viewWidth;




    checkTutorial();
    

    initTipDisplay();
    

    if( levelStackSeeds.size() > 2 ) {
        // pre-defined seeds from bookmark or recorded game
        // (must have at least 3, current level and 2 in stack, to be valid)
        initStartingLevels( &levelStackSeeds );
        }
    else {
        // default
        initStartingLevels();
        }
           
        
    if( startingPowers != NULL ) {
        currentLevel->setPlayerPowers( startingPowers );

        // pass up to next level, too, decayed version
        startingPowers->sortPowersRight();
        startingPowers->decayPowers();
        
        // sort right again, incase gap left by left-most hearts
        startingPowers->sortPowersRight();

        Level *nextHigherLevel = 
            *( levelRiseStack.getElement( levelRiseStack.size() - 1 ) );

        nextHigherLevel->setPlayerPowers( startingPowers );
        

        delete startingPowers;
        }
    


    // for level construction optimization
    /*
    if( false ) {
        
        double msTime = Time::getCurrentTime();
        
        for( int i=0; i<100; i++ ) {
            Level *l = new Level( randSource.getRandomInt() );
            delete l;
            }
        printf( "Contstructing levels took %f s\n",
                (Time::getCurrentTime() - msTime) );
    
        //exit(0);
        }
    */


    initMusicPlayer();
    setMusicLoudness( 0 );

    /*
    // demo:  fill grid with random
    for( int p=0; p<PARTS; p++ ) {
        //partLengths[p] = randSource.getRandomBoundedInt( 5, N );
        // try no phase shifting...
        partLengths[p] = N;

        int numNotesInPart = 0;
        while( numNotesInPart < 2 ) {
            for( int x=0; x<partLengths[p]; x++ ) {
                if( randSource.getRandomBoundedInt( 0, 10 ) > 8 ) {        
                    // at most one note in each timbre-column
                    int y = randSource.getRandomBoundedInt( 0, N - 1 );
                    noteToggles[p][y][x] = true;
                    numNotesInPart++;
                    }
                }
            }
            
        }
    */
    // load current level's music
    currentLevel->pushAllMusicIntoPlayer();

    setSoundPlaying( true );
    }


void freeFrameDrawer() {
    delete currentLevel;
    
    if( lastLevel != NULL ) {
        // remove from stack first, if it's there, to avoid double-delete
        // below
        levelRiseStack.deleteElementEqualTo( lastLevel );
        
        delete lastLevel;
        }
    

    delete [] tutorialMoveKeys;
    

    freeSpriteBank();
    freeBulletSizeSet();
    
    freeNumerals();
    
    freeTutorial();
    
    freeTipDisplay();
    freeSetTipDisplay();
    

    delete levelNumberFont;
    delete levelNumberReducedFont;
    delete mainFont2;
    delete tinyFont;
    
    if( currentUserTypedMessage != NULL ) {
        delete [] currentUserTypedMessage;
        currentUserTypedMessage = NULL;
        }
    

    for( int i=0; i<levelRiseStack.size(); i++ ) {
        delete *( levelRiseStack.getElement( i ) );
        }
    levelRiseStack.deleteAll();    
    levelRisePositionInfoStack.deleteAll();

    freeMusicPlayer();
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



// returns maximum x extent (to right) of health bar
static double drawHealthBar( doublePair inBarLeftEdge, float inHealthFraction,
                             int inMaxSegments,
                             float inFade ) {
    
    double redBarWidth;
    
    double segmentWidth = 0.5;

    char drawSegments = false;
    
    if( inMaxSegments < 10 ) {

        //if( inMaxSegments

        redBarWidth = segmentWidth * inMaxSegments;
        
        drawSegments = true;
        }
    else {
        redBarWidth = segmentWidth * 9;
        }
    
            

    // .125 border on both ends
    double fullBarWidth = redBarWidth + 0.25;
    

    
    // background
    setDrawColor( 0.25, 0.25, 0.25, inFade );
    // four rects that serve as borders without overlapping filled area
    // (so whole bar can fade in on top of another, even if they are
    //  different lengths)
    // left
    drawRect( inBarLeftEdge.x, inBarLeftEdge.y - 0.25, 
              inBarLeftEdge.x + 0.125, inBarLeftEdge.y + 0.25 );
    // right
    drawRect( inBarLeftEdge.x + fullBarWidth - 0.125, 
              inBarLeftEdge.y - 0.25, 
              inBarLeftEdge.x + fullBarWidth, inBarLeftEdge.y + 0.25 );

    // top
    drawRect( inBarLeftEdge.x + 0.125, inBarLeftEdge.y - 0.25, 
              inBarLeftEdge.x + fullBarWidth - 0.125, 
              inBarLeftEdge.y - 0.125 );
    
    // bottom
    drawRect( inBarLeftEdge.x + 0.125, inBarLeftEdge.y + 0.125, 
              inBarLeftEdge.x + fullBarWidth - 0.125, 
              inBarLeftEdge.y + 0.25 );
    
            
    // filling behind red
    setDrawColor( 0, 0, 0, inFade );
    drawRect( inBarLeftEdge.x + 0.125 + redBarWidth * inHealthFraction, 
              inBarLeftEdge.y - 0.125, 
              inBarLeftEdge.x + 0.125 + redBarWidth, 
              inBarLeftEdge.y + 0.125 );

    
    if( !drawSegments ) {
        // solid bar
        setDrawColor( 0.85, 0, 0, inFade );
        drawRect( inBarLeftEdge.x + 0.125, inBarLeftEdge.y - 0.125, 
                  inBarLeftEdge.x + 0.125 + redBarWidth * inHealthFraction, 
                  inBarLeftEdge.y + 0.125 );
        }
    else {
        // segments
      
        int numSegments = (int)( roundf( inHealthFraction * inMaxSegments ) );
        
        char swapColor = true;
        
        for( int i=0; i<numSegments; i++ ) {
            
            if( swapColor ) {
                setDrawColor( 0.85, 0, 0, inFade );
                }
            else {
                setDrawColor( 0.65, 0, 0, inFade );
                }
            swapColor = !swapColor;
            
            
            drawRect( 
                inBarLeftEdge.x + 0.125 + i * segmentWidth, 
                inBarLeftEdge.y - 0.125, 
                inBarLeftEdge.x + 0.125 + i * segmentWidth + segmentWidth, 
                inBarLeftEdge.y + 0.125 );
            }
        }

    return inBarLeftEdge.x + fullBarWidth;

    /*
    // segment markers
    if( inMaxSegments < 10 ) {
        
        setDrawColor( 0, 0, 0, inFade );

        for( int i=0; i<=inMaxSegments; i++ ) {
            
            double barCenterX = ( 1.75 / inMaxSegments ) * i;
            
            barCenterX += inBarLeftEdge.x + 0.125;

            drawRect( barCenterX - 0.03125, inBarLeftEdge.y - 0.125,
                      barCenterX + 0.03125, inBarLeftEdge.y + 0.125 );
            }
        }
    */
    }



static void saveLevelBookmark() {
    // only save bookmark to this level if local player is actually playing
    // (not for playback games from others)
    // also, never trap player down in (or near) negative levels 
    // after they quit
    if( !gamePlayingBack && levelNumber >= 1 ) {
        SettingsManager::setHashSalt( SETTINGS_HASH_SALT );
    
        SettingsManager::setHashingOn( true );
        

        // compose all starting information into a string
        // thus, it can be hashed together

        char *powersAsString =
            currentLevel->getPlayerPowers()->getStringEncoding();
        
        SimpleVector<char*> levelSeedStrings;
        
        levelSeedStrings.push_back( 
            autoSprintf( "%u", currentLevel->getSeed() ) );
        
        for( int i=levelRiseStack.size()-1; i>=0; i-- ) {
            Level *stackLevel = *( levelRiseStack.getElement( i ) );
                    
            levelSeedStrings.push_back( 
                autoSprintf( "%u", stackLevel->getSeed() ) );
            }

        char **stringArray = levelSeedStrings.getElementArray();
        
        char *levelStackString = join( stringArray, levelSeedStrings.size(),
                                       "_" );
        
        for( int i=0; i<levelSeedStrings.size(); i++ ) {
            delete [] stringArray[i];
            }
        delete [] stringArray;


        char *bookmarkData = autoSprintf( "level%d_%s_levelStack%d_%s",
                                          levelNumber,
                                          powersAsString,
                                          levelSeedStrings.size(),
                                          levelStackString );
        
        delete [] powersAsString;
        delete [] levelStackString;
        

        SettingsManager::setSetting( "bookmark", bookmarkData );
        
        delete [] bookmarkData;


        SettingsManager::setHashingOn( false );
        }
    }



static void updateLevelNumber() {
    levelNumber = currentLevel->getLevelNumber();
    
    int levelDifficulty = currentLevel->getDifficultyLevel();
    
    extraDifficultyNumber = 0;
    
    if( levelDifficulty != (int)fabs( levelNumber ) ) {
        extraDifficultyNumber = levelDifficulty - (int)fabs( levelNumber );
        }


    LevelPositionInfo *lastLevelInfo = 
        levelRisePositionInfoStack.getElement( 
            levelRisePositionInfoStack.size() - 1 );
        
    switch( lastLevelInfo->entryType ) {
        case player:
            levelNumberColor.r = 1;
            levelNumberColor.g = 1;
            levelNumberColor.b = 1;
            levelNumberColor.a = 0.5;
            break;
        case enemy:
            levelNumberColor.r = 1;
            levelNumberColor.g = 0;
            levelNumberColor.b = 0;
            levelNumberColor.a = 0.75;
            break;
        case power:
            if( currentLevel->isInsideEnemy() ) {
                // inside an enemy power
                levelNumberColor.r = 1;
                levelNumberColor.g = .375;
                levelNumberColor.b = 0;
                levelNumberColor.a = 0.75;
                }
            else {
                // inside a player power
                levelNumberColor.r = 1;
                levelNumberColor.g = .75;
                levelNumberColor.b = 0;
                levelNumberColor.a = 0.65;
                }
            break;
        }
    }

    



// used to keep level rise stack populated without a visible frame hiccup
// hide the hiccup right after the final freeze frame of the level
// we're rising into

// turns out there are two final zoom frames at end of rise out
static char secondToLastRiseFreezeFrameDrawn = false;
static char lastRiseFreezeFrameDrawn = false;


// draw code separated from updates
// some updates are still embedded in draw code, so pass a switch to 
// turn them off
static void drawFrameNoUpdate( char inUpdate );




static void drawPauseScreen() {

    double viewHeight = viewHeightFraction * viewWidth;

    setDrawColor( 1, 1, 1, 0.5 * pauseScreenFade );
        
    drawSquare( lastScreenViewCenter, 1.05 * ( viewHeight / 3 ) );
        

    setDrawColor( 0, 0, 0, 0.5 * pauseScreenFade  );
        
    drawSquare( lastScreenViewCenter, viewHeight / 3 );
        

    setDrawColor( 1, 1, 1, pauseScreenFade );

    doublePair messagePos = lastScreenViewCenter;

    messagePos.y += 4.5;

    mainFont2->drawString( translate( "pauseMessage1" ), 
                           messagePos, alignCenter );
        
    messagePos.y -= 1.25 * (viewHeight / 15);
    mainFont2->drawString( translate( "pauseMessage2" ), 
                           messagePos, alignCenter );

    if( currentUserTypedMessage != NULL ) {
            
        messagePos.y -= 1.25 * (viewHeight / 15);
            
        double maxWidth = 0.95 * ( viewHeight / 1.5 );
            
        int maxLines = 8;

        SimpleVector<char *> *tokens = 
            tokenizeString( currentUserTypedMessage );


        // collect all lines before drawing them
        SimpleVector<char *> lines;
        
            
        while( tokens->size() > 0 ) {

            // build up a a line

            // always take at least first token, even if it is too long
            char *currentLineString = 
                stringDuplicate( *( tokens->getElement( 0 ) ) );
                
            delete [] *( tokens->getElement( 0 ) );
            tokens->deleteElement( 0 );
            
            

            

            
            char nextTokenIsFileSeparator = false;
                
            char *nextLongerString = NULL;
                
            if( tokens->size() > 0 ) {

                char *nextToken = *( tokens->getElement( 0 ) );
                
                if( nextToken[0] == 28 ) {
                    nextTokenIsFileSeparator = true;
                    }
                else {
                    nextLongerString =
                        autoSprintf( "%s %s ",
                                     currentLineString,
                                     *( tokens->getElement( 0 ) ) );
                    }
                
                }
                
            while( !nextTokenIsFileSeparator 
                   &&
                   nextLongerString != NULL 
                   && 
                   mainFont2->measureString( nextLongerString ) 
                   < maxWidth 
                   &&
                   tokens->size() > 0 ) {
                    
                delete [] currentLineString;
                    
                currentLineString = nextLongerString;
                    
                nextLongerString = NULL;
                    
                // token consumed
                delete [] *( tokens->getElement( 0 ) );
                tokens->deleteElement( 0 );
                    
                if( tokens->size() > 0 ) {
                    
                    char *nextToken = *( tokens->getElement( 0 ) );
                
                    if( nextToken[0] == 28 ) {
                        nextTokenIsFileSeparator = true;
                        }
                    else {
                        nextLongerString =
                            autoSprintf( "%s%s ",
                                         currentLineString,
                                         *( tokens->getElement( 0 ) ) );
                        }
                    }
                }
                
            if( nextLongerString != NULL ) {    
                delete [] nextLongerString;
                }
                
            while( mainFont2->measureString( currentLineString ) > 
                   maxWidth ) {
                    
                // single token that is too long by itself
                // simply trim it and discard part of it 
                // (user typing nonsense anyway)
                    
                currentLineString[ strlen( currentLineString ) - 1 ] =
                    '\0';
                }
                
            if( currentLineString[ strlen( currentLineString ) - 1 ] 
                == ' ' ) {
                // trim last bit of whitespace
                currentLineString[ strlen( currentLineString ) - 1 ] = 
                    '\0';
                }

                
            lines.push_back( currentLineString );

            
            if( nextTokenIsFileSeparator ) {
                // file separator

                // put a paragraph separator in
                lines.push_back( stringDuplicate( "---" ) );

                // token consumed
                delete [] *( tokens->getElement( 0 ) );
                tokens->deleteElement( 0 );
                }
            }   


        // all tokens deleted above
        delete tokens;


        double messageLineSpacing = 0.625 * (viewHeight / 15);
        
        int numLinesToSkip = lines.size() - maxLines;

        if( numLinesToSkip < 0 ) {
            numLinesToSkip = 0;
            }
        
        
        for( int i=0; i<numLinesToSkip-1; i++ ) {
            char *currentLineString = *( lines.getElement( i ) );
            delete [] currentLineString;
            }
        
        int lastSkipLine = numLinesToSkip - 1;

        if( lastSkipLine >= 0 ) {
            
            char *currentLineString = *( lines.getElement( lastSkipLine ) );

            // draw above and faded out somewhat

            doublePair lastSkipLinePos = messagePos;
            
            lastSkipLinePos.y += messageLineSpacing;

            setDrawColor( 1, 1, 0.5, 0.125 * pauseScreenFade );

            mainFont2->drawString( currentLineString, 
                                   lastSkipLinePos, alignCenter );

            
            delete [] currentLineString;
            }
        

        setDrawColor( 1, 1, 0.5, pauseScreenFade );

        for( int i=numLinesToSkip; i<lines.size(); i++ ) {
            char *currentLineString = *( lines.getElement( i ) );
            
            if( false && lastSkipLine >= 0 ) {
            
                if( i == numLinesToSkip ) {
                    // next to last
                    setDrawColor( 1, 1, 0.5, 0.25 * pauseScreenFade );
                    }
                else if( i == numLinesToSkip + 1 ) {
                    // next after that
                    setDrawColor( 1, 1, 0.5, 0.5 * pauseScreenFade );
                    }
                else if( i == numLinesToSkip + 2 ) {
                    // rest are full fade
                    setDrawColor( 1, 1, 0.5, pauseScreenFade );
                    }
                }
            
            mainFont2->drawString( currentLineString, 
                                   messagePos, alignCenter );

            delete [] currentLineString;
                
            messagePos.y -= messageLineSpacing;
            }
        }
        
        

    setDrawColor( 1, 1, 1, pauseScreenFade );

    messagePos = lastScreenViewCenter;

    messagePos.y -= 3.75 * ( viewHeight / 15 );
    mainFont2->drawString( translate( "pauseMessage3" ), 
                           messagePos, alignCenter );

    messagePos.y -= 0.625 * (viewHeight / 15);
    mainFont2->drawString( translate( "pauseMessage4" ), 
                           messagePos, alignCenter );

    }



void deleteCharFromUserTypedMessage() {
    if( currentUserTypedMessage != NULL ) {
                    
        int length = strlen( currentUserTypedMessage );
        
        char fileSeparatorDeleted = false;
        if( length > 2 ) {
            if( currentUserTypedMessage[ length - 2 ] == 28 ) {
                // file separator with spaces around it
                // delete whole thing with one keypress
                currentUserTypedMessage[ length - 3 ] = '\0';
                fileSeparatorDeleted = true;
                }
            }
        if( !fileSeparatorDeleted && length > 0 ) {
            currentUserTypedMessage[ length - 1 ] = '\0';
            }
        }
    }



static void playerAutoMove();






void drawFrame( char inUpdate ) {

    if( !isSoundRunning() ) {
        // fake beat callbacks
        stepsSinceLastFakeBeat++;
        
        // full second
        if( stepsSinceLastFakeBeat > 60 / frameRateFactor ) {
            beatHit();
            stepsSinceLastFakeBeat = 0;
            }
        }
    


    if( !inUpdate ) {
        char oldFrozen = currentLevel->isFrozen();
        
        currentLevel->freezeLevel( true );

        drawFrameNoUpdate( false );

        currentLevel->freezeLevel( oldFrozen );
        
        
        
        drawPauseScreen();
        
        

        // handle delete key repeat
        if( holdDeleteKeySteps > -1 ) {
            holdDeleteKeySteps ++;
            
            if( holdDeleteKeySteps > stepsBetweenDeleteRepeat ) {        
                // delete repeat

                // platform layer doesn't receive event for key held down
                // tell it we are still active so that it doesn't
                // reduce the framerate during long, held deletes
                wakeUpPauseFrameRate();
                


                // subtract from messsage
                deleteCharFromUserTypedMessage();
                
                            

                // shorter delay for subsequent repeats
                stepsBetweenDeleteRepeat = (int)( 10 / frameRateFactor );
                }
            }
        
        // fade out music during pause
        
        double oldLoudness = getMusicLoudness();
        
        if( oldLoudness > 0 ) {
            
            oldLoudness -= ( 1.0 / 60 ) * frameRateFactor;

            if( oldLoudness < 0 ) {
                oldLoudness = 0;
                }
            setMusicLoudness( oldLoudness );
            }
        

        // fade in pause screen
        if( pauseScreenFade < 1 ) {
            pauseScreenFade += ( 1.0 / 30 ) * frameRateFactor;
        
            if( pauseScreenFade > 1 ) {
                pauseScreenFade = 1;
                }
            }
        

        return;
        }


    // not paused

    
    
    


    // fade music in
        
    double oldLoudness = getMusicLoudness();
    
    if( oldLoudness < 1 ) {
            
        oldLoudness += ( 1.0 / 60 ) * frameRateFactor;

        if( oldLoudness > 1 ) {
            oldLoudness = 1;
            }
        setMusicLoudness( oldLoudness );
        }

    // fade pause screen out
    if( pauseScreenFade > 0 ) {
        pauseScreenFade -= ( 1.0 / 30 ) * frameRateFactor;
        
        if( pauseScreenFade < 0 ) {
            pauseScreenFade = 0;
            if( currentUserTypedMessage != NULL ) {

                // make sure it doesn't already end with a file separator
                // (never insert two in a row, even when player closes
                //  pause screen without typing anything)
                int lengthCurrent = strlen( currentUserTypedMessage );

                if( lengthCurrent < 2 ||
                    currentUserTypedMessage[ lengthCurrent - 2 ] != 28 ) {
                         
                        
                    // insert at file separator (ascii 28)
                    
                    char *oldMessage = currentUserTypedMessage;
                    
                    currentUserTypedMessage = autoSprintf( "%s %c ", 
                                                           oldMessage,
                                                           28 );
                    delete [] oldMessage;
                    }
                }
            }
        }    
    
    

    if( !firstDrawFrameCalled ) {
        // do final init step... stuff that shouldn't be done until
        // we have control of screen
        
        char *moveKeyMapping = 
            SettingsManager::getStringSetting( "upLeftDownRightKeys" );
    
        if( moveKeyMapping != NULL ) {
            char *temp = stringToLowerCase( moveKeyMapping );
            delete [] moveKeyMapping;
            moveKeyMapping = temp;
        
            if( strlen( moveKeyMapping ) == 4 &&
                strcmp( moveKeyMapping, "wasd" ) != 0 ) {
                // different assignment

                mapKey( moveKeyMapping[0], 'w' );
                mapKey( moveKeyMapping[1], 'a' );
                mapKey( moveKeyMapping[2], 's' );
                mapKey( moveKeyMapping[3], 'd' );

                // replace in tutorial text, too
                tutorialMoveKeys[0] = moveKeyMapping[0];
                tutorialMoveKeys[2] = moveKeyMapping[1];
                tutorialMoveKeys[4] = moveKeyMapping[2];
                tutorialMoveKeys[6] = moveKeyMapping[3];

                temp = stringToUpperCase( tutorialMoveKeys );
                delete [] tutorialMoveKeys;
                tutorialMoveKeys = temp;
                }
            delete [] moveKeyMapping;
            }

        initTutorial();
        
        firstDrawFrameCalled = true;
        }
    



    if( lastRiseFreezeFrameDrawn ) {
        
        updateLevelNumber();
        
        

        // populate stack here, in case we rise back out further
        // this prevents frame hiccups, because this happens
        // at the tail end of lastLevel's freeze right before
        // it becomes the current level, so the hiccup is hidden
        populateLevelRiseStack();

        // also do decompaction of next up in stack here, to hide
        // decompaction time in lastLevel's final freeze frame
        Level *nextUp = 
            *( levelRiseStack.getElement( levelRiseStack.size() - 1 ) );
        
        nextUp->decompactLevel();

        saveLevelBookmark();
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
        char symmetrical = true;

        char insideEnemy = false;
        
        char knockDown = false;
        
        char enterSelf = false;

        char insidePowerUp = false;
        
        int tokenRecursionDepth = 0;
        
        int parentEnemyDifficultyLevel = 0;

        int parentTokenLevel = 0;
        
        int parentFloorTokenLevel = currentLevel->getFloorTokenLevel();

        int parentDifficultyLevel = currentLevel->getDifficultyLevel();
        

        PowerUpSet enteredStartSet = defaultSet;

        if( playerHealth > 0 && 
            currentLevel->isEnemy( mousePos, &itemIndex ) ) {
            
            enteringPos = currentLevel->getEnemyCenter( itemIndex );
            enteringHit = true;
            enteringType = enemy;
            symmetrical = false;
            insideEnemy = true;

            parentEnemyDifficultyLevel = 
                currentLevel->getEnemyDifficultyLevel( itemIndex );
            }
        else if( playerHealth == 0 ||
                 currentLevel->isPlayer( mousePos ) ) {
            enteringPos = playerPos;
            enteringHit = true;
            enteringType = player;
            symmetrical = true;

            if( playerHealth == 0 ) {
                knockDown = true;
                }
            else {
                enterSelf = true;
                }
            }
        else if( currentLevel->isPowerUp( mousePos, &itemIndex ) ) {
            enteringPos = currentLevel->getPowerUpCenter( itemIndex );
            enteringHit = true;
            enteringType = power;
            symmetrical = false;

            PowerUp hitPowerUp = 
                currentLevel->peekPowerUp( mousePos );

            spriteID powerType = hitPowerUp.powerType;

            if( powerType == powerUpEmpty ||
                powerType == powerUpBulletSize ||
                powerType == powerUpRapidFire ) {
                
                symmetrical = true;
                }

            insidePowerUp = true;
            

            parentTokenLevel = hitPowerUp.level;
            
            // one step deeper
            tokenRecursionDepth = currentLevel->getTokenRecursionDepth() + 1;

            // could be inside a power-up sub-chain in an enemy
            insideEnemy = currentLevel->isInsideEnemy();
            }
        

        if( enteringHit ) {
            // force entering to STOP here, so player doesn't get confused
            // if holding down enter key accidentally
            entering = false;
            
            if( playerHealth > 0 ) {
                // don't count forced entering
                tutorialSomethingEntered( enteringType );
                }
            else {
                tutorialPlayerKnockedDown();
                }

            
            // this call sets the last entering point power set, too
            ColorScheme c = 
                currentLevel->getEnteringPointColors( mousePos, enteringType );

            // make a copy
            PowerUpSet enteredStartSet( 
                currentLevel->getLastEnterPointPowers() );

            
            
            levelRiseStack.push_back( currentLevel );
            // enemy or player is entry position
            LevelPositionInfo info = 
                { playerPos, lastScreenViewCenter, 
                  enteringPos, enteringType, mousePos, enteredStartSet };
            levelRisePositionInfoStack.push_back( info );

            lastLevel = currentLevel;
            lastLevel->freezeLevel( true );

            lastLevelPosition = info;
            zoomProgress = 0;
            zoomDirection = 1;
            
#ifdef USE_MALLINFO
            struct mallinfo meminfo = mallinfo();
    
            int oldAllocedBytes = meminfo.uordblks;
#endif


            RandomWalkerSet walkerSet =
                currentLevel->getEnteringPointWalkerSet( mousePos,
                                                         enteringType );

            NoteSequence musicNotes =
                currentLevel->getEnteringPointNoteSequence( mousePos,
                                                            enteringType );

            printf( "Entering sub-level that should have music part %d\n",
                    musicNotes.partIndex );
            
            int subLevelNumber =
                currentLevel->getEnteringPointSubLevel( mousePos, 
                                                        enteringType );

            if( enteringType == power ) {
                // entering a power, show majority sub-power coloring
                PowerUpSet *subPowers = 
                    currentLevel->getLastEnterPointPowers();
                
                subPowers->setDimMinority( true );
                }
            

            
            // wait to compute player powers until we know level difficulty
            // (we give more hearts when knock-down to harder level happens)
            PowerUpSet *setPlayerPowers = NULL;
            
            

            currentLevel = new Level( randSource.getRandomInt(),
                                      NULL, NULL, &c, &walkerSet, NULL,
                                      &musicNotes,
                                      setPlayerPowers,
                                      subLevelNumber,
                                      symmetrical, 
                                      enterSelf,
                                      insideEnemy, insidePowerUp,
                                      knockDown,
                                      tokenRecursionDepth,
                                      parentEnemyDifficultyLevel,
                                      parentTokenLevel,
                                      parentFloorTokenLevel,
                                      parentDifficultyLevel );
            
            // don't let player get hit during zoom-in
            currentLevel->toggleShooting( false );
            

            if( ! knockDown ) {
                // entering, copy powers
                setPlayerPowers = new PowerUpSet( 
                    lastLevel->getPlayerPowers() );
                }
            else {
                // cause player that we're entering to drop down in power

                // replace left-most non-empty

                // replace all empties with heart(1)

                PowerUpSet newSet( lastLevel->getPlayerPowers() );

                newSet.sortPowersRight();
                

                // if left-most is not empty, make it empty

                if( newSet.mPowers[0].powerType != powerUpEmpty ) {
                    newSet.mPowers[0].powerType = powerUpEmpty;
                    newSet.mPowers[0].level = 0;     
                    }


                // now replace empty slots with X hearts each, depending
                // on difficulty

                int newLevelDifficulty = currentLevel->getDifficultyLevel();
                
                int heartLevel = 2;
                

                if( newLevelDifficulty > 20 ) {
                    heartLevel += newLevelDifficulty / 20;
                    }
                
                // cap at 8 hearts per slot (don't want to make player 
                // invicible at really deep, hard levels)
                if( heartLevel > 8 ) {
                    heartLevel = 8;
                    }


                for( int s=0; s<POWER_SET_SIZE; s++ ) {

                    if( newSet.mPowers[s].powerType == powerUpEmpty ) {    
                        newSet.mPowers[s].powerType = powerUpHeart;
                        newSet.mPowers[s].level = heartLevel;
                        }
                    
                    }
                
                PowerUpSet *currentPowers = lastLevel->getPlayerPowers();
                
                // skip this animation if we're already at default set
                if( !newSet.equals( currentPowers ) ) {        
                    currentPowers->dropDownToSet( &newSet );
                    }

                setPlayerPowers = new PowerUpSet( &newSet );
                }


            currentLevel->setPlayerPowers( setPlayerPowers, true );

            delete setPlayerPowers;

            

            currentLevel->pushAllMusicIntoPlayer();
            
#ifdef USE_MALLINFO            
            meminfo = mallinfo();
            printf( "Level construction used %d kbytes (%d tot)\n",
                    (meminfo.uordblks - oldAllocedBytes ) / 1024,
                    meminfo.uordblks / 1024 );
#endif

            if( symmetrical ) {
                playerPos.x = -0.5;
                playerPos.y = 0;
                mousePos.x = -0.5;
                mousePos.y = 0;

                lastScreenViewCenter.x = -0.5;
                // a little higher than player to line up with 
                // super-level sprite pixels
                lastScreenViewCenter.y = 0.5;
                }
            else {
                // safe, since -0.5 might be out of bounds
                playerPos.x = 0;
                playerPos.y = 0;
                mousePos.x = 0;
                mousePos.y = 0;

                // a little higher than player to line up with 
                // super-level sprite pixels
                lastScreenViewCenter.x = -0.5;
                lastScreenViewCenter.y = 0.5;
                }
            
            setViewCenterPosition( 0, 0 );
            currentLevel->drawFloorEdges( false );
            }
        
        }
    

    // trigger tool tips for power-ups that are moused over
    int itemIndex;
    if( currentLevel->isPowerUp( mousePos, &itemIndex ) ) {
        
        int subLevelDifficulty;

        PowerUp hitPowerUp = 
            currentLevel->peekPowerUp( mousePos, &subLevelDifficulty );

        int difficultyModifier;
        
        // show difficulty change relative to baseline "level below" 
        // difficulty 
        if( levelNumber > 0 ) {
            difficultyModifier = 
                subLevelDifficulty - 
                ( currentLevel->getDifficultyLevel() - 1 );
            }
        else {
            difficultyModifier = 
                subLevelDifficulty - 
                ( currentLevel->getDifficultyLevel() + 1 );
            }
        
        if( shouldEnterBeBlocked() ) {
            // don't display difficulty modifier
            difficultyModifier = 0;
            }

        // keepy showing, with a delay, after quota of showings filled
        // start at 0.5 to only hold solid for 1 second after mouse leaves 
        triggerTip( hitPowerUp.powerType, true, 0.5, difficultyModifier );
        }
    



    // if current level still frozen, last freeze frame not drawn yet,
    // and next level on stack not decompacted yet.
    // Don't try to rise into next level on stack if that's the case.

    if( currentLevel->isRiseSpot( playerPos ) && 
        lastLevel == NULL &&
        ! currentLevel->isFrozen() ) {
        
        
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
        
        // already sitting on stack decompacted
        // from last frame of previous rise-up

        lastLevel->freezeLevel( true );
        
        // some stuff may have changed while level was frozen
        // update it before drawing it again
        lastLevel->frozenUpdate();
        


        if( currentLevel->isKnockDown() ) {
            // reset level we're rising into after knock-down
            lastLevel->rewindLevel();

            // ignore position info that was used to zoom-in
            // re-center on player
            lastLevelPosition.playerPos = lastLevel->getPlayerPos();
            lastLevelPosition.lastScreenViewCenter = lastLevel->getPlayerPos();
            lastLevelPosition.entryPosition = lastLevel->getPlayerPos();
            lastLevelPosition.mousePos = lastLevel->getPlayerPos();
            }
        

        zoomProgress = 1;
        zoomDirection = -1;
        currentLevel->drawFloorEdges( false );

        Level *nextHigherLevel = 
            *( levelRiseStack.getElement( levelRiseStack.size() - 1 ) );


        char powersPasssedUp = false;

        if( currentLevel->isInsidePlayer() && lastLevel->isInsidePlayer() 
            &&
            lastLevel->getStepCount() == 0 ) {

            // copy player's collected tokens upward
            // ONLY if we're rising up to a fresh level OR if we're rising
            // up from a knock-down (rewind)

            // DO NOT pass up if we're rising up from entering self or
            // enemy or token


            if( nextHigherLevel->getStepCount() == 0 || 
                lastLevel->isKnockDown() ) {
                // pass gathered powers up through a knock-down chain OR
                // a fresh-level chain

                // like a full rewind of rising back into lastLevel
                // (if we got there by rising there or by being knocked there)

                PowerUpSet passedUpSet( lastLevel->getPlayerPowers() );
                

                // always sort on pass-up to encourage more pick-ups to 
                // replace weaker powers
                passedUpSet.sortPowersRight();
                

                passedUpSet.decayPowers();
                
                // sort right again, incase gap left by left-most hearts
                passedUpSet.sortPowersRight();


                nextHigherLevel->setPlayerPowers( &passedUpSet );

                powersPasssedUp = true;
                }
            }
        
        if( !powersPasssedUp 
            &&
            ! lastLevel->isKnockDown() && 
            nextHigherLevel->getStepCount() > 0
            &&
            lastLevel->getStepCount() == 0 ) {
            
            // reached lastLevel by entering something from next-higher
            // AND got knocked down from lastLevel (it has been rewound)

            // restore the tokens that we intentionally entered
            // lastLevel with from next higher level (full rewind
            //   as if we're starting lastLevel fresh from exactly
            //  how we got there)
            
            
            if( lastLevel->isInsidePlayer() ) {
                // ignore entered set in position info, because
                // we are not setting it properly farther up the
                // level stack where we didn't actually enter anything
                // (just inside self by default)
                nextHigherLevel->setPlayerPowers(
                    lastLevel->getStartingPlayerPowers() );
                }
            else {
                // inside power-up or enemy
                // this only happens with player intention, which 
                // means that a proper enter point start state
                // has been saved in position info
                
                PowerUpSet *s = nextHigherLevel->getLastEnterPointPowers();
                
                LevelPositionInfo *nextHigherLevelPositionInfo = 
                    levelRisePositionInfoStack.getElement( 
                        levelRisePositionInfoStack.size() - 1 );

                s->copySet( 
                    &( nextHigherLevelPositionInfo->
                           enterPointPowerStartState ) );
                
                }
            }
        
        if( !powersPasssedUp ) {
            // no set passed up or decayed
            
            // make sure that next-higher self does not have more hearts
            // than we have (to prevent hearts from lasting forever through
            // rising
    
            PowerUpSet *nextHigherPowers = nextHigherLevel->getPlayerPowers();
        
            PowerUpSet *lastLevelPowers = lastLevel->getPlayerPowers();
        

            int nextHigherHearts = 
                nextHigherPowers->getLevelSum( powerUpHeart );
            
            int lastLevelHearts = lastLevelPowers->getLevelSum( powerUpHeart );
        

            if( nextHigherHearts > lastLevelHearts ) {
            
                // do it instantly, because it's not visible
                nextHigherPowers->knockOffHearts( 
                    nextHigherHearts - lastLevelHearts, true );

                // sort them, so that player isn't stuck with a low
                // token on the right
                nextHigherPowers->sortPowersRight();
                }    
            }
        
        }
    
    
        

    
    






    int powerUpIndex;
    if( currentLevel->isPowerUp( playerPos, &powerUpIndex, true ) ) {
        
        doublePair powerPos = currentLevel->getPowerUpCenter( powerUpIndex );

        PowerUp p = currentLevel->getPowerUp( playerPos );
        

        
        Level *nextAbove;
        if( lastLevel != NULL ) {
            // zooming
            nextAbove = lastLevel;
            }
        else {
            nextAbove = getNextAbove();
            }
        
        PowerUpSet *s = nextAbove->getLastEnterPointPowers();

        s->pushPower( p, powerPos );

        // potentially need to keep player's health updated
        nextAbove->frozenUpdate();
        

        if( !shouldSetTipsBeShown() ) {
            // if we're done showing set combo tips, revert to showing
            // pick-up tips until we meet our quota for each type
            
            // don't trigger at all after quota filled
            // show for 2 seconds before a 1-second fade
            triggerTip( p.powerType, false, 0.25 );
            }
        
        }
    

    
    
    



    velocityX += accelX;
    if( velocityX > moveSpeed ) {
        velocityX = moveSpeed;
        }
    else if( velocityX < -moveSpeed ) {
        velocityX = -moveSpeed;
        }
    
    if( accelX == 0 ) {
        // slow down
        if( velocityX > 0 ) {
            velocityX -= moveAccel;
            }
        else if( velocityX < 0 ) {
            velocityX += moveAccel;
            }
        }
    


    velocityY += accelY;
    if( velocityY > moveSpeed ) {
        velocityY = moveSpeed;
        }
    else if( velocityY < -moveSpeed ) {
        velocityY = -moveSpeed;
        }


    if( accelY == 0 ) {
        // slow down
        if( velocityY > 0 ) {
            velocityY -= moveAccel;
            }
        else if( velocityY < 0 ) {
            velocityY += moveAccel;
            }
        }

    

    doublePair velocity = { velocityX, velocityY };
    
    if( velocityX != 0 && velocityY != 0 ) {
        
        // diagonal... slow it down so it's not faster than H or V move
        
        // this creates aliasing glitches in player position
        //double componentVelocity = sqrt( (moveSpeed * moveSpeed)/2 );
        
        // use closest fraction of 32 screen pixels:  3/32
        double componentVelocity = 0.09375 * frameRateFactor;
        

        // cap either component if it rises above max diagonal component
        // velocity

        if( fabs( velocityX ) > componentVelocity ) {
            
            // divide by fabs to extract sign
            velocity.x = componentVelocity * velocityX / fabs( velocityX );
            }
        if( fabs( velocityY ) > componentVelocity ) {
            
            // divide by fabs to extract sign
            velocity.y = componentVelocity * velocityY / fabs( velocityY );
            }
        }
    


    doublePair newPlayerPos = currentLevel->stopMoveWithWall( playerPos,
                                                              velocity );
    
    
    // printf( "Player pos = %f, %f\n", newPlayerPos.x, newPlayerPos.y );
    
    doublePair effectiveVelocity = sub( newPlayerPos, playerPos );
    
    currentLevel->setPlayerVelocity( effectiveVelocity );
    

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

    doublePair screenMoveDelta = { 0, 0 };
    

    // stop move screen whenever position to center is inside the center 
    // rectangle (separate threshold for x and y)
    if( screenCenterDistanceFromPlayerX > 
        minDistanceToMoveScreenX ||
        screenCenterDistanceFromPlayerY > 
        minDistanceToMoveScreenY ) {

        screenMoveDelta = sub( posToCenterOnScreen, 
                               tweakedScreenViewCenter );
        
        // set correction speed based on how far off we are from VERY CENTER
        // since we stop moving when player inside center box, this eliminates
        // jerky micro-movements.
        double correctionSpeedX = 
            0.0025 * 
            pow(
                (screenCenterDistanceFromPlayerX - 0),
                2 );
        double correctionSpeedY = 
            0.0025 *
            pow(
                (screenCenterDistanceFromPlayerY - 0) 
                / tweakedViewHeightFraction,
                2 );
        
        screenMoveDelta.x *= correctionSpeedX * frameRateFactor;
        screenMoveDelta.y *= correctionSpeedY * frameRateFactor;

        
        // not actually seeing any round-off errors.
        // hold off on doing this for now
        
        // seeing round-off errors when drawing full-level bitmap overlay

        // round to closest 1/32 (16 pixels per world square, double size)
        screenMoveDelta.y *= 32;
        screenMoveDelta.y = round( screenMoveDelta.y ) / 32;
        screenMoveDelta.x *= 32;
        screenMoveDelta.x = round( screenMoveDelta.x ) / 32;
        

        lastScreenViewCenter = add( lastScreenViewCenter, screenMoveDelta );
        
        

        setViewCenterPosition( lastScreenViewCenter.x, 
                               lastScreenViewCenter.y );

        if( ! shooting && ! entering ) {
            // move mouse with screen
            mousePos.x += screenMoveDelta.x;
            mousePos.y += screenMoveDelta.y;
            }
        }

    
    if( screenMoveDelta.x != 0 || screenMoveDelta.y != 0 ) {
        confineMouseOnScreen();
        }
    

    if( shooting ) {
        if( stepsTilNextBullet == 0 ) {
            // fire bullet
            PowerUpSet *playerPowers = currentLevel->getPlayerPowers();
            
            // set speed
            double bulletSpeed = getBulletSpeed( playerPowers );

            
            currentLevel->addBullet( playerPos, mousePos, 
                                     playerPowers,
                                     mousePos,
                                     playerCorneringDir,
                                     bulletSpeed, true );
            
            playerCorneringDir = ! playerCorneringDir;

            //stepsTilNextBullet = stepsBetweenBullets;
            stepsTilNextBullet = getStepsBetweenBullets( playerPowers );
            }
        }
    
    // always decrement, even when mouse not held down
    if( stepsTilNextBullet > 0 ) {
        stepsTilNextBullet --;
        }




    // is player auto-movement on?
    if( enableRobotPlayer ) {    
        playerAutoMove();
        }
    

    



    // now draw stuff AFTER all updates
    drawFrameNoUpdate( true );



    // draw tail end of pause screen, if it is still visible
    if( pauseScreenFade > 0 ) {
        drawPauseScreen();
        }
    }



void drawFrameNoUpdate( char inUpdate ) {

    // toggle animations that are built-in to power-up set drawing functions
    PowerUpSet::sPauseAllSets = !inUpdate;
    


    char stencilDrawn = false;
    
    double zoomFactor = 1;
    double viewSize = viewWidth;
    
    // even multiple of 16
    // each pixel in portal sprite maps to 5x5 block of tiles in sub-level
    double zoomScale = 79;
    double zoomScaleTweaked = zoomScale + 1;
    
    if( lastLevel != NULL ) {
        //zoomFactor = ( 1 + 50 * pow( zoomProgress, 2 ) );
        zoomFactor = 
            (sin( (zoomProgress * 2 - 1) * M_PI/2 ) * 0.5 + 0.5 ) 
            * zoomScale + 1;
        
        

        viewSize = viewWidth / zoomFactor;

        setViewSize( viewSize );
        lastLevelCurrentViewSize = viewSize;
        
        // move toward entry point as we zoom in
        double moveFraction = 1 - 1/zoomFactor + 
            ( zoomProgress * 1/ zoomScaleTweaked );
        
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
        center.x *= zoomScaleTweaked;
        center.y *= zoomScaleTweaked;
        
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


        viewSize = zoomScaleTweaked * viewWidth / zoomFactor;        
        setViewSize( viewSize );
        }
    else {
        setDrawColor( 0, 0, 0, 1 );
    
        drawSquare( lastScreenViewCenter, viewWidth );
        }

    
    if( lastRiseFreezeFrameDrawn ) {
        lastRiseFreezeFrameDrawn = false;
        currentLevel->freezeLevel( false );
        // with new knock-down rise-out structure, player immortality
        // no longer necessary (never rise back into the middle of a fight)
        // For rises out of intentionally entering things, we can assume
        //  that the player generally does not do this under heavy fire.
        // Furthermore, we MUST eliminate temporary immortality to thwart
        //  the "keep inching foward" skill-free way of passing a level.
        // If we kept immortatilty after entering things, a player could
        //  exploit this for skill-free grinding through a level.
        //currentLevel->startPlayerImmortal();
        }
    else if( secondToLastRiseFreezeFrameDrawn ) {
        secondToLastRiseFreezeFrameDrawn = false;
        lastRiseFreezeFrameDrawn = true;
        }
    else {
        // okay to pass player movement to level
        if( inUpdate ) {    
            currentLevel->setMousePos( mousePos );
            currentLevel->setPlayerPos( playerPos );
            currentLevel->setEnteringMouse( entering );
            }
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
            
            lastLevel->drawWindowShade( centerFade, frameFade,
                                        lastLevelCurrentViewCenter,
                                        lastLevelCurrentViewSize );
            }

        
        // force the end of any tips that were started DURING zoom
        forceTipEnd();
        forceSetTipEnd();

        // step zoom and check for zoom end
        if( inUpdate ) {
            zoomProgress += zoomSpeed * zoomDirection * frameRateFactor;
            }
    
        if( zoomProgress >= 1 && zoomDirection == 1) {
#ifdef USE_MALLINFO
            struct mallinfo meminfo = mallinfo();
    
            int oldAllocedBytes = meminfo.uordblks;
#endif
            if( levelRiseStack.size() >= 2 ) {
                // compact next above last level
                Level *nextUp = *( levelRiseStack.getElement(
                                       levelRiseStack.size() - 2 ) );
                nextUp->compactLevel();
                }
            
#ifdef USE_MALLINFO
            meminfo = mallinfo();
            printf( "Level compaction used %d kbytes (%d tot)\n",
                    (meminfo.uordblks - oldAllocedBytes ) / 1024,
                    meminfo.uordblks / 1024 );
#endif
            lastLevel = NULL;

            // saveScreenShot( "zoomScreen" );

            // go with current level
            setViewSize( viewWidth );
            setViewCenterPosition( lastScreenViewCenter.x, 
                                   lastScreenViewCenter.y );
            
            currentLevel->drawFloorEdges( true );
            
            currentLevel->toggleShooting( true );
            

            updateLevelNumber();
            
            saveLevelBookmark();
            
            zoomProgress = 0;
            }
        else if( zoomProgress <= 0 && zoomDirection == -1 ) {
            
            // done with current level
            delete currentLevel;

            // switch to last level (zooming out)
            currentLevel = lastLevel;
            currentLevel->pushAllMusicIntoPlayer();
            
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
            
            zoomProgress = 0;

            // report to tutorial
            tutorialRiseHappened( currentLevel->getLevelNumber() );
            }    
        }



    // draw dashboard
    setViewSize( viewWidth );
    setViewCenterPosition( lastScreenViewCenter.x, 
                           lastScreenViewCenter.y );
    


    // body of panel
    setDrawColor( 0, 0, 0, 1 );


    drawRect( lastScreenViewCenter.x - viewWidth /2,
              // off top, just in case
              lastScreenViewCenter.y + viewWidth /2,
              lastScreenViewCenter.x + viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 - dashHeight );
    

    // cover up bottom too, in case too much is shown
    drawRect( lastScreenViewCenter.x - viewWidth /2,
              // off top, just in case
              lastScreenViewCenter.y - viewHeightFraction * viewWidth /2,
              lastScreenViewCenter.x + viewWidth /2,
              lastScreenViewCenter.y - viewWidth / 2 );



    // compute level number position here, because we use it as a base
    // for other positions on dashboard

    doublePair levelNumberPos = { lastScreenViewCenter.x +
                                  viewWidth /2,
                                  lastScreenViewCenter.y +
                                  viewHeightFraction * viewWidth /2 - 0.625 };



    Level *nextAbove = getNextAbove();
    
        
        

    BorderSprite *weAreInsideSprite = nextAbove->getLastEnterPointSprite();
    PowerUpSet *p = nextAbove->getLastEnterPointPowers();
    
    
    doublePair spritePos = levelNumberPos;
    spritePos.x = lastScreenViewCenter.x - viewWidth/2 + 1.5625;
    //spritePos.x -= zoomProgress * viewWidth /2;
    
    spritePos.y += 0.125;
    
    float fade = 1;
    if( zoomProgress != 0 ) {
        fade = 1 - zoomProgress;

        if( zoomDirection != 1 ) {
            if( levelRiseStack.size() >= 1 ) {
                Level *nextHigher = *( levelRiseStack.getElement(
                                           levelRiseStack.size() - 1 ) );
                
                weAreInsideSprite = nextHigher->getLastEnterPointSprite();
                p = nextHigher->getLastEnterPointPowers();
                }
            
            }
        
        }
    

    weAreInsideSprite->draw( spritePos, fade );

    doublePair markerPos = spritePos;
    markerPos.x -= 1.0625;
    
    
    setDrawColor( 1, 1, 1, 1 );
    
    
    
    
    doublePair setPos = spritePos;
    setPos.x += 2.1875;

    p->drawSet( setPos, fade );
    

    
    Level *levelToGetCurrentFrom;
    if( lastLevel != NULL ) {
        levelToGetCurrentFrom = lastLevel;
        }
    else {
        levelToGetCurrentFrom = currentLevel;
        }


    if( levelToGetCurrentFrom != currentLevel ) {
        // draw for current level too, underneath, keep centered

        PowerUpSet *playerPowers = currentLevel->getPlayerPowers();
        setPos = spritePos;
        setPos.x = lastScreenViewCenter.x;
        
        playerPowers->drawSet( setPos, zoomProgress );

        PlayerSprite *playerSprite = currentLevel->getPlayerSprite(); 

        spritePos = setPos;
        spritePos.x -= 2.1875;
    
        playerSprite->draw( spritePos, zoomProgress );
        }



    fade = 1;

    PowerUpSet *playerPowers = levelToGetCurrentFrom->getPlayerPowers();
    setPos = spritePos;
    setPos.x = lastScreenViewCenter.x;
    setPos.x -= sin( zoomProgress * M_PI * 0.5 ) * (viewWidth /2 - 3.75);

    PlayerSprite *playerSprite = levelToGetCurrentFrom->getPlayerSprite(); 
    spritePos = setPos;
    spritePos.x -= 2.1875;

    BorderSprite *enteringPointSprite = playerSprite;

    
    if( zoomProgress != 0 && lastLevel != NULL ) {
        BorderSprite *lastLevelSprite = lastLevel->getLastEnterPointSprite();
        
        
        if( playerSprite != lastLevelSprite ) {
            // moving set doesn't match one we're going inside
            // fade out moving set
            fade = 1 - zoomProgress;

            PowerUpSet *lastLevelPowers = 
                lastLevel->getLastEnterPointPowers();
            
            lastLevelPowers->drawSet( setPos, 1 - fade );
            lastLevelSprite->draw( spritePos, 1 - fade );

            enteringPointSprite = lastLevelSprite;
            }
        }
    
    
    playerPowers->drawSet( setPos, fade );
    playerSprite->draw( spritePos, fade );
    
    
    Color *riseIconColor = Color::linearSum( 
        &( enteringPointSprite->getColors().special ),
        &( weAreInsideSprite->getColors().special ),
        zoomProgress );
    
    setDrawColor( riseIconColor->r, riseIconColor->g, riseIconColor->b, 1 );

    delete riseIconColor;
    
    drawSprite( riseMarker, markerPos );

    setDrawColor( 1, 1, 1, 1 );


    double healthBarMaxX = - DBL_MAX;

    if( levelToGetCurrentFrom != currentLevel ) {
        // draw faded in underneath, always centered
        
        doublePair currentBarPos = { lastScreenViewCenter.x, setPos.y };
        currentBarPos.x += 1.75;

        int playerHealth, playerMax;

        currentLevel->getPlayerHealth( &playerHealth, &playerMax );
        float playerHealthFraction = playerHealth / (float)playerMax;

        doublePair thisBarPos = 
            add( currentBarPos, 
                 currentLevel->getPlayerHealthBarJitter() );


        healthBarMaxX = 
            drawHealthBar( thisBarPos, playerHealthFraction, playerMax,
                           zoomProgress );
        }


    

    // top health bar, moves with set during zoom
    doublePair barPos = setPos;
    barPos.x += 1.75;

    int playerHealth, playerMax;
    
    levelToGetCurrentFrom->getPlayerHealth( &playerHealth, &playerMax );
    float playerHealthFraction = playerHealth / (float)playerMax;

    doublePair thisBarPos = 
        add( barPos, 
             levelToGetCurrentFrom->getPlayerHealthBarJitter() );
    

    double thisMaxX =
        drawHealthBar( thisBarPos, playerHealthFraction, playerMax, 
                       1 - zoomProgress );
    
    if( thisMaxX > healthBarMaxX ) {
        healthBarMaxX = thisMaxX;
        }
    
    



    // level number display on dash
    // shrink it to avoid hitting health bar if necessary
    setDrawColor( levelNumberColor.r, levelNumberColor.g, levelNumberColor.b, 
                  levelNumberColor.a );
    
    
    
    char *levelString = autoSprintf( "%d", levelNumber );
    
    double stringWidth = levelNumberFont->measureString( levelString );
    
    if( levelNumberPos.x - stringWidth < healthBarMaxX ) {
        // hits health bar

        // draw it smaller, and a bit up more
        doublePair smallNumberPos = levelNumberPos;
        
        smallNumberPos.y += 0.0625;
        
        levelNumberReducedFont->drawString( levelString, 
                                            smallNumberPos, alignRight );
        }
    else {
        levelNumberFont->drawString( levelString, levelNumberPos, alignRight );
        }
    
    delete [] levelString;







    // darken bottom of entire panel to push it back a bit
    float vertexColors[] =  { 0, 0, 0, .75,
                              0, 0, 0, .75,
                              0, 0, 0, 0,
                              0, 0, 0, 0 };    

    double vertices[] = 
        { lastScreenViewCenter.x - viewWidth /2,
          lastScreenViewCenter.y + 
          viewHeightFraction * viewWidth /2 - dashHeight,
          lastScreenViewCenter.x + viewWidth /2,
          lastScreenViewCenter.y + 
          viewHeightFraction * viewWidth /2 - dashHeight,
          lastScreenViewCenter.x + viewWidth /2,
          lastScreenViewCenter.y + 
          viewHeightFraction * viewWidth /2 - dashHeight + 3*dashHeight/4,
          lastScreenViewCenter.x - viewWidth /2,
          lastScreenViewCenter.y + 
          viewHeightFraction * viewWidth /2 - dashHeight + 3*dashHeight/4 };
        
    drawQuads( 1, vertices, vertexColors );



    // draw this flag on top, above shadows
    if( extraDifficultyNumber > 0 ) {

        setDrawColor( 0, 0.5, 0, 1 );

        doublePair difficultyPosition = levelNumberPos;

        const char *difficultyWord = translate( "difficultyTag" );

        double wordLength = tinyFont->measureString( difficultyWord );

        difficultyPosition.x -= 0.1875 + wordLength;

        difficultyPosition.y -= 0.25;
        
        tinyFont->drawString( difficultyWord, difficultyPosition, alignLeft );


        difficultyPosition.x -= 0.375;

        drawNumber( extraDifficultyNumber, difficultyPosition, 
                    alignRight, true );
        }




    // border around panel, draw on top so that powers move under it
    setDrawColor( 0.3, 0.3, 0.3, 1 );

    drawRect( lastScreenViewCenter.x - viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 - dashHeight,
              lastScreenViewCenter.x + viewWidth /2,
              lastScreenViewCenter.y + 
                viewHeightFraction * viewWidth /2 - dashHeight - 0.0625 );


    drawTutorial( lastScreenViewCenter );

    drawTipDisplay( lastScreenViewCenter );
    drawSetTipDisplay( lastScreenViewCenter );
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
        accelY = moveAccel;
        tutorialKeyPressed( 0 );
        }
    else if( movementKeysDown[1] && ! lastMovementKeysDown[1] ) {
        accelY = -moveAccel;
        tutorialKeyPressed( 1 );
        }
    // releases?
    else if( movementKeysDown[0] && ! movementKeysDown[1] ) {
        accelY = moveAccel;
        }
    else if( movementKeysDown[1] && ! movementKeysDown[0] ) {
        accelY = -moveAccel;
        }
    else if( ! movementKeysDown[0] && ! movementKeysDown[1] ) {
        accelY = 0;
        }
    

    // new presses
    if( movementKeysDown[2] && ! lastMovementKeysDown[2] ) {
        accelX = moveAccel;
        tutorialKeyPressed( 2 );
        }
    else if( movementKeysDown[3] && ! lastMovementKeysDown[3] ) {
        accelX = -moveAccel;
        tutorialKeyPressed( 3 );
        }
    // releases?
    else if( movementKeysDown[2] && ! movementKeysDown[3] ) {
        accelX = moveAccel;
        }
    else if( movementKeysDown[3] && ! movementKeysDown[2] ) {
        accelX = -moveAccel;
        }
    else if( ! movementKeysDown[2] && ! movementKeysDown[3] ) {
        accelX = 0;
        }


    memcpy( lastMovementKeysDown, movementKeysDown, 4 );
    }



void playerAutoMove() {
    
    doublePair nextPos = currentLevel->getNextPlayerPosTowardRise( moveSpeed );

    if( nextPos.y < playerPos.y - 0.3 ) {
        movementKeysDown[1] = true;
        movementKeysDown[0] = false;
        }
    else if( nextPos.y > playerPos.y + 0.3 ) {
        movementKeysDown[1] = false;
        movementKeysDown[0] = true;
        }
    else {
        movementKeysDown[1] = false;
        movementKeysDown[0] = false;
        }

    if( nextPos.x < playerPos.x - 0.3 ) {
        movementKeysDown[3] = true;
        movementKeysDown[2] = false;
        }
    else if( nextPos.x > playerPos.x + 0.3 ) {
        movementKeysDown[3] = false;
        movementKeysDown[2] = true;
        }
    else {
        movementKeysDown[3] = false;
        movementKeysDown[2] = false;
        }
    movementKeyChange();
    }





extern int testBulletValue;


void keyDown( unsigned char inASCII ) {
    
    if( isPaused() ) {
        // block general keyboard control during pause

        switch( inASCII ) {
            case 13:  // enter
                // unpause
                pauseGame();
                break;
            }
        
        
        
        if( inASCII == 127 || inASCII == 8 ) {
            // subtract from it

            deleteCharFromUserTypedMessage();

            holdDeleteKeySteps = 0;
            // start with long delay until first repeat
            stepsBetweenDeleteRepeat = (int)( 30 / frameRateFactor );
            }
        else if( inASCII >= 32 ) {
            // add to it
            if( currentUserTypedMessage != NULL ) {
                
                char *oldMessage = currentUserTypedMessage;

                currentUserTypedMessage = autoSprintf( "%s%c", 
                                                       oldMessage, inASCII );
                delete [] oldMessage;
                }
            else {
                currentUserTypedMessage = autoSprintf( "%c", inASCII );
                }
            }
        
        return;
        }
    
    
    switch( inASCII ) {
        case 'w':
        case 'W':
        case 'i':
        case 'I':
            movementKeysDown[0] = true;
            movementKeyChange();
            break;
        case 's':
        case 'S':
        case 'k':
        case 'K':
            movementKeysDown[1] = true;
            movementKeyChange();
            break;
        case 'd':
        case 'D':
        case 'l':
        case 'L':
            movementKeysDown[2] = true;
            movementKeyChange();
            break;
        case 'a':
        case 'A':
        case 'j':
        case 'J':
            movementKeysDown[3] = true;
            movementKeyChange();
            break;
        case ' ':
            shooting = true;
            break;
        case 13:  // enter
            if( ! shouldEnterBeBlocked() ) {
                entering = true;
                }
            break;
        case 'm':
        case 'M': {
#ifdef USE_MALLINFO
            struct mallinfo meminfo = mallinfo();
            printf( "Mem alloc: %d\n",
                    meminfo.uordblks / 1024 );
#endif
            }
            break;

        case 't':
        case 'T':
        case '?':
            resetTutorial();
            break;   
        case 'p':
        case 'P':
            pauseGame();
            break;
        case '=':
            saveScreenShot( "screen" );
            break;
        case '2':
            testBulletValue ++;
            break;
        case '1':
            testBulletValue --;
            if( testBulletValue < 0 ) {
                testBulletValue = 0;
                }
            break;
        }
    }


void keyUp( unsigned char inASCII ) {
    if( isPaused() ) {
        // block general keyboard control during pause
        
        if( inASCII == 127 || inASCII == 8 ) {
            // delete no longer held
            holdDeleteKeySteps = -1;
            }
    
        return;
        }
    



    switch( inASCII ) {
        case 'w':
        case 'W':
        case 'i':
        case 'I':
            movementKeysDown[0] = false;
            movementKeyChange();
            break;
        case 's':
        case 'S':
        case 'k':
        case 'K':
            movementKeysDown[1] = false;
            movementKeyChange();
            break;
        case 'd':
        case 'D':
        case 'l':
        case 'L':
            movementKeysDown[2] = false;
            movementKeyChange();
            break;
        case 'a':
        case 'A':
        case 'j':
        case 'J':
            movementKeysDown[3] = false;
            movementKeyChange();
            break;
        case ' ':
            shooting = false;
            break;
        case 13:  // enter
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
        case MG_KEY_LSHIFT:
        case MG_KEY_RSHIFT:
            if( ! shouldEnterBeBlocked() ) {
                entering = true;
                }
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
        case MG_KEY_LSHIFT:
        case MG_KEY_RSHIFT:
            entering = false;
            break;
        }

	} 




char getUsesSound() {
    return true;
    }



void triggerCurrentPlayerSetTip() {
    
    triggerSetTip( currentLevel->getPlayerPowers(), true, true, true );
    }



void drawString( const char *inString ) {
    
    setDrawColor( 1, 1, 1, 0.75 );

    doublePair messagePos = lastScreenViewCenter;

    messagePos.x -= viewWidth / 2;
        
    messagePos.x +=  0.25;
    

    
    messagePos.y += (viewWidth * viewHeightFraction) /  2;
    
    messagePos.y -= dashHeight;

    // avoid tutorial bracket
    messagePos.y -= 0.375;

    // avoid 4 set tip messages
    messagePos.y -= 2.5;
    
    messagePos.y -= 0.4375;
    messagePos.y -= 0.5;
    

    int numLines;
    
    char **lines = split( inString, "\n", &numLines );
    
    for( int i=0; i<numLines; i++ ) {
        

        mainFont2->drawString( lines[i], messagePos, alignLeft );
        messagePos.y -= 0.75;
        
        delete [] lines[i];
        }
    delete [] lines;
    }




// receive callback from level when hearts knocked
void playerHeartsKnockedCallback( int inNumKnockedOff ) {
    
    // don't perpetuate knock-off during zoom-out

    if( zoomDirection == -1 && zoomProgress > 0 ) {
        return;
        }

    Level *nextAbove = getNextAbove();


    if( currentLevel->isInsidePlayer() ) {
        nextAbove->getPlayerPowers()->knockOffHearts( inNumKnockedOff, false );
        }
    else {
        // do it instantly, since it's not visible
        nextAbove->getPlayerPowers()->knockOffHearts( inNumKnockedOff, true );
        }

    // keep player health updated
    nextAbove->frozenUpdate();
    }




/*
// gets the next buffer-full of sound samples from the game engine
// inBuffer should be filled with stereo Sint16 samples, little endian,
//    left-right left-right ....
// NOTE:  may not be called by the same thread that calls drawFrame,
//        depending on platform implementation
int totalFrameNumber = 0;

#define waveTableSize 50
float waveTable[ waveTableSize ];
char waveTableReady = false;

#include "minorGems/system/Thread.h"

void getSoundSamples( Uint8 *inBuffer, int inLengthToFillInBytes ) {
    //printf( "Asking for %d bytes\n", inLengthToFillInBytes );
    
    if( !waveTableReady ) {
        int sampleRate = getSampleRate();
        float sinFactor = 2 * M_PI * 441 / sampleRate;
        for( int i=0; i<waveTableSize; i++ ) {
            waveTable[i] = sin( i * sinFactor );
            }
        waveTableReady = true;
        }
    
    
    //printf( "Audio callback\n" );
    
    // 2 16-bit samples per frame
    int numFrames = inLengthToFillInBytes / 4;
    
    

    float *leftMix = new float[ numFrames ];
    float *rightMix = new float[ numFrames ];
    
    int f;

    
    for( f=0; f!=numFrames; f++ ) {
        leftMix[f] = waveTable[ totalFrameNumber % waveTableSize ];
        rightMix[f] = leftMix[f];
        totalFrameNumber++;
        }

    #define Sint16Max 32767

    // now copy samples into Uint8 buffer (converting them to Sint16s)
    int streamPosition = 0;
    for( f=0; f != numFrames; f++ ) {
        Sint16 intSampleL = (Sint16)( leftMix[f] * Sint16Max );
        Sint16 intSampleR = (Sint16)( rightMix[f] * Sint16Max );
        //printf( "Outputting samples %d, %d\n", intSampleL, intSampleR );

        inBuffer[ streamPosition ] = (Uint8)( intSampleL & 0xFF );
        inBuffer[ streamPosition + 1 ] = (Uint8)( ( intSampleL >> 8 ) & 0xFF );
        
        inBuffer[ streamPosition + 2 ] = (Uint8)( intSampleR & 0xFF );
        inBuffer[ streamPosition + 3 ] = (Uint8)( ( intSampleR >> 8 ) & 0xFF );
        
        streamPosition += 4;
        }
    
    delete [] leftMix;
    delete [] rightMix;
    }
*/





    



