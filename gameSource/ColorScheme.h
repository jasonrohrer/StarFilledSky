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
       

        // inverts the colors in this scheme
        void invert();
        
        
        colorSet primary;
        colorSet secondary;
        
        // color that stands out from primary and secondary
        Color special;
        
    protected:
        void populateScheme( float inPrimaryHue, float inSecondaryHue );

    };


#endif
