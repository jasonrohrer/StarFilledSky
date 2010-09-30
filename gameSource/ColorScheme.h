#ifndef COLOR_SCHEME_INCLUDED
#define COLOR_SCHEME_INCLUDED


#include "minorGems/graphics/Color.h"


typedef struct colorSet {

        // 4th is darker edge color
        Color elements[4];

    } colorSet;



class ColorScheme {
    
    public:    
        ColorScheme();
        

        colorSet primary;
        colorSet secondary;
        
    };


#endif
