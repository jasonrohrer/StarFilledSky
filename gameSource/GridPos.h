#ifndef GRID_POS_INCLUDED
#define GRID_POS_INCLUDED

#include "minorGems/game/doublePair.h"


typedef struct GridPos {
        int x;
        int y;
    } GridPos;


GridPos add( GridPos inA, GridPos inB );

GridPos add( GridPos inA, doublePair inB );

double distance( GridPos inA, GridPos inB );


#endif
