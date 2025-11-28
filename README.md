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
- **[OpenGL](https://www.opengl.org/)**
- **[GLEW](http://glew.sourceforge.net/)**
- **[FreeGLUT](http://freeglut.sourceforge.net/)**
- **[GLM](https://github.com/g-truc/glm)**
---

### Notes

- Ensure that the **OpenGL**, **GLEW**, **GLM** and **FreeGLUT** libraries are correctly linked in your Visual Studio project.  


# OpenGL Installation 

This guide explains how to install **OpenGL** along with the **FreeGLUT** and **GLEW** libraries required for the project.

We have included the OpenGL library we are using in glinstall folder.  
Alternatively, you can install OpenGL manually.


## Installation Steps

## File Paths

This section provides the default file paths for the OpenGL libraries required for the project.

---

### 1. Header File Path

C:\Program Files (x86)\Windows Kits\10\Include\xx.x.xxxxx.x\um\gl

### 2. DLL and Lib File Path

C:\Program Files (x86)\Windows Kits\10\Lib\xx.x.xxxxx.x\um\x64

The locations of the .dll files required for OpenGL installation are shown in the image. They can be placed in a local folder or copied to the system folder. 

**C:\Windows\System32**

![dll path0](./dllpath.png)

**.\Experiment\**

![dll path1](./dllpath2.png)


---

# Building the Project
After opening the configured `.sln` file in Visual Studio, build the project to generate the executable file.

# Project Structure
The software package includes the following key components:
- **MIVconverter** – Source code for the main application  
- **experiment** – Experimental scripts and utilities for testing and evaluation  
- **data** – Contains the datasets required by the software, including original MIV image frames such as chess, classroomvideo, and museum. In addition to the data, the folder also includes the GLSL global parameter files imageProcessingUniform.csv and parameters_hetro.csv, as well as brdfIntegrationMap.pfm, for ambient-light image processing and object–ambient-light interaction. 

![Heterogeneous object to MIV](./ExperimentFolder.png)

## Dataset and 3D Model Resources

The datasets required by this software are based on the MIV standard and can be downloaded from:

- Public MIV datasets: [MPEG Content Database](https://mpeg-miv.org/index.php/content-database-2/)  
- Restricted datasets (e.g., *museum*, *loot*, *redandblack*, *longdress*): available with permission from the [MPEG Expert Group](https://mpegfs.int-evry.fr/)  
- 3D models (.obj) for heterogeneous objects: [Textured Mesh Repository](https://texturedmesh.isti.cnr.it/download)


## Environment Light Rendering Support (Optional)

To enable environment light rendering, the software includes two .csv files in the data folder that define GLSL uniform parameters, and it implements a set of GLSL shaders based on https://www.gsn-lib.org/, which are available in the shader folder. These parameters and shader programs support GLSL image processing techniques such as importance sampling and the interaction between environment map and heterogeneous objects.
As part of the MIV standard, the software uses the renderer included in the MIV reference implementation to generate environment maps.  
Preprocessed environment light images are stored in the `dataset` folder, sharing filenames with the corresponding `.json` files but using the `.yuv` extension.


## Software manual

### Heterogeneous object to MIV
![Heterogeneous object to MIV](./ReadmeImage0.png)

After configuring the experimental files, run the program. The program operates in two modes. When the mode code is set to zero, a simple OpenGL rendering window is launched, displaying the MIV image rendered by the current camera along with the foreign object. The program provides basic keyboard controls that allow the user to move the foreign object within the MIV space. Once the object is positioned satisfactorily, it can be automatically captured by pressing the 'c' key. During automatic capture, the OpenGL window will automatically switch between cameras to record images of the object from multiple angles, collecting both texture and geometric information. Each automatic capture cycle corresponds to one frame, and the program will close automatically upon completion.
If the user wishes to composite the captured image with the original frame, the auto composition parameter can be set to 1 in the execution parameters. The composited MIV data will then be saved in the compositionResult folder.


### MIV to Point cloud
![MIV to Point cloud](./ReadmeImage1.png)

With the widespread use of point cloud data, this program also provides MIV transformation functionality for point cloud datasets. Setting the mode code to 1 in the parameters enables the program to read the original MIV data and perform point cloud reconstruction. Unlike mode code 0, this mode does not launch an OpenGL window for scene rendering or composition. Therefore, only the output path (the Output folder) needs to be specified in the parameters, and the composited results folder (CompositedResults folder) does not need to be defined.


## Example
The .bat files serve as example scripts for running the software, providing users with pre-configured program parameters. Users can easily run the program’s default functions by simply double-clicking the executable .bat file in the local folder.

### Heterogeneous object to MIV example
```
.\MIVConvertor.exe 0 ".\data\classroomvideo\\" ".\data\classroomvideo\ClassroomVideo.json" "_texture_4096x2048_yuv420p" "_depth_4096x2048_yuv420p" "_entity_4096x2048_yuv420p" "10le" "16le" ".\data\RWT121\CHASSEUR.obj" ".\Output\\" 0 ".\CompositedResults\\"
```

### MIV to Point cloud example
```
.\MIVConvertor.exe 1 ".\data\chess\\" ".\data\chess\Chess.json" "_texture_2048x2048_yuv420p" "_depth_2048x2048_yuv420p" "_entity_2048x2048_yuv420p" "10le" "16le" ".\Output\\"
```

## Evaluation

Ensure the following prerequisites are installed:

- A C++17 or newer compiler toolchain

- Git

- Internet connection

- Python 3.10 or newer

In Visual Studio, clone the repository:
https://gitlab.com/mpeg-i-visual/tmiv.git

Open the x64 Native Tools Command Prompt.

Navigate to the directory where the repository was cloned.

### Set up a Python virtual environment and install the required dependencies:
```
python -m venv venv
venv\Scripts\activate    # On Windows
python -m pip install --upgrade pip
pip install -r requirements.txt
```

### Install external dependencies, build the project, and run unit tests:
```
python scripts/install.py cl-release
```

Reference:
https://gitlab.com/mpeg-i-visual/tmiv


### Create the in folder and add the data
After cloning, the repository directory is named tmiv.
Inside the tmiv directory, create a new folder named in.
If you are using the museum sequence, create a subfolder named B01 inside the in directory:
```
.\tmiv\in\B01

```

### Add the dataset to .\tmiv\in\B01
Place the dataset files inside the .\tmiv\in\B01 directory.
Ensure that the dataset filenames follow the required naming convention, such as:

v0_depth_2048x2048_yuv420p16le.yuv

v0_texture_2048x2048_yuv420p10le.yuv

Reference:
https://gitlab.com/mpeg-i-visual/tmiv/-/blob/master/doc/running.md

### TMIV encoder configuring
```
.\out\install\cl-release\bin\TmivEncoder.exe -n 1 -s B01 -f 0 -c .\config\ctc\miv_main_anchor\A_1_TMIV_encode.json -p configDirectory .\config -p inputDirectory .\in -p outputDirectory .\out
```
### Example VVenC Encoder Configuration for Rate Point 1 (RP1)
```
.\out\install\cl-release\bin\vvencFFapp.exe -c .\config\ctc\miv_main_anchor\A_2_VVenC_encode_tex.cfg -i .\out\A1\B01\RP0\TMIV_A1_B01_RP0_tex_c00_2048x4352_yuv420p10le.yuv -b .\out\A1\B01\RP1\TMIV_A1_B01_RP1_tex_c00.bit -s 2048x4352 -q 29 -f 1 -fr 30
```
```
.\out\install\cl-release\bin\vvencFFapp.exe -c .\config\ctc\miv_main_anchor\A_2_VVenC_encode_tex.cfg -i .\out\A1\B01\RP0\TMIV_A1_B01_RP0_tex_c01_2048x4352_yuv420p10le.yuv -b .\out\A1\B01\RP1\TMIV_A1_B01_RP1_tex_c01.bit -s 2048x4352 -q 29 -f 1 -fr 30
```
```
.\out\install\cl-release\bin\vvencFFapp.exe -c .\config\ctc\miv_main_anchor\A_2_VVenC_encode_geo.cfg -i .\out\A1\B01\RP0\TMIV_A1_B01_RP0_geo_c00_1024x2176_yuv420p10le.yuv -b .\out\A1\B01\RP1\TMIV_A1_B01_RP1_geo_c00.bit -s 1024x2176 -q 9 -f 1 -fr 30
```
```
.\out\install\cl-release\bin\vvencFFapp.exe -c .\config\ctc\miv_main_anchor\A_2_VVenC_encode_geo.cfg -i .\out\A1\B01\RP0\TMIV_A1_B01_RP0_geo_c01_1024x2176_yuv420p10le.yuv -b .\out\A1\B01\RP1\TMIV_A1_B01_RP1_geo_c01.bit -s 1024x2176 -q 9 -f 1 -fr 30
```

### Multiplexer Configuration
```
.\out\install\cl-release\bin\TmivMultiplexer.exe -n 1 -s B01 -r RP1 -c .\config\ctc\miv_main_anchor\A_3_TMIV_mux.json -p configDirectory .\config -p inputDirectory .\out -p outputDirectory .\out
```
### TMIV decoding

```
.\out\install\cl-release\bin\TmivDecoder.exe -n 1 -N 1 -s B01 -r RP1 -v v11 -c .\config\ctc\miv_main_anchor\A_4_TMIV_decode.json -p configDirectory .\config -p inputDirectory .\out -p outputDirectory .\out
```