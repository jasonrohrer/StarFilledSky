#!/bin/bash

if [ $# -lt 1 ] ; then
	echo "Usage: $0 fileToZip"
	exit 1
fi


MyString=atablemadeofgreenstone


nextFileToZip=$1
removeFileAfterZipping=0

for(( i=${#MyString}; i>=1; i-- )) 
  do

  
  char=$(expr substr "$MyString" $i 1)
  
  
  mkdir $char
  cp $nextFileToZip $char/

  

  if [ $removeFileAfterZipping -eq 1 ]; then
	  echo "next" > $char/next.txt
	  rm $nextFileToZip
  fi

  zip -r $char.zip $char

  rm -r $char
  
  
  nextFileToZip="$char.zip"
  removeFileAfterZipping=1
done



mkdir enter
cp $nextFileToZip enter/
echo "next" > enter/next.txt

if [ $removeFileAfterZipping -eq 1 ]; then
	  rm $nextFileToZip
fi

zip -r enter.zip enter

rm -r enter