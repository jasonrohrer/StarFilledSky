#include "BorderSprite.h"
#include "ColorScheme.h"


class EnemySprite : public BorderSprite {
        

    public:

        // a random player sprite using a given scheme
        EnemySprite();
        

        ColorScheme getColors();

    protected:
        
        ColorScheme mColors;
        
    };
