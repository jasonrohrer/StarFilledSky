#include "fixedSpriteBank.h"
#include "minorGems/game/gameGraphics.h"


static SpriteHandle spriteBank[ 100 ];


int firstPowerUpID = powerUpEmpty;
int lastPowerUpID = powerUpExplode;

int firstBehaviorID = enemyBehaviorFollow;
int lastBehaviorID = enemyBehaviorFast;



void initSpriteBank() {
    spriteBank[ riseMarker ] = loadSprite( "riseMarker.tga", false );
    spriteBank[ riseIcon ] = loadSprite( "riseIcon.tga" );
    spriteBank[ crosshair ] = loadSprite( "crosshair.tga" );
    spriteBank[ enterCrosshair ] = loadSprite( "enterCrosshair.tga" );
    spriteBank[ powerUpSlot ] = loadSprite( "powerUpSlot.tga" );
    spriteBank[ powerUpBorder ] = loadSprite( "powerUpBorder.tga" );
    spriteBank[ powerUpEmpty ] = loadSprite( "powerUpEmpty.tga" );
    spriteBank[ powerUpHeart ] = loadSprite( "powerUpHeart.tga" );
    spriteBank[ powerUpBulletSize ] = loadSprite( "powerUpBulletSize.tga" ); 
    spriteBank[ powerUpRapidFire ] = loadSprite( "powerUpRapidFire.tga" );
    spriteBank[ powerUpBulletSpeed ] = loadSprite( "powerUpBulletSpeed.tga" );
    spriteBank[ powerUpAccuracy ] = loadSprite( "powerUpAccuracy.tga" );
    spriteBank[ powerUpSpread ] = loadSprite( "powerUpSpread.tga" );
    spriteBank[ powerUpHeatSeek ] = loadSprite( "powerUpHeatSeek.tga" );
    spriteBank[ powerUpBulletDistance ] = 
        loadSprite( "powerUpBulletDistance.tga" );
    spriteBank[ powerUpBounce ] = loadSprite( "powerUpBounce.tga" );
    spriteBank[ powerUpExplode ] = loadSprite( "powerUpExplode.tga" );
    spriteBank[ enemyBehaviorFollow ] = 
        loadSprite( "enemyBehaviorFollow.tga" );
    spriteBank[ enemyBehaviorDodge ] = 
        loadSprite( "enemyBehaviorDodge.tga" );
    spriteBank[ enemyBehaviorFast ] = 
        loadSprite( "enemyBehaviorFast.tga" );
   }



void freeSpriteBank() {
    
    for( int i=0; i<endSpriteID; i++ ) {
        freeSprite( spriteBank[ i ] );
        }
    }



void drawSprite( spriteID inID, doublePair inCenter ) {    
    drawSprite( spriteBank[ inID ], inCenter, 1.0/16 );
    
    }

