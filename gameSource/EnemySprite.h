#include "BorderSprite.h"
#include "ColorScheme.h"


class EnemySprite : public BorderSprite {
        

    public:

        // a random player sprite using a given scheme
        EnemySprite();
        
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
