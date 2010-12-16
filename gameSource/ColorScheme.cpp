#include "ColorScheme.h"


#include "minorGems/util/random/CustomRandomSource.h"


extern CustomRandomSource randSource;


// defaults to hue spread for analgous colors on a 12-segment color wheel
static colorSet makeColorSet( float inCenterHue, float inSaturation,
                              float inValue, float inValueRadius,
                              float inHueRadius = 1.0f / 12 ) {

    float offsetFromCenter = inHueRadius;

    colorSet c;
    
    Color *newColor;
    
    newColor = Color::makeColorFromHSV( inCenterHue,
                                        inSaturation,
                                        inValue );
    c.elements[0] = *( newColor );

    delete newColor;
    

    float leftHue = inCenterHue - offsetFromCenter;
    if( leftHue < 0 ) {
        leftHue += 1;
        }


    float leftValue = inValue * ( 1 + inValueRadius );
    if( leftValue > 1 ) {
        leftValue = 1;
        }
    
    
    newColor = Color::makeColorFromHSV( leftHue,
                                        inSaturation,
                                        leftValue );
    c.elements[1] = *( newColor );
    delete newColor;
    
    float rightHue = inCenterHue + offsetFromCenter;
    if( rightHue > 1 ) {
        rightHue -= 1;
        }

    float rightValue = inValue * ( 1 - inValueRadius );
    
    newColor = Color::makeColorFromHSV( rightHue,
                                        inSaturation,
                                        rightValue );
    
    c.elements[2] = *( newColor );
    delete newColor;
    
    // edge
    float edgeValue = inValue * 0.35;

    newColor = Color::makeColorFromHSV( rightHue,
                                        inSaturation,
                                        edgeValue );
    
    c.elements[3] = *( newColor );
    delete newColor;
    
    return c;
    }



void ColorScheme::populateScheme( float inPrimaryHue, float inSecondaryHue ) {
    // primaries are saturated
    float primarySaturation = randSource.getRandomBoundedDouble( 0.85, 1 );
    
    // dark
    float primaryValue = randSource.getRandomBoundedDouble( 0.45, 0.65 );


    primary = makeColorSet( inPrimaryHue, primarySaturation,
                            primaryValue, 0.5, 0.125 );


    
    // secondaries are less saturated
    float secondarySaturation = randSource.getRandomBoundedDouble( 0.25, 
                                                                   0.45 );

    // brighter
    float secondaryValue = randSource.getRandomBoundedDouble( 0.65, 0.65 );

    secondary = makeColorSet( inSecondaryHue, secondarySaturation,
                              secondaryValue, 0.35, 1.0f / 12 );
                              

    // half way between
    float specialHue = ( inPrimaryHue + inSecondaryHue ) / 2;
    if( inPrimaryHue == inSecondaryHue ) {
        // no half-way
        // go around wheel by 1/4 to get a different-looking color
        specialHue = inPrimaryHue += 0.25;
        if( specialHue > 1 ) {
            specialHue -= 1;
            }
        }
    
    float specialSat = ( primarySaturation + secondarySaturation ) / 2;
    
    Color *newColor = Color::makeColorFromHSV( specialHue,
                                               specialSat,
                                               secondaryValue );
    special = *newColor;

    delete newColor;
    }

    

ColorScheme::ColorScheme() {

    /*
    Color *makeColorFromHSV(
            float inHue, float inSaturation, float inValue,
            float inAlpha=1, char inBuildComposite=false );
    */
    float primaryCenterHue = randSource.getRandomFloat();

    // complement
    float secondaryCenterHue = primaryCenterHue + 0.5;
    
    if( secondaryCenterHue > 1 ) {
        secondaryCenterHue -= 1;
        }

    populateScheme( primaryCenterHue, secondaryCenterHue );
    }



ColorScheme::ColorScheme( float inPrimaryHue, float inSecondaryHue ) {
    populateScheme( inPrimaryHue, inSecondaryHue );
    }


