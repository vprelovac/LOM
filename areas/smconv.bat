@echo off
s2c -n%1 %2.are

echo1 Building destination area.
cat %1.zon >%1.are
echo1 "$end~\n\n\n#ROOMS" >>%1.are
cat %1.wld >>%1.are
echo1 "$end~\n\n\n#MOBILES" >>%1.are
cat %1.mob >>%1.are
echo1 "$end~\n\n\n#OBJECTS" >>%1.are
cat %1.obj >>%1.are
echo1 "$end~\n\n\n#SHOPS" >>%1.are
cat %1.shp >>%1.are 
echo1 "$end~" >>%1.are

mv %1.are %2.%1
mv %1.* areas/
mv %1_prg.* areas/
mv *.%1 areas/
	


