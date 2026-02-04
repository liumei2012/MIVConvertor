.\MIVConvertor.exe 0 ".\data\classroomvideo\\" ".\data\classroomvideo\ClassroomVideo.json" "_texture_4096x2048_yuv420p" "_depth_4096x2048_yuv420p" "_entity_4096x2048_yuv420p" "10le" "16le" ".\data\RWT121\CHASSEUR.obj" ".\Output\\" 1 ".\CompositedResults\\" data/classroomvideo/ClassroomVideo.raw 0 1


@echo off
setlocal EnableDelayedExpansion

FOR /L %%i IN (0,1,8) DO (
.\MIVConvertor.exe 0 ".\data\classroomvideo\\" ".\data\classroomvideo\ClassroomVideo.json" "_texture_4096x2048_yuv420p" "_depth_4096x2048_yuv420p" "_entity_4096x2048_yuv420p" "10le" "16le" ".\data\RWT121\CHASSEUR.obj" ".\Output\\" 1 ".\CompositedResults\\" data/classroomvideo/ClassroomVideo.raw %%i 1
)

endlocal