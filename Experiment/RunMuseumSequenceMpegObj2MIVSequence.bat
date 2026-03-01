.\MIVConvertor.exe 0 ".\data\Museum\\" ".\data\Museum\B.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\data\soldier\soldier_fr0539.obj" ".\Output\\" 1 ".\CompositedResults\\" data/Museum/B.raw 0 1

@echo off
setlocal EnableDelayedExpansion

FOR /L %%i IN (0,1,8) DO (
    rem set /A frame=1450+%%i

    .\MIVConvertor.exe 0 ".\data\Museum\\" ".\data\Museum\B.json" ^
    "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" ^
    "10le" "16le" ".\data\soldier\soldier_fr%%04d.obj" ^
    ".\Output\\" 1 ".\CompositedResults\\" data/Museum/B.raw %%i 8 539 1 >> log.txt
)

endlocal