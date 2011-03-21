#include "NoteSequence.h"
#include "musicPlayer.h"
#include "flagSprite.h"

#include <string.h>

#include "minorGems/util/random/CustomRandomSource.h"

extern CustomRandomSource randSource;



NoteSequence generateRandomNoteSequence( int inPartIndex,
                                         int inPartLength ) {
    NoteSequence s;
    s.partIndex = inPartIndex;
    
    // s.partLength = randSource.getRandomBoundedInt( 5, N );
    
    // allow caller to determine part length and phase shifting
    s.partLength = inPartLength;

    memset( s.noteYIndex, -1, N );
    

    int numNotesInPart = 0;
    while( numNotesInPart < 2 ) {
        for( int x=0; x<s.partLength; x++ ) {
            if( randSource.getRandomBoundedInt( 0, 10 ) > 6 ) {        
                // at most one note in each timbre-column
                int y = randSource.getRandomBoundedInt( 0, N - 1 );
                s.noteYIndex[x] = y;
                numNotesInPart++;
                }
            }
        }

    return s;
    }




NoteSequence generateFlagNoteSequence( int inPartIndex,
                                       const char *inFlagString ) {
    NoteSequence s;
    s.partIndex = inPartIndex;
    
    s.partLength = N;
    
    // clear
    memset( s.noteYIndex, -1, N );
    

    if( strcmp( inFlagString, "BLANKFLAG" ) == 0 ) {
        // leave blank pattern in place
        return s;
        }
    
    // else generate a pattern from the string

    // for now, just fill first 9 notes
    for( int x=0; x<9; x++ ) {
        s.noteYIndex[x] = hexTo16( inFlagString[x] );
        }
    
    return s;
    }




DrumSequence generateRandomDrumSequence( int inPartLength ) {
    DrumSequence s;
    
    s.parts[0].partIndex = PARTS - 4;
    s.parts[1].partIndex = PARTS - 3;

    
    int minNotesPerPart = inPartLength / 4;
    int maxNotesPerPart = ( inPartLength / 4 ) * 3;

    for( int p=0; p<2; p++ ) {
        
        s.parts[p].partLength = inPartLength;
        
        memset( s.parts[p].noteYIndex, -1, N );

        int numNotesInPart = 0;
        while( numNotesInPart < minNotesPerPart ) {
            for( int x=0; 
                 x<inPartLength && 
                     numNotesInPart < maxNotesPerPart; 
                 x++ ) {
                
                if( randSource.getRandomBoundedInt( 0, 10 ) > 6 ) {        
                    // all drum beats use middle notes only
                    s.parts[p].noteYIndex[x] = N/2;
                    numNotesInPart++;
                    }
                }
            }
        }
    
    return s;
    }


DrumSequence generateStraightDrumSequence( int inPartLength ) {
        DrumSequence s;
    
    s.parts[0].partIndex = PARTS - 4;
    s.parts[1].partIndex = PARTS - 3;

    
    for( int p=0; p<2; p++ ) {
        
        s.parts[p].partLength = inPartLength;
        
        memset( s.parts[p].noteYIndex, -1, N );

        for( int x=0; x<inPartLength; x++ ) {
            
            if( p == 0 && x == 0 ) {
                // snare
                s.parts[p].noteYIndex[x] = N/2;
                }
            else if( p == 1 && x % 2 == 0 ) {
                // kick
                s.parts[p].noteYIndex[x] = N/2;
                }
            }
        }
    
    return s;
    }





// applies a sequence to a part number in the musicPlayer
void setNoteSequence( NoteSequence inSequence ) {
    
    for( int x=0; x<N; x++ ) {
        
        // clear out existing notes in column
        for( int y=0; y<N; y++ ) {        
            noteToggles[inSequence.partIndex][y][x] = false;
            }
        
        if( inSequence.noteYIndex[x] != -1 ) {
            noteToggles
                [inSequence.partIndex]
                [ (int)( inSequence.noteYIndex[x] ) ]
                [x] = true;
            }            
        }

    partLengths[inSequence.partIndex] = inSequence.partLength;
    }

