// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#ifndef DEF_RENDERER_H
#define DEF_RENDERER_H

#include <stdio.h>
#include <string>
#include <vector>
#include "Mesh.h"
#include "ShaderNode.h"
#include "ReaderObjectMesh.h"
#include "ReaderSequenceParameters.h"
#include "vMeshReadSequence.h"
#include <array>

namespace gsn {
  /*!
  \class Renderer Renderer.h
  \brief This class defines a renderer.
  */
    struct Camera {
        std::string name;
        std::array<double, 3> position{};
        std::array<double, 3> rotation{};
        int depthmap{};
        int background{};
        std::array<int, 2> resolution{};
        std::string projection;
        std::array<double, 2> focal{};
        std::array<double, 2> principle_point{};
        std::array<double, 2> depth_range{};
        std::array<int, 2> HorRange;
        std::array<int, 2> VerRange;
        int bit_depth_color{};
        int bit_depth_depth{};
        bool has_invalid_depth{};
        std::string color_space;
        std::string depth_color_space;
    };

  class Renderer {
  public:

    //! constructor
    Renderer();

    //! destructor
    ~Renderer();

  public:
    //! initialize all rendering resources
      void init();
      void Preinit();
    void initShader();
    void getBoundingBox(float fMax[3], float fMin[3], float fx, float fy, float fz);
    void initMeshHetroObj(float fModelScale);
    void ComposeContents(std::string strYUVPath,
        std::string strYUVOutputPath,
        std::string strPostfixTex,
        std::string strPostfixGeo,
        std::string strPostfixEntity,
        std::string strBitDepthTex,
        std::string strBitDepthGeo,
        std::string strCompositionRetOutPath,
        /*int nNoofView, */
        int nTexWidth, int nTexHeight, bool bEnableCompositionTool);
    //! resize event
    void resize(int w, int h);
    void printProgressBar(int progress, int total);
    //! draw call
    void display();
    void ImageProcessingInBg();
    //! release all rendering resources
    void dispose();

  public:
    float t;
    int selectedOutput;
    int nCamIndex;
    int nMaxCamCount = 24;
    bool bPointCloudConversion = false;
    int nProgMode = 0;

    float fMaxOfBoxMIV[3] = { -999999999.9 , -999999999.9 , -999999999.9 };
    float fMinOfBoxMIV[3] = { 999999999.9 ,999999999.9 ,999999999.9 };

  public:
    int windowWidth;
    int windowHeight;

    std::string strMIVSequencePath ;
    std::string strHeterObjPath;
    std::string strPostfixTex;
    std::string strPostfixGeo;
    std::string strPostfixEntity;
    
    std::string strTexBitDepth;
    std::string strGeoBitDepth;
    std::string strOutputPath;
    std::string strOutputType;

    std::string strGeoOutputFile;
    std::string strTexOutputFile;
    std::string strEntityOutputFile;

    float fFieldOfView0 = 3.14159;
    float fFieldOfView1 = 1.5708;
    float fNearPlane = 0.5;
    float fFarPlane = 25.0;

    //float fObjScale = 0.0017;
    float fObjScale = 1.0;

    bool bAutoCapture = false;
    bool bAutoComposition = false;
    bool bEnableEnvironmentRelighting = false;
    std::string strCompositedRetOutPath;
    std::string strPointCloudOutPath;
    
    struct ShaderSettings {
      int width = 1024;
      int height = 512;
      /*bool wireframe = false;*/
      Matrix backgroundColor; 
      //std::string nodeClassName;
      //std::string nodeName;
    };

    ShaderNode shaderNodeInputTex;
    ShaderNode shaderNodeInputTexMipmap;
   

    ShaderNode shaderNodeGray;
    ShaderNode shaderNodeRowAvg;
    ShaderNode shaderNodeColAvg;
    ShaderNode shaderNodePDFCond;
    ShaderNode shaderNodePDFJoint;
    ShaderNode shaderNodePDFMarg;
    ShaderNode shaderNodeCDFMarg;
    ShaderNode shaderNodeCDFCond;
    ShaderNode shaderNodeEnv;

    ShaderNode shaderNodeHetroObj;
    ShaderNode shaderNodeHetroSphere;


    Mesh meshDummyImagePlane;
    //Mesh meshDummyImageOutputPlane;
    Mesh meshHetroObj;
    Mesh meshSphere;

    ShaderSettings shaderSettingsBGImgProc;
    //ShaderSettings shaderSettingsHetroObj;
   
    ShaderNode shaderNodeImageWindowPlane;

    ShaderSettings loadShaderSettings(const std::string& filename) const;

   // bool loadHetro(std::vector<float>& verts, std::vector<float>& normals, std::vector<float>& texCoords, Mesh& mesh);
    void readSequence(vMeshParameters& params, Sequence& eVmeshSequence);

    std::vector<uint8_t> SceneTex[24];
    std::vector<unsigned short> SceneGeo[24];
    std::vector<Camera> cameras;

  };
}



#endif
