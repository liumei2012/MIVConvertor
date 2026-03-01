FOR /L %%i IN (0,1,6) DO (
.\MIVConvertor.exe 1 ".\data\classroomvideo\\" ".\data\classroomvideo\ClassroomVideo.json" "_texture_4096x2048_yuv420p" "_depth_4096x2048_yuv420p" "_entity_4096x2048_yuv420p" "10le" "16le" ".\Output\\" %%i
)