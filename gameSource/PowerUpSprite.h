#include "BorderSprite.h"
#include "ColorScheme.h"
#include "PowerUpSet.h"


class PowerUpSprite : public BorderSprite {
        

    public:

        PowerUpSprite( PowerUp inPower, PowerUpSet *inSubPowers );
        

        // override these from BorderSprite to draw fixed sprites from sprite
        // bank
        virtual void drawBorder( doublePair inPosition, double inFade = 1 );
        
        virtual void drawCenter( doublePair inPosition, double inFade = 1 );
        
        virtual void draw( doublePair inPosition, double inFade = 1 );

        ColorScheme getColors();

        char mKeepUpdated;
        
    protected:
        PowerUp mPower;
        PowerUpSet *mSubPowers;
        
        ColorScheme mScheme;
    };
