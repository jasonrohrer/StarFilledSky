#ifndef EYE_BORDER_SPRITE_INCLUDED
#define EYE_BORDER_SPRITE_INCLUDED


#include "BorderSprite.h"
#include "ColorScheme.h"


class EyeBorderSprite : public BorderSprite {
        

    public:
        EyeBorderSprite();
        
        virtual ~EyeBorderSprite();


        // override
        virtual void drawCenter( doublePair inPosition, double inFade = 1 );
        

        virtual void drawShadow( doublePair inPosition, double inFade = 1 );
        

        // must be normalized
        void setLookVector( doublePair inLookDir );
      
        void startSquint();
        
        ColorScheme getColors();

    protected:

        // must be called by sub-class to specify shadow
        // freed by this class
        void setShadow( SpriteHandle inShadow );
        
        
        ColorScheme mColors;
        
        doublePair mEyeOffset;
        
        char mFillMap[16][16];

        double mSquintTimeLeft;

        SpriteHandle mShadowSprite;

    };


#endif
