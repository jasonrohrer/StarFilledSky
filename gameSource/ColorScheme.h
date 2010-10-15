#ifndef COLOR_SCHEME_INCLUDED
#define COLOR_SCHEME_INCLUDED


#include "minorGems/graphics/Color.h"


typedef struct colorSet {

        // 4th is darker edge color
        Color elements[4];

    } colorSet;



class ColorScheme {
    
    public:   
        // random color scheme
        ColorScheme();
        
        // color scheme based on two given hues 
        ColorScheme( float inPrimaryHue, float inSecondaryHue );
       

        colorSet primary;
        colorSet secondary;
        
    protected:
        void populateScheme( float inPrimaryHue, float inSecondaryHue );

    };


#endif
