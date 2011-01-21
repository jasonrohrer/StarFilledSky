#include "musicPlayer.h"


// compaction of a note grid that contains just a single-note melody
// -1 means no note present
typedef struct NoteSequence {
        char noteYIndex[ N ];
        unsigned char partLength;
        int partIndex;
    } NoteSequence;


// default to no phase shifting
NoteSequence generateRandomNoteSequence( int inPartIndex,
                                         int inPartLength = 16 );


// two part drum beat
typedef struct DrumSequence {
        NoteSequence parts[2];
    } DrumSequence;


DrumSequence generateRandomDrumSequence( int inPartLength = 4 );




// applies a sequence to its part number in the musicPlayer
void setNoteSequence( NoteSequence inSequence );
