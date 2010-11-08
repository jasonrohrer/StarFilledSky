#include "GridPos.h"
#include <math.h>


GridPos add( GridPos inA, GridPos inB ) {
    inA.x += inB.x;
    inA.y += inB.y;
    
    return inA;
    }


GridPos add( GridPos inA, doublePair inB ) {
    inA.x += (int)rint( inB.x );
    inA.y += (int)rint( inB.y );
    
    return inA;
    }


double distance( GridPos inA, GridPos inB ) {
    double delX = inA.x - inB.x;
    double delY = inA.y - inB.y;
    
    return sqrt( delX * delX + delY * delY );
    }

         
