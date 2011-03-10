frame=0

for i in {937..1404};
do
  oldFileName=`printf frame%05d.png $i`
  newFileName=`printf frame%05d.png $frame`
  mv $oldFileName $newFileName
  let "frame += 1"
done