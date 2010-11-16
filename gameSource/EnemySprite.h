#include "EyeBorderSprite.h"
#include "ColorScheme.h"


class EnemySprite : public EyeBorderSprite {
        

    public:

        // a random player sprite using a given scheme
        EnemySprite();
        
        // compacts sprite, discarding reproducible data
        // sprite cannot be drawn until decompact is called
        void compactSprite();
        
        void decompactSprite();
        


        

    protected:

        void generateReproducibleData();
        
        // free up memory consumed by reproducible data
        void freeReproducibleData();

        
        
        // random generator state that generated this sprite
        unsigned int mRandSeedState;

        char mDataGenerated;


        
    };
