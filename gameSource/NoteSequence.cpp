#include "NoteSequence.h"
#include "musicPlayer.h"

#include <string.h>

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



NoteSequence generateRandomNoteSequence() {
    NoteSequence s;
    
    // s.partLength = randSource.getRandomBoundedInt( 5, N );
    
    // try no phase shifting...
    s.partLength = N;

    memset( s.noteYIndex, -1, N );
    

    int numNotesInPart = 0;
    while( numNotesInPart < 2 ) {
        for( int x=0; x<s.partLength; x++ ) {
            if( randSource.getRandomBoundedInt( 0, 10 ) > 8 ) {        
                // at most one note in each timbre-column
                int y = randSource.getRandomBoundedInt( 0, N - 1 );
                s.noteYIndex[x] = y;
                numNotesInPart++;
                }
            }
        }

    return s;
    }



// applies a sequence to a part number in the musicPlayer
void setNoteSequence( NoteSequence inSequence, int inPartNumber ) {
    
    for( int x=0; x<N; x++ ) {
        
        // clear out existing notes in column
        for( int y=0; y<N; y++ ) {        
            noteToggles[inPartNumber][y][x] = false;
            }
        
        if( inSequence.noteYIndex[x] != -1 ) {
            noteToggles
                [inPartNumber]
                [ (int)( inSequence.noteYIndex[x] ) ]
                [x] = true;
            }            
        }

    partLengths[inPartNumber] = inSequence.partLength;
    }

