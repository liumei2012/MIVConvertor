// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#include <string>
#include <iostream> 
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Renderer.h"
#include "Mesh.h"
#include "ShaderNode.h"
#include "LoadOBJ.h"
#include "Matrix.h"
#include "FileTools.h"
#include "StringTools.h"
#include "Util.h"
#include "Camera.h"
#include "MIVCamInfo.h"
#include "ply.h"
// #define IMAGPROCESSINGTEST 

using namespace std;
using namespace gsn;
#define MAXFRAME 1

std::vector<float> Hetro_vertices[MAXFRAME];
std::vector<float> Hetro_normals[MAXFRAME];
std::vector<float> Hetro_texcoords[MAXFRAME];
std::vector<unsigned int> Hetro_indexes[MAXFRAME];

std::vector<Vertex> Tmpvertices;
std::vector<unsigned int> Tmpindexes;
std::vector<Texture> Tmptextures;



vMeshParameters params;
Sequence           eVmeshSequence;

std::vector<float> vertex_in;
std::vector<float>  normal_in;
std::vector<float>  texcoords_in;
std::vector<unsigned int>  indices_in;

#define PI 3.141592
#define TWOPI 6.283185

#define radius_in 0.5

void initBufers_in()
{
    float theta;
    float sinTheta;
    float cosTheta;
    float phi;
    float sinphi;
    float cosphi;
    float x, y, z, u, v;

    int first;
    int second;

    int normalindex = 0;
    int vertexindex = 0;
    int texcoordsindex = 0;
    int indexindex = 0;

    int latitudeBands = 30;
    int longitudeBands = 30;
    double dOffsetx = 0;
    for (int latNum = 0; latNum <= latitudeBands; latNum++)
    {
        theta = latNum * PI / latitudeBands;
        sinTheta = sin(theta);
        cosTheta = cos(theta);
        for (int longNum = 0; longNum <= longitudeBands; longNum++)
        {
            phi = longNum * 2 * PI / longitudeBands;
            sinphi = sin(phi);
            cosphi = cos(phi);

            x = cosphi * sinTheta;
            y = cosTheta;
            z = -(sinphi * sinTheta);
            u = 1.0 - (float(longNum) / float(longitudeBands));
            v = (float(latNum) / float(latitudeBands));

            normal_in.push_back(x);
            normalindex++;
            normal_in.push_back(y);
            normalindex++;
            normal_in.push_back(z);
            normalindex++;

            texcoords_in.push_back(u);
            texcoordsindex++;
            texcoords_in.push_back(v);
            texcoordsindex++;

            vertex_in.push_back(radius_in * x + dOffsetx);
            vertexindex++;
            vertex_in.push_back(radius_in * y);
            vertexindex++;
            vertex_in.push_back(radius_in * z);
            vertexindex++;
        }
    }


    for (int latNum = 0; latNum < latitudeBands; latNum++)
    {
        for (int longNum = 0; longNum < longitudeBands; longNum++)
        {
            first = (latNum * (longitudeBands + 1)) + longNum;
            second = first + longitudeBands + 1;

            indices_in.push_back(first);
            indexindex++;

            indices_in.push_back(second);
            indexindex++;

            indices_in.push_back(first+1);
            indexindex++;

            indices_in.push_back(second);
            indexindex++;

            indices_in.push_back(second+1);
            indexindex++;

            indices_in.push_back(first+1);
            indexindex++;
        }
    }
}


void initMeshHetroObj(float fModelScale)
{
    for (int j = 0; j < MAXFRAME; j++) {
        eVmeshSequence.setFrameIndex(j);
        vMeshObj* pGeometry = (vMeshObj*)(eVmeshSequence.getObject().getGeometry());
        Box b = pGeometry->m_Meshes[0].getBox();
        //Box b1 = eVmeshSequence.getObject().getBox();
        //eVmeshSequence.getObject().scale(b1, 0.5);
        Box b2 = eVmeshSequence.getBox();
        float fModelScaleFactorRedandBlack = fModelScale;
        pGeometry->m_Meshes[0].scale(b2.center(), fModelScaleFactorRedandBlack);

        Tmpvertices = pGeometry->m_Meshes[0].getVertices();
        Tmpindexes = pGeometry->m_Meshes[0].getIndices();
        Tmptextures = pGeometry->m_Meshes[0].getTextures();

        Hetro_vertices[j].clear();
        Hetro_normals[j].clear();
        Hetro_texcoords[j].clear();

        for (int i = 0; i < Tmpvertices.size(); i++)
        {
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.x);
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.y);
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.z);

            Hetro_normals[j].push_back(Tmpvertices[i].normal_.x);
            Hetro_normals[j].push_back(Tmpvertices[i].normal_.y);
            Hetro_normals[j].push_back(Tmpvertices[i].normal_.z);

            Hetro_texcoords[j].push_back(Tmpvertices[i].texCoords_.x);
            Hetro_texcoords[j].push_back(1.0 - Tmpvertices[i].texCoords_.y);
        }

        for (int i = 0; i < Tmpindexes.size(); i++)
        {
            Hetro_indexes[j].push_back(Tmpindexes[i]);
        }
    }
}

void Renderer::readSequence(vMeshParameters& params, Sequence& eVmeshSequence) {

    eVmeshSequence.readFile(params.getFile(), params.getFrameIndex(), params.getFrameNumber(), params.getBinFile());
   
    if (params.getBoxSize() > 0) {
        eVmeshSequence.setBoxSize((float)params.getBoxSize());
    }
    else {
        eVmeshSequence.setBoxSize(eVmeshSequence.getBox().getMaxSize());
    }

    // Normalize objects position and size
    eVmeshSequence.normalize(2, params.getCenter());
}

Renderer::Renderer()
{
  nCamIndex = 0;
  t = 0.0;
  windowWidth = 0;
  windowHeight = 0;
  selectedOutput = 0;
}

Renderer::~Renderer() {
}

void Renderer::initShader() {
    // Initialize shader A
    
    //shaderSettingsHetroObj = loadShaderSettings(FileTools::findFile("data/parameters_hetro.csv"));
    {
        // for regular shaders, load the input mesh
       // 
        // for regular shaders, a fragment and a vertex shader is required
        std::string v = FileTools::findFile("shaders/vertex_shader.txt");

        std::string f = FileTools::findFile("shaders/fragment_shader_Gray.txt");
        shaderNodeGray.setShaderSourceFromFile(v, f);

        //f = FileTools::findFile("shaders/fragment_shader_InputTex.txt");
        f = FileTools::findFile("shaders/fragment_shader_InputTex.txt");
        shaderNodeInputTex.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_InputTexLod.txt");
        shaderNodeInputTexMipmap.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_RowAvg.txt");
        shaderNodeRowAvg.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_ColAvg.txt");
        shaderNodeColAvg.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_PDFJoint.txt");
        shaderNodePDFJoint.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_PDFMarg.txt");
        shaderNodePDFMarg.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_PDFCond.txt");
        shaderNodePDFCond.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_CDFMarg.txt");
        shaderNodeCDFMarg.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_CDFCond.txt");
        shaderNodeCDFCond.setShaderSourceFromFile(v, f);

        f = FileTools::findFile("shaders/fragment_shader_Env.txt");
        shaderNodeEnv.setShaderSourceFromFile(v, f);

        v = FileTools::findFile("shaders/vertex_shader_IBL.txt");
        f = FileTools::findFile("shaders/fragment_shader_IBL.txt");
        shaderNodeHetroObj.setShaderSourceFromFile(v, f);
       
        v = FileTools::findFile("shaders/vertex_shader_IBL.txt");
        f = FileTools::findFile("shaders/fragment_shader_IBL.txt");
        shaderNodeHetroSphere.setShaderSourceFromFile(v, f); 
    }
}

void Renderer::Preinit()
{
    if (nProgMode == 0)
    {
        bPointCloudConversion = false;
    }
    else if (nProgMode == 1)
    {
        bPointCloudConversion = true;
    }


    for (int nView = 0; nView < nMaxCamCount; nView++)
    {
        if (bPointCloudConversion) {
            FileTools::YUVToGeoTex(strMIVSequencePath, strPostfixGeo, strGeoBitDepth, nView, SceneGeo[nView],
                shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight);
        }
        FileTools::YUVToRGBTex(strMIVSequencePath,
            strPostfixTex,
            strTexBitDepth,
            nView,
            SceneTex[nView],
            shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight, bPointCloudConversion);

    }

    for (int i = 0; i < nMaxCamCount; i++)
    {
        pCamProp[i] = new CammeraProperty(ViewID[i], ViewRotID[i]);
    }

    if (bPointCloudConversion) {
        struct PointCloudAttribute
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };

        int nFactorResolution = 4;

        for (int c = 0; c < nMaxCamCount; c++)
        {
            std::vector<pcc::point_t> pointCloud;
            std::vector<PointCloudAttribute> attrpointCloud;
            std::vector<unsigned char> attrVec;
            int nRGBIndex = 0;
            for (int i = 0; i < shaderNodeHetroObj.nHetroBGImageDimHeight; i++)
            {
                for (int j = 0; j < shaderNodeHetroObj.nHetroBGImageDimWidth; j++)
                {
                    if (i % nFactorResolution == 0 && j % nFactorResolution == 0)
                    {
                        float fOutputPoints[4] = {0.0, 0.0, 0.0, 0.0};
                        pcc::point_t vOutputPoints;
                        unsigned short pDepth;
                        float fFOV[2] = { fFieldOfView0, fFieldOfView1 };
                        float fCamPlane[2] = { fNearPlane, fFarPlane };

                        pDepth = SceneGeo[c].at(i * shaderNodeHetroObj.nHetroBGImageDimWidth + j);

                        if (pDepth > 0) {
                            pCamProp[c]->ConvertERPToWorldPoints(&pDepth,
                                j, i, fOutputPoints,
                                shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight,
                                16,
                                fFOV,
                                fCamPlane);

                            vOutputPoints[0] = fOutputPoints[0];
                            vOutputPoints[1] = fOutputPoints[1];
                            vOutputPoints[2] = fOutputPoints[2];

                            pointCloud.push_back(vOutputPoints);

                            PointCloudAttribute pcRGB;
                            pcRGB.r = SceneTex[c].at(i * shaderNodeHetroObj.nHetroBGImageDimWidth * 3 + j * 3);
                            pcRGB.g = SceneTex[c].at(i * shaderNodeHetroObj.nHetroBGImageDimWidth * 3 + j * 3 + 1);
                            pcRGB.b = SceneTex[c].at(i * shaderNodeHetroObj.nHetroBGImageDimWidth * 3 + j * 3 + 2);
                            attrpointCloud.push_back(pcRGB);
                        }
                    }
                }
            }

            pcc::PCCPointSet3 OutputPointCloud;
            OutputPointCloud.resize(pointCloud.size());
            OutputPointCloud.addColors();
            for (int i = 0; i < OutputPointCloud.getPointCount(); i++)
            {
                OutputPointCloud[i] = pointCloud[i];
                pcc::Vec3<unsigned char> tempCol;
                tempCol[0] = attrpointCloud[i].g;
                tempCol[1] = attrpointCloud[i].b;
                tempCol[2] = attrpointCloud[i].r;
                OutputPointCloud.setColor(i, tempCol);
            }
            std::string strPointCloudPostfix = "test";
            std::string strPointCLoudPath = strPointCloudPostfix + std::to_string(c) + ".ply";
            pcc::ply::PropertyNameMap propNames;
            propNames.position = { "x", "y", "z" };
            pcc::ply::write(OutputPointCloud, propNames, 1.0, 0.0, strPointCLoudPath, 1);

            pointCloud.clear();
            OutputPointCloud.clear();
            attrVec.clear();
            attrpointCloud.clear();
            //std::string TestFileName = "FileDump_";
            //TestFileName = TestFileName + std::to_string(c) + ".raw";
            //std::ofstream outFile(TestFileName, std::ios::binary);
            //outFile.write(reinterpret_cast<char*> (attrVec.data()), attrVec.size());
            //outFile.close();
        }
     
    }

    for (int i = 0; i < nMaxCamCount; i++)
    {
        SceneTex[i].clear();
        SceneGeo[i].clear();

        delete pCamProp[i];
        pCamProp[i] = NULL;
    }
    //exit(0);
}

void Renderer::init()
{

  params.m_pFile = strHeterObjPath;
  params.m_nFrameNumber = MAXFRAME;
  params.m_nFrameIndex = 0;

  //std::cout << "Read MIV sequence" << std::endl;
  if (nProgMode == 0)
  {
      bPointCloudConversion = false;
  }
  else if (nProgMode == 1)
  {
      bPointCloudConversion = true;
  }

  
  for (int nView = 0; nView < nMaxCamCount; nView++)
  {
      //if (bPointCloudConversion) {
      //    FileTools::YUVToGeoTex(strMIVSequencePath, strPostfixGeo, strGeoBitDepth, nView, SceneGeo[nView],
      //        shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight);
      //}
      FileTools::YUVToRGBTex(strMIVSequencePath, 
          strPostfixTex, 
          strTexBitDepth, 
          nView, 
          SceneTex[nView], 
          shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight, bPointCloudConversion);

  }

  for (int i = 0; i < nMaxCamCount; i++)
  {
      pCamProp[i] = new CammeraProperty(ViewID[i], ViewRotID[i]);
  }

  readSequence(params, eVmeshSequence);

  initMeshHetroObj(fObjScale);
  initBufers_in();

  eVmeshSequence.setFrameIndex(0);
  vMeshObj* pGeometry = (vMeshObj*)(eVmeshSequence.getObject().getGeometry());

  initShader();

  LoadOBJ::loadHetro(Hetro_vertices[0], Hetro_normals[0], Hetro_texcoords[0], Hetro_indexes[0], meshHetroObj);

#ifdef IMAGPROCESSINGTEST
  shaderNodeInputTex.setUniformsFromFile(FileTools::findFile("data/parameters.csv"));
  shaderNodeInputTexMipmap.setUniformsFromFile(FileTools::findFile("data/parameters_lod.csv"));
  shaderNodeGray.setUniformsFromFile(FileTools::findFile("data/parameters_gray.csv"));
  shaderNodeRowAvg.setUniformsFromFile(FileTools::findFile("data/parameters_RowAvg.csv"));
  shaderNodeColAvg.setUniformsFromFile(FileTools::findFile("data/parameters_ColAvg.csv"));
  shaderNodePDFJoint.setUniformsFromFile(FileTools::findFile("data/parameters_PDFJoint.csv"));
  shaderNodePDFMarg.setUniformsFromFile(FileTools::findFile("data/parameters_PDFMarg.csv"));
  shaderNodePDFCond.setUniformsFromFile(FileTools::findFile("data/parameters_PDFCond.csv"));
  shaderNodeCDFMarg.setUniformsFromFile(FileTools::findFile("data/parameters_CDFMarg.csv"));
  shaderNodeCDFCond.setUniformsFromFile(FileTools::findFile("data/parameters_CDFCond.csv"));
  shaderNodeEnv.setUniformsFromFile(FileTools::findFile("data/parameters_Env.csv"));
#else
  shaderNodeInputTex.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeInputTexMipmap.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeGray.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeRowAvg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeColAvg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodePDFJoint.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodePDFMarg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodePDFCond.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeCDFMarg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeCDFCond.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
  shaderNodeEnv.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"));
#endif

#ifdef IMAGPROCESSINGTEST
  //shaderNodeHetroSphere.setUniformsFromFile(FileTools::findFile("data/parameters_hetro.csv"));
  std::string fragSrc;
  fragSrc += "#version 300 es\n";
  fragSrc += "precision highp float;\n";
  fragSrc += "out vec4 outColor;\n";
  fragSrc += "in vec2 tc; // texture coordinate of the output image in range [0.0, 1.0]\n";
  fragSrc += "\n";
  fragSrc += "uniform sampler2D img; // mag_filter=\"LINEAR\" \n";
  fragSrc += "uniform float aspectX;\n";
  fragSrc += "uniform float aspectY;\n";
  fragSrc += "\n";
  fragSrc += "void main() {\n";
  fragSrc += "  vec2 tcc = (vec2(aspectX, aspectY) * (tc - vec2(0.5))) + vec2(0.5);\n";
  fragSrc += "  if(tcc.x >= 0.0 && tcc.x <= 1.0 && tcc.y >= 0.0 && tcc.y <= 1.0) {\n";
  fragSrc += "    outColor = texture(img, tcc);\n";
  fragSrc += "  } else {\n";
  fragSrc += "    discard;\n";
  fragSrc += "  }\n";
  fragSrc += "}\n";
  shaderNodeImageWindowPlane.setShaderSource("", fragSrc);

#else

  shaderNodeHetroObj.image_hetro_ = pGeometry->m_Meshes[0].getTextures()[0].data_.data();
  shaderNodeHetroObj.nHetroImageDimWidth = pGeometry->m_Meshes[0].getTextures()[0].width_;
  shaderNodeHetroObj.nHetroImageDimHeight = pGeometry->m_Meshes[0].getTextures()[0].height_;

  shaderNodeHetroObj.setUniformsFromFile(FileTools::findFile("data/parameters_hetro.csv"));
  shaderNodeHetroObj.setUniformsFromSeq("baseColorTexture");
  shaderNodeHetroObj.setUniformsFromMIVTex();
  
#endif

  meshDummyImagePlane.createQuad();
  
}

void Renderer::resize(int w, int h) {
  windowWidth = w;
  windowHeight = h;
}

void Renderer::ImageProcessingInBg() {

    shaderNodeInputTex.render(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height/*, shaderSettingsMipmap.wireframe*/, true);
    shaderNodeInputTexMipmap.render(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height/*, shaderSettingsMipmap.wireframe*/, true);

    shaderNodeGray.setUniformImage("MyTex", shaderNodeInputTex.getRenderTarget(selectedOutput));
    shaderNodeGray.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height/*, shaderSettingsMipmap.wireframe*/, true);

    shaderNodeRowAvg.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodeRowAvg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height/*, shaderSettingsMipmap.wireframe*/, true);

    shaderNodeColAvg.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodeColAvg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodePDFJoint.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodePDFJoint.setUniformImage("ColAvg", shaderNodeColAvg.getRenderTarget(selectedOutput));
    shaderNodePDFJoint.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodePDFMarg.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodePDFMarg.setUniformImage("ColAvg", shaderNodeColAvg.getRenderTarget(selectedOutput));
    shaderNodePDFMarg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodePDFCond.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodePDFCond.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodePDFCond.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodeCDFMarg.setUniformImage("PDFMarg", shaderNodePDFMarg.getRenderTarget(selectedOutput));
    shaderNodeCDFMarg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodeCDFCond.setUniformImage("PDFCon", shaderNodePDFCond.getRenderTarget(selectedOutput));
    shaderNodeCDFCond.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

    shaderNodeEnv.setUniformImage("MyTex", shaderNodeInputTex.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("CDFMarg", shaderNodeCDFMarg.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("PDFJoint", shaderNodePDFJoint.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("CDFCon", shaderNodeCDFCond.getRenderTarget(selectedOutput));
    shaderNodeEnv.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, true);

}
void Renderer::display() {

  ImageProcessingInBg(); 

#ifdef IMAGPROCESSINGTEST
  // compute aspect ratio for shaderNodeImageWindowPlane
  float aspectX = 1.0;
  float aspectY = 1.0;
  float aspectShader = float(shaderSettingsMipmap.width) / float(shaderSettingsMipmap.height);
  float aspectWindow = float(windowWidth) / float(windowHeight);
  if (aspectShader >= aspectWindow) {
    aspectY = aspectShader / aspectWindow;
  } else {
    aspectX = aspectWindow / aspectShader;
  }
  shaderNodeImageWindowPlane.setUniformFloat("aspectX", aspectX);
  shaderNodeImageWindowPlane.setUniformFloat("aspectY", aspectY);

  shaderNodeImageWindowPlane.setUniformImage("img", shaderNodeEnv.getRenderTarget(selectedOutput));
  shaderNodeImageWindowPlane.renderBgHDRImage(meshDummyImagePlane, shaderSettingsMipmap.backgroundColor, shaderSettingsMipmap.width, shaderSettingsMipmap.height, false);

#else
  Matrix projection(4, 4);
  projection.setPerspectiveNew();
  shaderNodeHetroObj.setUniformMatrix("cameraProjection", projection);

  Matrix mCamMat;
  Matrix mPlaneMeshTransform;
  Matrix mPlaneProjTransform;
  //mCamMat.e.push_back();
  int nInternalMatIndex = 0;
  for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++)
      {
          mCamMat.e[i * 4 + j] = (pCamProp[nCamIndex]->_matView[i * 4 + j]);
          mPlaneMeshTransform.e[i * 4 + j] = (pCamProp[nCamIndex]->_matPlaneModel[i * 4 + j]);
          mPlaneProjTransform.e[i * 4 + j] = (pCamProp[nCamIndex]->_matProj[i * 4 + j]);
      }
  }

  shaderNodeHetroObj.setUniformMatrix("uView", mCamMat);

  shaderNodeHetroObj.setUniformImage("envmapDiffuse", shaderNodeEnv.getRenderTarget(selectedOutput));
  shaderNodeHetroObj.setUniformImage("envmapSpecularLevel5", shaderNodeInputTexMipmap.getRenderTarget(selectedOutput));
  shaderNodeHetroObj.setUniformFloat("fFieldOfViewH", fFieldOfView0);
  shaderNodeHetroObj.setUniformFloat("fFieldOfViewV",fFieldOfView1);

  shaderNodeHetroObj.image_hetro_background =  &SceneTex[nCamIndex].at(0);

  shaderNodeHetroObj.nCurCamIndex = nCamIndex;

  strGeoOutputFile = "v" + std::to_string(nCamIndex) + strPostfixGeo + strGeoBitDepth + ".yuv";
  strTexOutputFile = "v" + std::to_string(nCamIndex) + strPostfixTex + strTexBitDepth + ".yuv";
  strEntityOutputFile = "v" + std::to_string(nCamIndex) + strPostfixEntity + ".yuv";

  shaderNodeHetroObj.renderHetro(meshHetroObj,
      shaderSettingsMipmap.backgroundColor,
      mPlaneMeshTransform,
      mPlaneProjTransform, windowWidth, windowHeight, true, strGeoOutputFile, strTexOutputFile, strEntityOutputFile);

  shaderNodeHetroObj.renderHetro(meshHetroObj, 
      shaderSettingsMipmap.backgroundColor, 
      mPlaneMeshTransform, 
      mPlaneProjTransform, windowWidth, windowHeight, false, strGeoOutputFile, strTexOutputFile, strEntityOutputFile);

  if (bAutoCapture) {
      nCamIndex++;
      if (nCamIndex == nMaxCamCount)
      {
          nCamIndex = 0;
      }
  }

#endif
}

void Renderer::dispose() {



}

Renderer::ShaderSettings Renderer::loadShaderSettings(const std::string& filename) const
{
  Renderer::ShaderSettings settings;

  std::string content = FileTools::readTextFile(filename);
  std::vector <string> lines = StringTools::split(content, "\n");
  for (int i = 0; i < int(lines.size()); i++) {
    std::string& line = lines[i];
    std::vector <string> items = StringTools::split(line, ",");
    if (items.size() >= 2) {
      string name = items[0];
      string type = StringTools::toLower(items[1]);
      if (type == "integer" && name == "width"  && items.size() >= 3) {
        settings.width = StringTools::stringToInt(items[2]);
      }
      if (type == "integer" && name == "height" && items.size() >= 3) {
        settings.height = StringTools::stringToInt(items[2]);
      }
      //if (type == "boolean" && name == "Wireframe" &&  items.size() >= 3) {
      //  settings.wireframe = StringTools::stringToBool(items[2]);
      //}
      if (type == "color" && name == "Background" && items.size() >= 6) {
        float x = StringTools::stringToFloat(items[2]);
        float y = StringTools::stringToFloat(items[3]);
        float z = StringTools::stringToFloat(items[4]);
        float w = StringTools::stringToFloat(items[5]);
        Matrix mat(x, y, z, w);
        settings.backgroundColor = mat;
      }
      if (type == "text" && name == "NodeClassName" && items.size() >= 3) {
        settings.nodeClassName = items[2];
      }
      if (type == "text" && name == "NodeName" && items.size() >= 3) {
        settings.nodeName = items[2];
      }
    }
  }

  return settings;
}