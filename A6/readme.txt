Group Members:

Member 1:
Name: Astha Singh
Student ID: 110092930

Member 2:
Name: Aryan Aryan
Student ID: 110101715

Compile : gcc decode_video.c -o decode_video $(pkg-config --cflags --libs libavcodec libavformat libavutil libswscale gtk4)
Run : ./decode_video input.mp4 610 0.299 0.587 0.11 