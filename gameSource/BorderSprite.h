#ifndef BORDER_SPRITE_INCLUDED
#define BORDER_SPRITE_INCLUDED


#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"

#include "ColorScheme.h"


// base class
class BorderSprite {

    public:

        virtual ~BorderSprite();
        
        virtual void drawBorder( doublePair inPosition, double inFade = 1 );
        
        virtual void drawCenter( doublePair inPosition, double inFade = 1 );
        
        virtual void draw( doublePair inPosition, double inFade = 1 );

        virtual ColorScheme getColors() = 0;


    protected:
        
        BorderSprite();
        
        SpriteHandle mBorderSprite;
        SpriteHandle mCenterSprite;
        

    };


#endif
