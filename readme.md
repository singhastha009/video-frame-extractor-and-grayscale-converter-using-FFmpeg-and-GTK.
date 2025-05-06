# 🎞️ Video Frame Extractor & Grayscale Converter – FFmpeg + GTK (C)

A C application that demonstrates how to extract a specific frame from a video using **FFmpeg**, convert it to a **color image (PPM)** and a **grayscale image (PGM)** using custom RGB weights, and display both images side-by-side using **GTK 4**.

---

## 🧠 Features

- Extracts a frame at a given index from any supported video format
- Converts the frame into:
  - A raw RGB color image (`.ppm`)
  - A grayscale image (`.pgm`) using custom R, G, B weights
- Displays images in separate GTK windows using `GtkImage`

---

## 🛠 Technologies Used

- **Language:** C  
- **Multimedia Library:** FFmpeg (`libavcodec`, `libavformat`, `libswscale`)  
- **GUI Toolkit:** GTK 4  
- **Image Formats:** PPM (P6), PGM (P5)

---

Compile - gcc decode_video.c -o decode_video $(pkg-config --cflags --libs libavcodec libavformat libavutil libswscale gtk4)
Run : ./decode_video input.mp4 610 0.299 0.587 0.11 

