@echo off
echo Adding mob file.
echo %1.mob >a
cat a mob\index >b
sort -n -d b >mob\index


echo Adding obj file.
echo %1.obj >a
cat a obj\index >b
sort -n -d b >obj\index


echo Adding wld file.
echo %1.wld >a
cat a wld\index >b
sort -n -d b >wld\index


echo Adding zon file.
echo %1.zon >a
cat a zon\index >b
sort -n -d b >zon\index


rm a
rm b          
echo All done.





