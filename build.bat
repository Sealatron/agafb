call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

cl /Zi /Feagafb.exe /ID:\Workspace\Projects\mingw_dev_lib\include\ main.cpp /link /LIBPATH:D:\Workspace\Projects\mingw_dev_lib\lib\x86 SDL2main.lib SDL2.lib /SUBSYSTEM:CONSOLE
