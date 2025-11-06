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

C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\gl

### 2. DLL and Lib File Path

C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64

If necessary, copy the required .dll files to the following folder:

C:\Windows\System32

---

# Step 2 Project build
After opening the configured .sln file in Visual Studio and building the project, an executable file will be generated.

# Step 3 Run batch