call "E:\Code\Scripts\shell.bat"

set libPath="..\..\resources\SDL2-2.0.5\lib\x86"
set incPath="..\..\resources\SDL2-2.0.5\include"

cl /Zi /Feagafb.exe /I%incPath% main.cpp /link /LIBPATH:%libPath% SDL2main.lib SDL2.lib /SUBSYSTEM:CONSOLE
