erase /s /q *.vcxproj
erase /s /q *.vcxproj.filters

call Premake\premake5 vs2019
PAUSE