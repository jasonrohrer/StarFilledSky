
#include "minorGems/graphics/converters/TGAImageConverter.h"
#include "minorGems/io/file/FileInputStream.h"
#include "minorGems/io/file/FileOutputStream.h"


int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs < 4 ) {
        printf( "Usage:  %s  file1.tga file2.tga ... fileN.tga out.tga", 
                inArgs[0] );    
        return -1;
        }
    

    char *outFileName = inArgs[ inNumArgs - 1 ];
    
    int numInputFiles = inNumArgs - 2;
    
    
    File firstInFile( NULL, inArgs[ 1 ] );
    

    FileInputStream firstIn( &firstInFile );
    
    TGAImageConverter converter;


    Image *sumImage = converter.deformatImage( &firstIn );

    int numInSum = 1;

    int w = sumImage->getWidth();
    int h = sumImage->getHeight();
    int numChannels = sumImage->getNumChannels();

    int numPixels = w * h;
    
    
    for( int i=2; i<=numInputFiles; i++ ) {
        
        File nextInFile( NULL, inArgs[ i ] );
        
        
        FileInputStream nextIn( &nextInFile );
        

        Image *nextImage = converter.deformatImage( &nextIn );
            
        if( nextImage->getWidth() != w ||
            nextImage->getHeight() != h ||
            nextImage->getNumChannels() != numChannels ) {
            printf( "Image %s doesn't match size, skipping\n",
                    inArgs[i] );
            break;
            }
        
        printf( "Adding in image %s\n", inArgs[i] );

        for( int c=0; c<numChannels; c++ ) {
            double *sumChannel = sumImage->getChannel( c );
            double *nextChannel = nextImage->getChannel( c );
            
            for( int p=0; p<numPixels; p++ ) {
                sumChannel[p] += nextChannel[p];
                }
            }
        
        delete nextImage;

        numInSum ++;        
        }


    // finish average
    double factor = 1.0 / (double)numInSum;
    
    for( int c=0; c<numChannels; c++ ) {
        double *sumChannel = sumImage->getChannel( c );
        for( int p=0; p<numPixels; p++ ) {
            sumChannel[p] *= factor;
            }
        }

    

    File outFile( NULL, outFileName );
    

    FileOutputStream out( &outFile );
    
    converter.formatImage( sumImage, &out );

    delete sumImage;
    }

    
