#include "BorderSprite.h"
#include "ColorScheme.h"


class PlayerSprite : public BorderSprite {
        

    public:

        // a random player sprite using a given scheme
        PlayerSprite();
        

        ColorScheme getColors();

    protected:
        
        ColorScheme mColors;
        
    };
