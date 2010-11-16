#ifndef EYE_BORDER_SPRITE_INCLUDED
#define EYE_BORDER_SPRITE_INCLUDED


#include "BorderSprite.h"
#include "ColorScheme.h"


class EyeBorderSprite : public BorderSprite {
        

    public:
        EyeBorderSprite();
        


        // override
        virtual void drawCenter( doublePair inPosition, double inFade = 1 );


        // must be normalized
        void setLookVector( doublePair inLookDir );
      
        
        ColorScheme getColors();

    protected:
        
        ColorScheme mColors;

        doublePair mEyeOffset;
        
        char mFillMap[16][16];
    };


#endif
