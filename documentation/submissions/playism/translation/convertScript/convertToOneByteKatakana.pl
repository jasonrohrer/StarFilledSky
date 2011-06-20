#!/usr/bin/perl -w 

use lib '.';
use ShiftJIS::String;


my $numArgs = $#ARGV + 1;

if( $numArgs != 2 ) {
    usage();
    } 


$fileAText = readFile( $ARGV[0] );


$fileBText = ShiftJIS::String::kataZ2H( $fileAText );


writeFile( $ARGV[1], $fileBText );






##
# Writes a string to a file.
#
# @param0 the name of the file.
# @param1 the string to print.
#
# Example:
# writeFile( "myFile.txt", "the new contents of this file" );
##
sub writeFile {
    my $fileName = $_[0];
    my $stringToPrint = $_[1];
    
    open( FILE, ">$fileName" ) or die;
    flock( FILE, 2 ) or die;

    print FILE $stringToPrint;
        
    close FILE;
}



##
# Reads file as a string.
#
# @param0 the name of the file.
#
# @return the file contents as a string.
#
# Example:
# my $value = readFile( "myFile.txt" );
##
sub readFile {
    my $fileName = $_[0];
    open( FILE, "$fileName" ) or die;
    flock( FILE, 1 ) or die;

    # read the entire file, set the <> separator to nothing
    local $/;

    my $value = <FILE>;
    close FILE;

    return $value;
}




sub usage {
    print "\nUsage:\n";
    print "  convertToOneByteKatakana.pl source_file  destination_file\n";
    print "Example:\n";
    print "  convertToOneByteKatakana.pl Japanese.txt  Japanese_2.txt\n\n";
	exit;
    }
