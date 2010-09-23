#include "minorGems/game/doublePair.h"
#include "minorGems/game/gameGraphics.h"


// base class
class BorderSprite {

    public:

        virtual ~BorderSprite();
        
        void drawBorder( doublePair inPosition, double inFade = 1 );
        
        void drawCenter( doublePair inPosition, double inFade = 1 );
        
        void draw( doublePair inPosition, double inFade = 1 );

        

    protected:
        
        BorderSprite();
        
        SpriteHandle mBorderSprite;
        SpriteHandle mCenterSprite;
        

    };

