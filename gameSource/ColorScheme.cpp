#include "ColorScheme.h"


#include "minorGems/util/random/CustomRandomSource.h"


extern CustomRandomSource randSource;


static colorSet makeColorSet( float inCenterHue, float inSaturation,
                              float inValue ) {

    // for analgous colors on a 12-segment color wheel
    float offsetFromCenter = 1.0f / 12;

    colorSet c;
    
    c.elements[0] = *( Color::makeColorFromHSV( inCenterHue,
                                                inSaturation,
                                                inValue ) );
    float leftHue = inCenterHue - offsetFromCenter;
    if( leftHue < 0 ) {
        leftHue += 1;
        }
    
    c.elements[1] = *( Color::makeColorFromHSV( leftHue,
                                                inSaturation,
                                                inValue ) );
    
    float rightHue = inCenterHue + offsetFromCenter;
    if( rightHue > 1 ) {
        rightHue -= 1;
        }
    
    c.elements[2] = *( Color::makeColorFromHSV( rightHue,
                                                inSaturation,
                                                inValue ) );

    
    return c;
    }


    

ColorScheme::ColorScheme() {

    /*
    Color *makeColorFromHSV(
            float inHue, float inSaturation, float inValue,
            float inAlpha=1, char inBuildComposite=false );
    */
    float primaryCenterHue = randSource.getRandomFloat();
    
    
    // primaries are saturated
    float primarySaturation = randSource.getRandomBoundedDouble( 0.85, 1 );
    
    // dark
    float primaryValue = randSource.getRandomBoundedDouble( 0.45, 0.65 );


    primary = makeColorSet( primaryCenterHue, primarySaturation,
                            primaryValue );


    // complement
    float secondaryCenterHue = primaryCenterHue + 0.5;
    
    if( secondaryCenterHue > 1 ) {
        secondaryCenterHue -= 1;
        }
    
    // secondaries are less saturated
    float secondarySaturation = randSource.getRandomBoundedDouble( 0.5, 0.85 );
    
    secondary = makeColorSet( secondaryCenterHue, secondarySaturation,
                              primaryValue );
    }

