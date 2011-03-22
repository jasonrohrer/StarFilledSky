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



/*
static int fact( int inN ) {
    // fact(0) defined as 1
    int value = 1;
    
    for( int i=1; i<=inN; i++ ) {
        value *= i;
        }
    return value;
    }


// factorial-based formula overflows for large N (N>12)
static int choose( int inN, int inK ) {

    if( inN < inK ) {
        return 0;
        }
    
    // works in case of K = N
    return 
        fact( inN ) / 
        ( fact( inK ) * fact( inN - inK ) );
    }
*/

// recursive formula does not overflow unless answer itself overflows
static int choose( int inN, int inK ) {
    
    if( inK == 0 ) {
        return 1;
        }
    if( inN < inK ) {
        return 0;
        }

    // recurse
    return choose( inN - 1, inK - 1 ) + choose( inN - 1, inK );
    }








// indexes K-combinations on natural numbers in lexographical order 
// See:
// http://en.wikipedia.org/wiki/Combinatorial_number_system
// outCombination must have K spots in it for subet of natural numbers to
// be returned
static void combIndex( int inIndex, int inK, int *outCombination ) {

    int remainder = inIndex;
    

    while( inK > 0 ) {
    
        // find N choose K that consumes largest (but not larger) portion
        // of remainder

        int c = inK - 1;
        
        while( choose( c, inK ) <= remainder ) {
            c++;
            }
        
        // passed remainder, take last c
        c--;
        
        outCombination[ inK - 1 ] = c;

        remainder -= choose( c, inK );
        
        inK --;
        }
    }




NoteSequence generateFlagNoteSequence( int inPartIndex,
                                       const char *inFlagString ) {
    /*
    // test code
    int comb[9];


    int n = 16;
    int k = 9;


    int numComb = choose( n, k );
    
    printf( "All %d (%d choose %d) combinations:\n", numComb, n, k );
    
    

    for( int i=0; i<numComb; i++ ) {
        combIndex( i, k, comb );
        
        printf( "%d:  { ", i );
        
        for( int j=0; j<k; j++ ) {
            printf( "%d", comb[j] );
            if( j<k-1 ) {
                printf( ", " );
                }
            }
        printf( " }\n" );
        }
    

    exit( 0 );
    */



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

    // keep a note sum
    int noteSum = 0;

    // if sum is even, skip a column
    // if odd, do not skip a column


    // This is bad, and doesn't produce most of the interesting combinations
    // check this article:
    // http://en.wikipedia.org/wiki/Combinatorial_number_system

    int c = 0;
    
    int x = 0;
    
    while( c < N && x < 9) {
        
        int noteHeight = hexTo16( inFlagString[x] );
        x++;
        
        s.noteYIndex[c] = noteHeight;
        
        noteSum += noteHeight;
        
        if( noteSum % 2 == 0 ) {
            c += 2;
            }
        else {
            c += 1;
            }
        }
    
    printf( "Note sequence: " );
    for( int i=0; i<N; i++ ) {
        printf( "%d, ", s.noteYIndex[i] );
        }
    printf( "\n" );
    

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

