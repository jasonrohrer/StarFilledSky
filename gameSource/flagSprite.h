#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"

SpriteHandle generateFlagSprite( const char *inFlagString );



// utility stuff

// generates a 0..15 index from a hex character
// assumes upper case
int hexTo16( char inHexChar );

// generates 0..F chars from 0..16 ints
char sixteenToHex( int inNumber );


extern unsigned char flagColorMap[16][3];
