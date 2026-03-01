.\MIVConvertor.exe 0 ".\data\chess\\" ".\data\chess\Chess.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\data\RWT121\CHASSEUR.obj" ".\Output\\" 1 ".\CompositedResults\\" data/chess/Chess.raw 0 1

@echo off
setlocal EnableDelayedExpansion

FOR /L %%i IN (0,1,8) DO (
    rem set /A frame=1450+%%i

    .\MIVConvertor.exe 0 ".\data\chess\\" ".\data\chess\Chess.json" ^
    "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" ^
    "10le" "16le" ".\data\RWT121\CHASSEUR.obj" ^
    ".\Output\\" 1 ".\CompositedResults\\" data/chess/Chess.raw %%i 1 
)

endlocal