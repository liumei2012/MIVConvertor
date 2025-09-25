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

namespace gsn {
  /*!
  \class Renderer Renderer.h
  \brief This class defines a renderer.
  */
    
  class Renderer {
  public:

    //! constructor
    Renderer();

    //! destructor
    ~Renderer();

  public:
    //! initialize all rendering resources
    void init();
    void initShader();
    //! resize event
    void resize(int w, int h);

    //! draw call
    void display();

    //! release all rendering resources
    void dispose();

  public:
    float t;
    int selectedOutput;
    int nCamIndex;
    int nMaxCamCount = 24;
  public:
    int windowWidth;
    int windowHeight;

    std::string strMIVSequencePath ;
    std::string strJsonPath;
    std::string strOutputPath;
    std::string strOutputType;
    std::string strHeterObjPath;

    struct ShaderSettings {
      int width = 512;
      int height = 512;
      /*bool wireframe = false;*/
      Matrix backgroundColor; 
      std::string nodeClassName;
      std::string nodeName;
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

    ShaderSettings shaderSettingsMipmap;
    ShaderSettings shaderSettingsHetroObj;
   
    ShaderNode shaderNodeImageWindowPlane;

    ShaderSettings loadShaderSettings(const std::string& filename) const;

   // bool loadHetro(std::vector<float>& verts, std::vector<float>& normals, std::vector<float>& texCoords, Mesh& mesh);
    void readSequence(vMeshParameters& params, Sequence& eVmeshSequence);
    //uint8_t* image_hetro = NULL;
    std::vector<uint8_t> SceneTex[24];
  };
}
#endif
