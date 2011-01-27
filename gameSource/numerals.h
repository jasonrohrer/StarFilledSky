#ifndef NUMERALS_INCLUDED
#define NUMERALS_INCLUDED


#include "Font.h"


void initNumerals( const char *inTGAFileName );

void freeNumerals();



void drawNumber( unsigned int inNumber, doublePair inPosition, 
                 TextAlignment inAlign = alignRight,
                 char inPrependPlus = false );


#endif
