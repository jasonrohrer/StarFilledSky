#include "PowerUpSet.h"


// get total level sum for a given power type
int getTotalLevel( PowerUpSet *inSet, 
                   spriteID inPowerType );



int getMaxHealth( PowerUpSet *inSet );


float getBulletSize( PowerUpSet *inSet );


int getStepsBetweenBullets( PowerUpSet *inSet );


float getBulletSpeed( PowerUpSet *inSet );


//float getAccuracy( PowerUpSet *inSet );


// angle between pack and outlying member that hasn't joined pack yet
extern float spreadD1;
// angle between members of condensed spread pack
extern float spreadD2;

//  A's are in the pack, B outside:
//
//  B     A A A A     B
  

float getSpread( PowerUpSet *inSet );



// weight of heat-seek influence, 0..1
float getHeatSeek( PowerUpSet *inSet );



float getBulletDistance( PowerUpSet *inSet );


int getBounce( PowerUpSet *inSet );

int getCornering( PowerUpSet *inSet );

int getStickySteps( PowerUpSet *inSet );


float getExplode( PowerUpSet *inSet );
