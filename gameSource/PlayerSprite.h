#include "BorderSprite.h"
#include "ColorScheme.h"


class PlayerSprite : public BorderSprite {
        

    public:

        // a random player sprite, possibly using a given scheme
        PlayerSprite( ColorScheme *inColors=NULL );
        
        // compacts sprite, discarding reproducible data
        // sprite cannot be drawn until decompact is called
        void compactSprite();
        
        void decompactSprite();
        

        ColorScheme getColors();

    protected:
        
        void generateReproducibleData();
        
        // free up memory consumed by reproducible data
        void freeReproducibleData();

        
        ColorScheme mColors;
        
        // random generator state that generated this sprite
        unsigned int mRandSeedState;

        char mDataGenerated;
        
    };
