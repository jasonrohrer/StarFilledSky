#include "musicPlayer.h"


// compaction of a note grid that contains just a single-note melody
// -1 means no note present
typedef struct NoteSequence {
        char noteYIndex[ N ];
        unsigned char partLength;
        int partIndex;
    } NoteSequence;


NoteSequence generateRandomNoteSequence( int inPartIndex );


// applies a sequence to its part number in the musicPlayer
void setNoteSequence( NoteSequence inSequence );
