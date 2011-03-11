mkdir $4

frame=$3

i=$1
while [[ $i -le $2 ]] ;
do
  oldFileName=`printf frame%05d.png $i`
  newFileName=`printf frame%05d.png $frame`
  cp $oldFileName $4/$newFileName
  let "frame += 1"
  let "i += 1"
done