#include <stdio.h>


int main() {
    
    double x = (double)1 / (double)3;
    
    double yD = x * 3;

    FILE *outFile = fopen( "testDouble.txt", "w" );
    
    fprintf( outFile, 
             "x = %f, (yD = x*3) = %f,  (int)yD = %d, (int)( x * 3 ) = %d\n", 
             x, yD, (int)yD, (int)( x * 3 ) );
    fclose( outFile );

    return 0;
    }
