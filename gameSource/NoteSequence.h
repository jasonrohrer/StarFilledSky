#include "musicPlayer.h"


// compaction of a note grid that contains just a single-note melody
// -1 means no note present
typedef struct NoteSequence {
        char noteYIndex[ N ];
        unsigned char partLength;
    } NoteSequence;


NoteSequence generateRandomNoteSequence();


// applies a sequence to a part number in the musicPlayer
void setNoteSequence( NoteSequence inSequence, int inPartNumber );
