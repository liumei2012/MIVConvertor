FOR /L %%i IN (0,1,6) DO (
.\MIVConvertor.exe 1 ".\data\Museum\\" ".\data\Museum\B.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\Output\\" %%i >> log.txt
)