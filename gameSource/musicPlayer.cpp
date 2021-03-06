
#include "musicPlayer.h"

#include "Timbre.h"
#include "Envelope.h"

#include "beatTracker.h"


#include "minorGems/game/game.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/log/AppLog.h"
#include "minorGems/system/Time.h"


#include <math.h>
#include <stdlib.h>



static int beatPart = 22;


// whether note is currently on and playing or not
// toggle these to control music as it plays

char noteToggles[PARTS][N][N];

// some parts loop sooner or have silence at end (longer than N), 
// allowing phase cycles between parts
int partLengths[PARTS];


double partStepDurations[PARTS];
int partStepDurationsInSamples[PARTS];


double partLoudness[PARTS];
double partStereo[PARTS];







int sampleRate;



// 16x16 tone matrix used in each phrase of each part
int w = N;
int h = N;

// total number of samples played so far
int streamSamples = 0;

// offset into grid at start
// for testing
int gridStartOffset = 0;



// overal loudness of music
double musicLoudness = 1.0;






// one grid step in seconds
double gridStepDuration;
int gridStepDurationInSamples;


void setSpeed( int inSpeed ) {
    lockAudio();

    //double oldDuration = gridStepDuration;
    
    switch( inSpeed ) {
        case 0:
            gridStepDuration = 1;
            break;
        case 1:
            gridStepDuration = 0.5;
            break;
        case 2:
            gridStepDuration = 0.25;
            break;
        }

    //double speedMultiple = gridStepDuration / oldDuration;
    
    gridStepDurationInSamples = (int)( gridStepDuration * sampleRate );
    
    // jump in stream to maintain our current grid location
    // otherwise, we're in danger of playing the same note twice simultaneously

    unlockAudio();
    }


//double entireGridDuraton;


// c
//double keyFrequency = 261.63;

// actually, can't support high notes in the key of c w/out round-off errors
// because this is a wavetable implementation, and when the tables get short,
// the errors get huge
// THIS, however, evenly divides our sample rate (22050)
// which means that we get perfect, whole-number wave tables at octaves
double keyFrequency = 172.265625;




int numTimbres = PARTS;

Timbre *musicTimbres[ PARTS ];

int numEnvelopes = PARTS;

Envelope *musicEnvelopes[ PARTS ];


// waiting for destruction
SimpleVector<Timbre *> oldTimbres;
SimpleVector<Envelope *> oldEnvelopes;




class Note {
    public:
        // index into musicTimbres array
        int mTimbreNumber;

        // pointer to actual Timbre used when note was last sounded
        // (so it doesn't change out from under note, in middle of note)
        Timbre *mTimbrePointer;
        

        // index into musicEnvelopes array
        int mEnvelopeNumber;
                
        // pointer to actual Envelope, for same reason as above
        Envelope *mEnvelopePointer;
        

        int mScaleNoteNumber;
        
        // additional loudness adjustment
        // places note in stereo space
        double mLoudnessLeft;
        double mLoudnessRight;
        

        // start time, in seconds from start of note grid
        //double mStartTime;

        // duration in seconds
        //double mDuration;

        // used when note is currently playing to track progress in note
        // negative if we should wait before starting to play the note
        int mCurrentSampleNumber;

        // duration in samples, set each time the note is played
        // incase speed changes
        int mNumSamples;
        
        // set once
        int mNumGridSteps;
        

        // carry the grid step duration with us as we play, in case
        // it is changed before we're done playing
        int mOurGridStepDurationInSamples;
        

        Note *copy() {
            Note *note = new Note();
            
            note->mTimbreNumber = mTimbreNumber;
            note->mTimbrePointer = mTimbrePointer;
        
            note->mEnvelopeNumber = mEnvelopeNumber;
                
            note->mEnvelopePointer = mEnvelopePointer;
        
            note->mScaleNoteNumber = mScaleNoteNumber;
            note->mLoudnessLeft = mLoudnessLeft;
            note->mLoudnessRight = mLoudnessRight;
            note->mCurrentSampleNumber = mCurrentSampleNumber;
            
            note->mNumSamples = mNumSamples;
            note->mNumGridSteps = mNumGridSteps;
            note->mOurGridStepDurationInSamples = 
                mOurGridStepDurationInSamples;
            
            return note;
            }
        
    };


// all possible notes in a 16x16 phrase grid

// indexed as noteGrid[part][y][x]
Note *noteGrid[PARTS][N][N];





SimpleVector<Note*> currentlyPlayingNotes;



// need to synch these with audio thread

void setMusicLoudness( double inLoudness ) {
    lockAudio();
    
    musicLoudness = inLoudness;
    
    unlockAudio();
    }



double getMusicLoudness() {
    return musicLoudness;
    }



void restartMusic() {
    lockAudio();

    // return to beginning (and forget samples we've played so far)
    streamSamples = 0;
    
    // drop all currently-playing notes
    currentlyPlayingNotes.deleteAll();
        
    unlockAudio();
    }




// for testing timbre set
// call from audio callback to override whatever notes/volumes external
// program has been setting
int startTestPart = 10;
void setTestTones() {
    
    // let each part run for a full grid length before switching parts

    // assume all parts are same length
    int currentTestPart = 
        startTestPart + 
        streamSamples / ( partStepDurationsInSamples[startTestPart] * (w/2) );
    
    // wrap
    // last part is a copy slot, skip it
    currentTestPart = currentTestPart % (PARTS-1);
    

    for( int p=0; p<PARTS; p++ ) {
        partLoudness[p] = 0;
        }
    printf( "Current part = %d\n", currentTestPart );
    
    partLoudness[currentTestPart] = 1;

    for( int y=0; y<h; y++ ) {
        for( int x=0; x<w; x++ ) {
            noteToggles[currentTestPart][y][x] = false;
            }
        }
    // skip every other note up scale
    for( int x=0; x<w/2-1; x++ ) {
        noteToggles[currentTestPart][x*2][x] = true;

        // twice
        noteToggles[currentTestPart][x*2][x+w/2] = true;
        }
    }




// called by platform to get more samples
void getSoundSamples( Uint8 *inBuffer, int inLengthToFillInBytes ) {
    
    // turn on to override externally set notes for testing
    //setTestTones();
    

    // 2 bytes for each channel of stereo sample
    int numSamples = inLengthToFillInBytes / 4;
    

    Sint16 *samplesL = new Sint16[ numSamples ];
    Sint16 *samplesR = new Sint16[ numSamples ];
    
    // first, zero-out the buffer to prepare it for our sum of note samples
    // each sample is 2 bytes
    memset( samplesL, 0, 2 * numSamples );
    memset( samplesR, 0, 2 * numSamples );
    

    int i;


    // for each part
    for( int si=0; si<PARTS; si++ ) {


        // hop through all grid steps that *start* in this stream buffer
        // add notes that start during this stream buffer
        
        // how far into stream buffer before we hit our first grid step? 
        int startOfFirstGridStep = 
            streamSamples % partStepDurationsInSamples[si];
        
        if( startOfFirstGridStep != 0 ) {
            startOfFirstGridStep = 
                partStepDurationsInSamples[si] - startOfFirstGridStep;
            }
    



        // hop from start of grid step to start of next grid step
        // ignore samples in between, since notes don't start there,
        // and all we're doing right now is finding notes that start
        for( i=startOfFirstGridStep; 
             i<numSamples; 
             i += partStepDurationsInSamples[si] ) {
        
            // start of new grid position
            
            // check for new notes that are starting
            
            // map into our music image:
            int x = ( streamSamples + i ) / partStepDurationsInSamples[si];
          
                                
                
            // step in tone matrix for that part-step
            int matrixX = x % partLengths[si];
            
            // else silence when we go past end of matrix
            if( matrixX < w )
            for( int y=0; y<h; y++ ) {
                    
                Note *note = noteGrid[si][y][matrixX];
                
                if( note != NULL && 
                    noteToggles[si][y][matrixX] ) {
                    // new note
                    note = note->copy();
                    
                    if( si == beatPart ) {
                        // report beat hit
                        beatHit();
                        }
                    

                    currentlyPlayingNotes.push_back( note );
                    
                    // save pointer to active envelope and timbre
                    // when this note began
                    note->mTimbrePointer = musicTimbres[
                        note->mTimbreNumber ];
                    note->mEnvelopePointer = musicEnvelopes[
                        note->mEnvelopeNumber ];
                    
                    note->mTimbrePointer->mActiveNoteCount ++;
                    note->mEnvelopePointer->mActiveNoteCount ++;
                    
                    // tweak loudness based on part loudness and stereo
                    note->mLoudnessRight *= partLoudness[ si ];
                    note->mLoudnessLeft *= partLoudness[ si ];
                    
                    // constant power rule
                    double p = M_PI * partStereo[ si ] / 2;
                    
                    note->mLoudnessRight *= sin( p );
                    note->mLoudnessLeft *= cos( p );
                    
                    

                    // start it
                    
                    
                    // base these on our
                    // envelope (instead of currently set duration in
                    // player, which may have been updated before
                    // the envelopes were properly updated due to thread
                    // interleaving issues)
                    int envGridStepDuration = 
                        musicEnvelopes[ note->mEnvelopeNumber ]->
                            mGridStepDurationInSamples;
                    
                    // compute length
                    note->mNumSamples = 
                        note->mNumGridSteps * envGridStepDuration;
                    
                    note->mOurGridStepDurationInSamples = 
                        envGridStepDuration;
                    
                    
                    // set a delay for its start based on our position
                    // in this callback buffer
                    note->mCurrentSampleNumber = -i;
                    }            
                }
            }
        
        }
    
    
    
    
    streamSamples += numSamples;
    

    // loop over all current notes and add their samples to buffer
    for( int n=0; n<currentlyPlayingNotes.size(); n++ ) {
            
        Note *note = *( currentlyPlayingNotes.getElement( n ) );
             
        int waveTableNumber = note->mScaleNoteNumber;
        //Timbre *timbre = musicTimbres[ note->mTimbreNumber ];
        Timbre *timbre = note->mTimbrePointer;
        int tableLength = timbre->mWaveTableLengths[ waveTableNumber ];
            
        Sint16 *waveTable = timbre->mWaveTable[ waveTableNumber ];
        
        //Envelope *env = musicEnvelopes[ note->mEnvelopeNumber ];
        Envelope *env = note->mEnvelopePointer;
        double *envLevels = 
            env->getEnvelope( 
                // index envelope by number of grid steps in note
                note->mNumSamples / 
                note->mOurGridStepDurationInSamples );
        
        
        double noteLoudnessL = note->mLoudnessLeft;
        double noteLoudnessR = note->mLoudnessRight;
        
        // do this outside inner loop
        noteLoudnessL *= musicLoudness;
        noteLoudnessR *= musicLoudness;


        int noteStartInBuffer = 0;
        int noteEndInBuffer = numSamples;
        
        if( note->mCurrentSampleNumber < 0 ) {
            // delay before note starts in this sample buffer
            noteStartInBuffer = - note->mCurrentSampleNumber;
            
            // we've taken account of the delay
            note->mCurrentSampleNumber = 0;
            }

        char endNote = false;
        
        int numSamplesLeftInNote = 
            note->mNumSamples - note->mCurrentSampleNumber;
        
        if( noteStartInBuffer + numSamplesLeftInNote < noteEndInBuffer ) {
            // note ends before end of buffer
            noteEndInBuffer = noteStartInBuffer + numSamplesLeftInNote;
            endNote = true;
            }
        

        int waveTablePos = note->mCurrentSampleNumber % tableLength;
        
        int currentSampleNumber = note->mCurrentSampleNumber;
        
        for( i=noteStartInBuffer; i != noteEndInBuffer; i++ ) {
            double envelope = envLevels[ currentSampleNumber ];
            
            double monoSample = envelope * 
                waveTable[ waveTablePos ];
            

            samplesL[i] += (Sint16)( noteLoudnessL * monoSample );
            samplesR[i] += (Sint16)( noteLoudnessR * monoSample );
            
            currentSampleNumber ++;
            
            waveTablePos ++;
            
            // avoid using mod operator (%) in inner loop
            // found with profiler
            if( waveTablePos == tableLength ) {
                // back to start of table
                waveTablePos = 0;
                }
            
            }
        
        note->mCurrentSampleNumber += ( noteEndInBuffer - noteStartInBuffer );
        
        if( endNote ) {
            // note ended in this buffer
            currentlyPlayingNotes.deleteElement( n );
            n--;

            // note not using these anymore
            note->mTimbrePointer->mActiveNoteCount --;
            note->mEnvelopePointer->mActiveNoteCount --;
            
            if( note->mTimbrePointer->mActiveNoteCount == 0 ) {
                
                char stillUsed = false;
                for( int i=0; i<numTimbres && !stillUsed; i++ ) {
                    if( musicTimbres[ i ] == note->mTimbrePointer ) {
                        stillUsed = true;
                        }
                    }

                if( !stillUsed ) {
                    // this timbre is no longer used

                    oldTimbres.deleteElementEqualTo( note->mTimbrePointer );
                    delete note->mTimbrePointer;
                    }
                }

            if( note->mEnvelopePointer->mActiveNoteCount == 0 ) {
                
                char stillUsed = false;
                for( int i=0; i<numEnvelopes && !stillUsed; i++ ) {
                    if( musicEnvelopes[ i ] == note->mEnvelopePointer ) {
                        stillUsed = true;
                        }
                    }

                if( !stillUsed ) {
                    // this envelope is no longer used
                    oldEnvelopes.deleteElementEqualTo( 
                        note->mEnvelopePointer );
                    
                    delete note->mEnvelopePointer;
                    }
                
                }

            // this was a copy
            delete note;
            }
        
        }


    // now copy samples into Uint8 buffer
    int streamPosition = 0;
    for( i=0; i != numSamples; i++ ) {
        Sint16 intSampleL = samplesL[i];
        Sint16 intSampleR = samplesR[i];
        
        inBuffer[ streamPosition ] = (Uint8)( intSampleL & 0xFF );
        inBuffer[ streamPosition + 1 ] = (Uint8)( ( intSampleL >> 8 ) & 0xFF );
        
        inBuffer[ streamPosition + 2 ] = (Uint8)( intSampleR & 0xFF );
        inBuffer[ streamPosition + 3 ] = (Uint8)( ( intSampleR >> 8 ) & 0xFF );
        
        streamPosition += 4;
        }

    delete [] samplesL;
    delete [] samplesR;
    
    }



// limit on n, based on Nyquist, when summing sine components
//int nLimit = (int)( sampleRate * M_PI );
// actually, this is way too many:  it takes forever to compute
// use a lower limit instead
// This produces fine results (almost perfect square wave)
int nLimit = 40;



// square wave with period of 2pi
double squareWave( double inT ) {
    double sum = 0;
    
    for( int n=1; n<nLimit; n+=2 ) {
        sum += 1.0/n * sin( n * inT );
        }
    return sum;
    }



// sawtoot wave with period of 2pi
double sawWave( double inT ) {
    double sum = 0;
    
    for( int n=1; n<nLimit; n++ ) {
        sum += 1.0/n * sin( n * inT );
        }
    return sum;
    }


// white noise, ignores inT
double whiteNoise( double inT ) {
    return 2.0 * ( rand() / (double)RAND_MAX ) - 1.0;
    }


// white noise where each sample is averaged with last sample
// effectively a low-pass filter
double lastNoiseSample = 0;

double smoothedWhiteNoise( double inT ) {
    // give double-weight to last sample to make it even smoother
    lastNoiseSample = ( 2 * lastNoiseSample + whiteNoise( inT ) ) / 3;
    
    return lastNoiseSample;
    }



// square where each sample is averaged with last sample
// effectively a low-pass filter
double lastSquareSample = 0;

double smoothedSquareWave( double inT ) {
    // give double-weight to last sample to make it even smoother
    lastSquareSample = ( 4 * lastSquareSample + squareWave( inT ) ) / 5;
    
    return lastSquareSample;
    }


double harmonicSine( double inT ) {
    return
        1.0 * sin( inT ) 
        +
        0.5 * sin( 2 * inT )
        +
        0.25 * sin( 4 * inT );
    }


double harmonicSaw( double inT ) {
    return
        1.0 * sawWave( inT ) 
        +
        0.5 * sawWave( 2 * inT )
        +
        0.25 * sawWave( 4 * inT );
    }


double harmonicSquare( double inT ) {
    return
        1.0 * squareWave( inT ) 
        +
        0.5 * squareWave( 2 * inT )
        +
        0.25 * squareWave( 4 * inT );
    }

double harmonicSmoothedSquare( double inT ) {
    return
        1.0 * smoothedSquareWave( inT ) 
        +
        0.5 * smoothedSquareWave( 2 * inT )
        +
        0.25 * smoothedSquareWave( 4 * inT );
    }



static double coefficients[256];
static int numCoefficientsToUse;

double coefficientMix( double inT ) {
    double sum = 0;
    
    for( int n=1; n<=numCoefficientsToUse; n++ ) {
        sum += coefficients[n-1] * sin( n * inT );
        }
    return sum;
    }



// a sine wave that falls off over time
double sinThwip( double inT ) {
    return sin( inT / (1 + pow(inT, .2) ) ) ;
    }

double kickWave( double inT ) {
    return sinThwip( inT ) +
        // white noise at start, then fall-off
        smoothedWhiteNoise( inT ) * ( 1 - inT / (inT + 10 ) );
    }




void setTimbre( int inTimbreNumber,
                double *inPartialCoefficients, int numCoefficients,
                int inOctavesDown ) {    

    // set up coefficients used by mix function above
    numCoefficientsToUse = numCoefficients;
    
    for( int i=0; i<numCoefficients; i++ ) {
        coefficients[i] = inPartialCoefficients[i];
        }
    


    int heightPerTimbre = h;

    // possible for all notes in a column to be on at user's request
    // and notes are 3 long at max (decays), so consider overlap
    double  maxNoteLoudnessInAColumn = h * 3;

    double loudnessPerTimbre = 1.0 / maxNoteLoudnessInAColumn;


    //double t = Time::getCurrentTime();
    

    double freq = keyFrequency / pow( 2, inOctavesDown );

    Timbre *newTimbre = new Timbre( sampleRate, 1.0 * loudnessPerTimbre,
                                    freq,
                                    heightPerTimbre, coefficientMix, 1 );
    
    // now replace it
    lockAudio();

    if( musicTimbres[inTimbreNumber]->mActiveNoteCount > 0 ) {    
        // save old one, because some currently-playing notes are using it!
        oldTimbres.push_back( musicTimbres[inTimbreNumber] );
        }
    else {
        delete musicTimbres[inTimbreNumber];
        }
    
    
    
    musicTimbres[inTimbreNumber] = newTimbre;
    
    unlockAudio();
    }





void setScale( char inToneOn[12] ) {
    setTimbreScale( inToneOn );
    }




void setEnvelope( int inTimbreNumber,
                  double inAttack, double inHold,
                  double inRelease ) {

    if( inAttack + inHold + inRelease > 1.0 ) {
        AppLog::error( 
            "Attack + Hold + Release in specified envelope too long" );
        if( inAttack > 1 ) {
            inAttack = 1;
            }
        inHold = 1 - inAttack;
        inRelease = 0;
        }
    
    int maxNoteLength = 3;

    Envelope *newEnvelope =  new Envelope( inAttack, inHold, 
                                           inRelease,
                                           maxNoteLength,
                                           maxNoteLength,
                                           gridStepDurationInSamples );
    // replace it
    lockAudio();
    
    if( musicEnvelopes[inTimbreNumber]->mActiveNoteCount > 0 ) {    
        // save old one, because some currently-playing notes are using it!
        oldEnvelopes.push_back( musicEnvelopes[inTimbreNumber] );
        }
    else {
        delete musicEnvelopes[inTimbreNumber];
        }
    
    
    
    musicEnvelopes[inTimbreNumber] = newEnvelope;
    
    unlockAudio();
    }



void setDefaultMusicSounds() {

    setDefaultScale();
    
    
    lockAudio();

    for( int i=0; i<PARTS; i++ ) {
        
        if( musicTimbres[i] != NULL ) {
            if( musicTimbres[i]->mActiveNoteCount > 0 ) {    
            
                // save old one, because some currently-playing notes 
                // are using it!
                oldTimbres.push_back( musicTimbres[i] );
                }
            else {
                delete musicTimbres[i];
                }
            }
        if( musicEnvelopes[i] != NULL ) {
            if( musicEnvelopes[i]->mActiveNoteCount > 0 ) {    
            
                // save old one, because some currently-playing notes 
                // are using it!
                oldEnvelopes.push_back( musicEnvelopes[i] );
                }
            else {
                delete musicEnvelopes[i];
                }
            }
        }

    
    int heightPerTimbre = h;


    // possible for all notes in a column to be on at user's request
    // and notes are 3 long at max (decays), so consider overlap
    //double  maxNoteLoudnessInAColumn = h * 3;
    

   
    // divide loudness amoung timbres to avoid clipping
    // for now, assume at most one simultaneous note per part
    // double loudnessPerTimbre = 1.0 / maxNoteLoudnessInAColumn;
    double loudnessPerTimbre = 1.0 / PARTS;// / maxNoteLoudnessInAColumn;


    // next, compute the longest note in the song
    // fixed at 2
    int maxNoteLength = 3;
        
    AppLog::getLog()->logPrintf( 
        Log::INFO_LEVEL,
        "Max note length in song = %d\n", maxNoteLength );

    
    
    // further adjust loudness per channel here as we construct
    // each timbre.

    //double t = Time::getCurrentTime();













    // first, power-up parts, slower and more organic
    
    
    musicTimbres[10] = new Timbre( sampleRate, loudnessPerTimbre,
                                  keyFrequency / 2,
                                  heightPerTimbre, sin );   

    musicEnvelopes[10] = new Envelope( 0.5, 0.5, 0, 0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[10] );
 


    musicTimbres[11] = new Timbre( sampleRate, 0.3 * loudnessPerTimbre,
                                  keyFrequency,
                                  heightPerTimbre, harmonicSine );
    
    musicEnvelopes[11] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[11] );



    musicTimbres[12] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                  keyFrequency / 2,
                                  heightPerTimbre, harmonicSine );

    musicEnvelopes[12] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[12] );


    musicTimbres[13] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                  keyFrequency,
                                  heightPerTimbre, sin );

    musicEnvelopes[13] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[13] );


    musicTimbres[14] = new Timbre( sampleRate, 0.5 * loudnessPerTimbre,
                                  keyFrequency / 4,
                                  heightPerTimbre, sawWave );

    musicEnvelopes[14] = new Envelope( 0.5, 0.5, 0, 0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[14] );


    musicTimbres[15] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                  keyFrequency / 4,
                                  heightPerTimbre, harmonicSmoothedSquare );

    musicEnvelopes[15] = new Envelope( 0.5, 0.5, 0, 0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[15] );



    musicTimbres[16] = new Timbre( sampleRate, 0.3 * loudnessPerTimbre,
                                  keyFrequency,
                                  heightPerTimbre, harmonicSaw );
    
    musicEnvelopes[16] = new Envelope( 0.5, 0.5, 0, 0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[16] );



    musicTimbres[17] = new Timbre( sampleRate, loudnessPerTimbre,
                                  keyFrequency / 4,
                                  heightPerTimbre, harmonicSine );

    musicEnvelopes[17] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[17] );


    musicTimbres[18] = new Timbre( sampleRate, 0.3 * loudnessPerTimbre,
                                  keyFrequency,
                                  heightPerTimbre, sawWave );
    
    musicEnvelopes[18] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[18] );


    musicTimbres[19] = new Timbre( sampleRate, 0.6 * loudnessPerTimbre,
                                  0.5 * keyFrequency,
                                  heightPerTimbre, smoothedSquareWave );
    
    musicEnvelopes[19] = new Envelope( 0.5, 0.5, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[19] );




    // enemy parts
    // faster and staccato
    
    musicTimbres[0] = new Timbre( sampleRate, loudnessPerTimbre,
                                   keyFrequency / 2,
                                   heightPerTimbre, sin );   

    musicEnvelopes[0] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[0] );
 


    musicTimbres[1] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, harmonicSine );
    
    musicEnvelopes[1] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[1] );



    musicTimbres[2] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency * 2,
                                   heightPerTimbre, sin );

    musicEnvelopes[2] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[2] );


    musicTimbres[3] = new Timbre( sampleRate, loudnessPerTimbre,
                                   keyFrequency / 4,
                                   heightPerTimbre, harmonicSine );

    musicEnvelopes[3] = new Envelope( 0.02, 0.25, 0.0, 0.0,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[3] );


    musicTimbres[4] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, harmonicSmoothedSquare );

    musicEnvelopes[4] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[4] );


    musicTimbres[5] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency / 4,
                                   heightPerTimbre, harmonicSaw );
    
    musicEnvelopes[5] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[5] );



    musicTimbres[6] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, harmonicSaw );
    
    musicEnvelopes[6] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[6] );



    musicTimbres[7] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   2 * keyFrequency,
                                   heightPerTimbre, harmonicSquare );

    musicEnvelopes[7] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[7] );
    

    musicTimbres[8] = new Timbre( sampleRate, 0.5 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, sawWave );
    
    musicEnvelopes[8] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[8] );
    

    musicTimbres[9] = new Timbre( sampleRate, 0.8 * loudnessPerTimbre,
                                   0.5 * keyFrequency,
                                   heightPerTimbre, smoothedSquareWave );
    
    musicEnvelopes[9] = new Envelope( 0.02, 0.25, 0, 0,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[9] );
    


    // try swapping enemy and power parts
    if(false)
    for( int i=0; i<10; i++ ) {
        Timbre *tempTimbre = musicTimbres[i];
        Envelope *tempEnv = musicEnvelopes[i];
    

        musicTimbres[i] = musicTimbres[i+10];
        musicEnvelopes[i] = musicEnvelopes[i+10];
        
        musicTimbres[i+10] = tempTimbre;
        musicEnvelopes[i+10] = tempEnv;
        }



    // flag parts
    musicTimbres[20] = new Timbre( sampleRate, 0.6 * loudnessPerTimbre,
                                   keyFrequency/2,
                                   heightPerTimbre, harmonicSaw );
    // AHR model
    musicEnvelopes[20] = new Envelope( 0.05, 0.1, 0.3,
                                       maxNoteLength,
                                       maxNoteLength,
                                       partStepDurationsInSamples[20] );


    musicTimbres[21] = new Timbre( sampleRate, 0.6 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, smoothedSquareWave );
    // AHR model
    musicEnvelopes[21] = new Envelope( 0.05, 0.1, 0.3,
                                      maxNoteLength,
                                      maxNoteLength,
                                      partStepDurationsInSamples[21] );


    

    
    // rise marker parts
    
    // snare type sound
    musicTimbres[22] = new Timbre( sampleRate, 0.7 * loudnessPerTimbre,
                                   keyFrequency/2,
                                   heightPerTimbre, smoothedWhiteNoise,
                                   // extra periods per table so that
                                   // noise doesn't become tonal through
                                   // short looping
                                   10 ); 

    musicEnvelopes[22] = new Envelope(
        0.0, 0.125, 0.0, 0.0,
        maxNoteLength,
        maxNoteLength,
        partStepDurationsInSamples[22] );


    // kick drum type sound
    musicTimbres[23] = new Timbre( sampleRate, 1.0 * loudnessPerTimbre,
                                   keyFrequency,
                                   heightPerTimbre, kickWave,
                                   // extra periods in table to make room
                                   // for entire kick sweep
                                   200 ); 

    musicEnvelopes[23] = new Envelope(
        // AHR model
        0.0, 0.25, 0.05,
        maxNoteLength,
        maxNoteLength,
        partStepDurationsInSamples[23] );
    
    
    



    // player part
    musicTimbres[PARTS-2] = new Timbre( sampleRate, 0.5 * loudnessPerTimbre,
                                        keyFrequency/2,
                                        heightPerTimbre, harmonicSine ); 
    
    musicEnvelopes[PARTS-2] = new Envelope(
        0.01, 0.99, 0.0, 0.0,
        maxNoteLength,
        maxNoteLength,
        partStepDurationsInSamples[PARTS-2] );
    

    // harmony part, copied

    musicTimbres[PARTS-1] = musicTimbres[PARTS-2];
    musicEnvelopes[PARTS-1] = musicEnvelopes[PARTS-2];


    for( int i=0; i<PARTS; i++ ) {
        partLoudness[i] = 0;
        partStereo[i] = 0.5;
        }
    
    // player part and super-part
    // constant loudness, and centered
    partLoudness[PARTS - 1] = 1;
    partLoudness[PARTS - 2] = 1;
    partStereo[PARTS - 1] = 0.5;
    partStereo[PARTS - 2] = 0.5;
    

    
    unlockAudio();

    }






void initMusicPlayer() {


    /*
    // test for profiler
    for( int i=0; i<20; i++ ) {
        
        Envelope *newEnvelope =  new Envelope( 0.25, 0.5, 
                                               0.25,
                                               10,
                                               gridStepDurationInSamples );
        delete newEnvelope;
        }
    exit( 0 );
    */    



    
    // setDefaultScale();
    

    //entireGridDuraton = gridStepDuration * w;
    
    sampleRate = getSampleRate();

    
    gridStepDuration = 0.25;
    gridStepDurationInSamples = (int)( gridStepDuration * sampleRate );

    // set special durations for parts
    for( int i=0; i<PARTS; i++ ) {
        partStepDurations[i] = gridStepDuration;
        partStepDurationsInSamples[i] = gridStepDurationInSamples;
        }

    
    // power-ups are special
    for( int i=10; i<20; i++ ) {
        partStepDurations[i] *= 2;
        partStepDurationsInSamples[i] *= 2;
        }
    // enemies are special
    for( int i=0; i<10; i++ ) {
        partStepDurations[i] *= 0.5;
        partStepDurationsInSamples[i] /= 2;
        }
    
    // last part is special
    //partStepDurations[PARTS - 1] *= 2;
    //partStepDurationsInSamples[PARTS - 1] *= 2;
    




    // jump ahead in stream, if needed
    streamSamples += gridStartOffset * gridStepDurationInSamples;

    int heightPerTimbre = h;

    AppLog::getLog()->logPrintf( 
        Log::INFO_LEVEL,
        "Height in grid per timbre = %d\n", heightPerTimbre );
    

    // nullify so they are not added to old list by setDefaultMusicSounds
    for( int i=0; i<PARTS; i++ ) {
        musicTimbres[i] = NULL;
        musicEnvelopes[i] = NULL;
        }

    setDefaultMusicSounds();
    

    

    // fix L/R loudness for all notes, based on their timbres
    
    double leftLoudness[PARTS];//  = { 0.75, 1.0, 1.0 };
    double rightLoudness[PARTS];// = { 1.0, 0.75, 1.0 };
    
            

    
    for( int si=0; si<PARTS; si++ ) {
        
        //leftLoudness[si] = 0.75 + (si+1) / (double)PARTS;
        //rightLoudness[si] = 1.0 - 0.75 * (si+1) / (double)PARTS;
        // live per-part panning now
        leftLoudness[si] = 1;
        rightLoudness[si] = 1;
        

        for( int y=0; y<h; y++ ) {
            
            for( int x=0; x<w; x++ ) {
                
                // default to NULL
                noteGrid[si][y][x] = NULL;
            
                /*
                // the note number in our scale
                // scale starts over for each timbre 
                int noteNumber = y % heightPerTimbre;
                
                if( y == h-1 ) {
                // highest note, hangs off top
                noteNumber = heightPerTimbre;
                }
                */
                int noteNumber = y;
            

                // start a new note
                noteGrid[si][y][x] = new Note();
                
                noteGrid[si][y][x]->mScaleNoteNumber = noteNumber;
                
                noteGrid[si][y][x]->mTimbreNumber = si;

                noteGrid[si][y][x]->mTimbrePointer = musicTimbres[ si ];
            

                // same as timbre number
                noteGrid[si][y][x]->mEnvelopeNumber = 
                    noteGrid[si][y][x]->mTimbreNumber;
                
                noteGrid[si][y][x]->mEnvelopePointer = musicEnvelopes[ si ];


                // left loudness fixed
                noteGrid[si][y][x]->mLoudnessLeft = 
                    leftLoudness[ noteGrid[si][y][x]->mTimbreNumber ];
                    
                // right loudness fixed
                noteGrid[si][y][x]->mLoudnessRight = 
                    rightLoudness[ noteGrid[si][y][x]->mTimbreNumber ];
                

                //noteGrid[si][y][x]->mStartTime = gridStepDuration * x;
                
                // three grid steps long, for overlap
                //noteGrid[si][y][x]->mDuration = 
                //    gridStepDuration * 3;
                noteGrid[si][y][x]->mNumGridSteps = 3;
                
                // set this when the note is played
                //noteGrid[si][y][x]->mNumSamples = 
                //    gridStepDurationInSamples * 3;
                }
            }
        }
    
    
    for( int i=0; i<PARTS; i++ ) {
        for( int y=0; y<h; y++ ) {
            
            for( int x=0; x<w; x++ ) {
                
                // all notes start off off
                noteToggles[i][y][x] = false;
                }
            }
        partLengths[i] = w;
        }
    

    // fixed rhythm part for rise marker
    for( int n=0; n<N; n++ ) {
        if( n%4 == 0 ) {
            noteToggles[22][N/2][n] = true;
            }
        if( n%2 == 0 ) {
            noteToggles[23][N/2][n] = true;
            }
        }
    
    
    

        
    // test
    for( int i=0; i<w; i++ ) {
        //noteToggles[i][i] = true;
        }
    
        
    }






void freeMusicPlayer() {

    for( int i=0; i<PARTS; i++ ) {
        for( int y=0; y<h; y++ ) {
            
            for( int x=0; x<w; x++ ) {
                
                if( noteGrid[i][y][x] != NULL ) {
                    delete noteGrid[i][y][x];
                    }
                }
            }
        }
    
    int i;
    
    // last ones are pointers to previous envelopes/timbres, don't delete
    for( i=0; i<numTimbres-1; i++ ) {
        delete musicTimbres[i];
        }
    for( i=0; i<numEnvelopes-1; i++ ) {
        delete musicEnvelopes[i];
        }


    for( i=0; i<oldTimbres.size(); i++ ) {
        delete *( oldTimbres.getElement( i ) );
        }
    for( i=0; i<oldEnvelopes.size(); i++ ) {
        delete *( oldEnvelopes.getElement( i ) );
        }

    // delete these copies
    for( i=0; i<currentlyPlayingNotes.size(); i++ ) {
        delete *( currentlyPlayingNotes.getElement( i ) );
        }
    
    
    }




void setCopiedPart( int inPartIndex ) {
    
    musicTimbres[PARTS-1] = musicTimbres[inPartIndex];
    musicEnvelopes[PARTS-1] = musicEnvelopes[inPartIndex];  

    
    partStepDurations[PARTS-1] = partStepDurations[inPartIndex];
    partStepDurationsInSamples[PARTS-1] = 
        partStepDurationsInSamples[inPartIndex];
    
    partLengths[PARTS-1] = partLengths[inPartIndex];
    
    }

