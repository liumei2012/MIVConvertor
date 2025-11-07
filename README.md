# MIVConvertor

MIV Converter is a conversion software designed to comply with the MIV Standard v2 for representing heterogeneous objects.
Its main features include heterogeneous object embedding—particularly geometric information processing of heterogeneous object models—and MIV conversion of point cloud models

## Development Environment

This software is developed in **C++17** using the **Visual Studio 2022** Windows development environment.  
It relies on **OpenGL** library functions for rendering and visualization.

---

### Required Tools

To build and run this software, make sure the following tools and libraries are installed:

- **Visual Studio**
- **OpenGL**
- **GLEW**
- **FreeGLUT**
- **GLM**
---

### Notes

- Ensure that the **OpenGL**, **GLEW**, **GLM** and **FreeGLUT** libraries are correctly linked in your Visual Studio project.  


# OpenGL Installation 

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

# Building the Project
After opening the configured `.sln` file in Visual Studio, build the project to generate the executable file.

# Project Structure
The software package includes the following key components:
- **MIVconverter** – Source code for the main application  
- **experiment** – Experimental scripts and utilities for testing and evaluation  
- **data** – Contains datasets required by the software, including original MIV image frames such as *chess*, *classroomvideo*, and *museum*.

## Dataset and 3D Model Resources

The datasets required by this software are based on the MIV standard and can be downloaded from:

- Public MIV datasets: [MPEG Content Database](https://mpeg-miv.org/index.php/content-database-2/)  
- Restricted datasets (e.g., *museum*): available with permission from the [MPEG Expert Group](https://mpegfs.int-evry.fr/)  
- 3D models (.obj) for heterogeneous objects: [Textured Mesh Repository](https://texturedmesh.isti.cnr.it/download)


The .bat files serve as example scripts for running the software, providing users with pre-configured program parameters.

## Environment Light Rendering Support

To enable environment light rendering, the software includes several `.csv` files in the `data` folder that define GLSL uniform parameters.  
These parameters support GLSL-based image processing techniques, such as importance sampling.

As part of the MIV standard, the software uses the renderer included in the MIV reference implementation to generate environment maps.  
Preprocessed environment light images are stored in the `dataset` folder, sharing filenames with the corresponding `.json` files but using the `.yuv` extension.



# Software manual

![Heterogeneous object to MIV](./ReadmeImage0.png)

![MIV to Point cloud](./ReadmeImage1.png)

# Excute batch

```
.\MIVConvertor.exe 1 ".\data\chess\\" ".\data\chess\Chess.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\Output\\"
```