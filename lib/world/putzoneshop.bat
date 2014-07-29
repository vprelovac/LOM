@echo off

echo Moving files.
call put.bat %1

echo Adding mob file.
echo %1.mob >a
cat a mob\index >b
sort -n -d b >index
cat index >mob\index

echo Adding obj file.
echo %1.obj >a
cat a obj\index >b
sort -n b >index
cat index >obj\index

echo Adding wld file.
echo %1.wld >a
cat a wld\index >b
sort -n b >index
cat index >wld\index

echo Adding zon file.
echo %1.zon >a
cat a zon\index >b
sort -n b >index
cat index >zon\index

echo Adding shp file.
echo %1.shp >a
cat a shp\index >b
sort -n b >index
cat index >shp\index


rm a
rm b          
rm index
echo All done.





