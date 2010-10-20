#include "PowerUpSet.h"

int getMaxHealth( PowerUpSet *inSet );


float getBulletSize( PowerUpSet *inSet );


int getStepsBetweenBullets( PowerUpSet *inSet );


float getBulletSpeed( PowerUpSet *inSet );


float getAccuracy( PowerUpSet *inSet );


// distance between pack and outlying member that hasn't joined pack yet
extern float spreadD1;
// distance between members of condensed spread pack
extern float spreadD2;

//  A's are in the pack, B outside:
//
//  B     A A A A     B  

float getSpread( PowerUpSet *inSet );


