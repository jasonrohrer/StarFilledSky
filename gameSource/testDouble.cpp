#include <stdio.h>
#include <math.h>

int main() {
    
    double x = (double)1 / (double)3;
    
    double yD = x * 3;

    FILE *outFile = fopen( "testDouble.txt", "w" );
    
    fprintf( outFile, 
             "x = %f \n(yD = x*3) = %f \n(int)yD = %d \n(int)( x * 3 ) = %d\n"
             "(int)( round( x*3 ) ) = %d\n", 
             x, yD, (int)yD, (int)( x * 3 ), (int)( round( x*3 ) ) );
    fclose( outFile );

    return 0;
    }
