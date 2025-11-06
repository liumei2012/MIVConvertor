# MIVConvertor

MIV Converter is a conversion software designed to comply with the MIV Standard v2 for representing heterogeneous objects.
Its main features include heterogeneous object embedding—particularly geometric information processing of heterogeneous object models—and MIV conversion of point cloud models

## Development Environment

This software is developed in **C++17** using the **Visual Studio 2022** Windows development environment.  
It relies on **OpenGL** library functions for rendering and visualization.

---

### Required Tools

To build and run this software, make sure the following tools and libraries are installed:

- **Visual Studio 2022**
- **OpenGL**
- **GLEW**
- **FreeGLUT**
- **GLM**
---

### Notes

- Ensure that the **OpenGL**, **GLEW**, **GLM** and **FreeGLUT** libraries are correctly linked in your Visual Studio project.  


# Step 1 OpenGL Installation 

This guide explains how to install **OpenGL** along with the **FreeGLUT** and **GLEW** libraries required for the project.

We have included the OpenGL library we are using in glinstall folder.  
Alternatively, you can install OpenGL manually using the link provided below.

https://www.opengl.org/

http://freeglut.sourceforge.net/

http://glew.sourceforge.net/

https://github.com/g-truc/glm

## Installation Steps

## File Paths

This section provides the default file paths for the OpenGL libraries required for the project.

---

### 1. Header File Path

C:\Program Files (x86)\Windows Kits\10\Include\xx.x.xxxxx.x\um\gl

### 2. DLL and Lib File Path

C:\Program Files (x86)\Windows Kits\10\Lib\xx.x.xxxxx.x\um\x64

If necessary, copy the required .dll files to the following folder:

C:\Windows\System32

---

# Step 2 Project build
After opening the configured .sln file in Visual Studio and building the project, an executable file will be generated.

# Step 3 Project file organization
In addition to the source code folder MIVconverter, this software includes an experiment folder for user convenience.

The data folder contains the datasets required by the software, such as the original MIV image frames chest, classroomvideo, and museum. The original data is publicly available according to the MIV standard and can be downloaded from the link below.

https://mpeg-miv.org/index.php/content-database-2/

Unreleased datasets, such as museum, require permission from the MPEG expert group and can also be obtained from the link provided.

https://mpegfs.int-evry.fr/

3D model (.obj) files representing heterogeneous objects are available from the following link.

https://texturedmesh.isti.cnr.it/download

The .bat files serve as example scripts for running the software, providing users with pre-configured program parameters.

To enable environment light rendering, the software includes several CSV files in the data folder that support GLSL uniform parameters, allowing for efficient GLSL-based image processing such as importance sampling.

As this functionality is part of the MIV standard, the software requires the renderer included in the MIV reference software to generate environment map when needed.

To further facilitate environment light rendering, we have preprocessed some environment light images and stored them in the dataset folder. The filenames are identical to the .json files provided in the original data, but with the .yuv extension.


# Step 4 Run prepared batch


---

```
.\MIVConvertor.exe 1 ".\data\chess\\" ".\data\chess\Chess.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\Output\\"
```