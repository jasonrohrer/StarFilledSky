#ifndef COLOR_SCHEME_INCLUDED
#define COLOR_SCHEME_INCLUDED


#include "minorGems/graphics/Color.h"


typedef struct colorSet {

        Color elements[3];

    } colorSet;



class ColorScheme {
    
    public:    
        ColorScheme();
        

        colorSet primary;
        colorSet secondary;
        
    };


#endif
