#include "BorderSprite.h"
#include "ColorScheme.h"


class PlayerSprite : public BorderSprite {
        

    public:

        // a random player sprite, possibly using a given scheme
        PlayerSprite( ColorScheme *inColors=NULL );
        

        ColorScheme getColors();

    protected:
        
        ColorScheme mColors;
        
    };
