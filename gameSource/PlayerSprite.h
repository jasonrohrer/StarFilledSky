#include "EyeBorderSprite.h"


class PlayerSprite : public EyeBorderSprite {
        

    public:

        // a random player sprite, possibly using a given scheme
        PlayerSprite( ColorScheme *inColors=NULL, char inInvert=true );
        
        // compacts sprite, discarding reproducible data
        // sprite cannot be drawn until decompact is called
        void compactSprite();
        
        void decompactSprite();
        
        
        // override to draw two eyes
        virtual void drawCenter( doublePair inPosition, double inFade = 1 );


    protected:
        
        void generateReproducibleData();
        
        // free up memory consumed by reproducible data
        void freeReproducibleData();

        
        // random generator state that generated this sprite
        unsigned int mRandSeedState;

        char mDataGenerated;
        
    };
