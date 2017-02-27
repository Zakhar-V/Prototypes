lua-5.1.exe glLoadGen_2_0_3a/LoadGen.lua -style=pointer_c -spec=gl -version=4.5 -profile=core -extfile=extensions Load
copy .\glLoad.* ..\..\Engine\Source
md .\GL
move .\glLoad.* .\GL
pause 