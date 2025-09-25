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
            //printf("x:%f,",normal[normalindex]);
            normalindex++;
            normal_in.push_back(y);
            //normal_in[normalindex] = y;
            // printf("y:%f,",normal_in[normalindex]);
            normalindex++;
            normal_in.push_back(z);
            //normal_in[normalindex] = z;
            //printf("z:%f,",normal_in[normalindex]);
            normalindex++;
           // normal_in.push_back(1);
            //normal_in[normalindex] = 1;
            //printf("a:%f,",normal[normalindex]);
            //normalindex++;

            //texcoords_in[texcoordsindex] = u;
            texcoords_in.push_back(u);
            //printf("u:%f,",texcoords[texcoordsindex]);
            texcoordsindex++;
            texcoords_in.push_back(v);
            //texcoords_in[texcoordsindex] = v;
            //printf("v:%f\n,",texcoords[texcoordsindex]);
            texcoordsindex++;

            //vertex_in[vertexindex] = radius_in * x + dOffsetx;
            vertex_in.push_back(radius_in * x + dOffsetx);
            //printf("x:%f,",vertex[vertexindex]);
            vertexindex++;
            vertex_in.push_back(radius_in * y);
            //vertex_in[vertexindex] = radius_in * y;
            //printf("y:%f,",vertex[vertexindex]);
            vertexindex++;
            vertex_in.push_back(radius_in * z);
            //vertex_in[vertexindex] = radius_in * z;
            //printf("z:%f,",vertex[vertexindex]);
            vertexindex++;
            //vertex_in.push_back(1.0);
            ////vertex_in[vertexindex] = 1.0;
            ////printf("a:%f\n",vertex[vertexindex]);
            //vertexindex++;
        }
    }


    for (int latNum = 0; latNum < latitudeBands; latNum++)
    {
        for (int longNum = 0; longNum < longitudeBands; longNum++)
        {
            first = (latNum * (longitudeBands + 1)) + longNum;
            second = first + longitudeBands + 1;
            //printf("first:%d,second:%d\n", first, second);

            //indices_in[indexindex] = first;
            indices_in.push_back(first);
            //printf("1:%d,",indices[indexindex]);
            indexindex++;

            //indices_in[indexindex] = second;
            indices_in.push_back(second);
            //printf("2:%d,",indices[indexindex]);
            indexindex++;

            //indices_in[indexindex] = first + 1;
            indices_in.push_back(first+1);
            indexindex++;

            //indices_in[indexindex] = second;
            indices_in.push_back(second);
            indexindex++;

            //indices_in[indexindex] = second + 1;
            indices_in.push_back(second+1);
            indexindex++;

            //indices_in[indexindex] = first + 1;
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
    shaderSettingsMipmap = loadShaderSettings(FileTools::findFile("data/parameters_lod.csv"));
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

void Renderer::init() 
{
  
  //params.m_pFile = "D:\\dmlab\\MPEGDataSet\\redandblack\\redandblack_fr%04d.obj";
  params.m_pFile = "data\\RWT144\\cabbage_model.obj";
  params.m_nFrameNumber = MAXFRAME;
  params.m_nFrameIndex = 0;

  std::cout << "Read MIV sequence" << std::endl;
  for (int nView = 0; nView < nMaxCamCount; nView++)
  {
      FileTools::YUVToRGBTex(strMIVSequencePath, "_100_102_frames_texture_2048x2048_yuv420p", "10le", nView, SceneTex[nView]);

  }

  readSequence(params, eVmeshSequence);
  initMeshHetroObj(0.17);
  initBufers_in();

  eVmeshSequence.setFrameIndex(0);
  vMeshObj* pGeometry = (vMeshObj*)(eVmeshSequence.getObject().getGeometry());

  initShader();

  //LoadOBJ::load(FileTools::findFile("data/Mesh.obj"), meshHetroObj);
  LoadOBJ::loadHetro(Hetro_vertices[0], Hetro_normals[0], Hetro_texcoords[0], Hetro_indexes[0], meshHetroObj);
    //LoadOBJ::loadHetro(vertex_in, normal_in, texcoords_in, indices_in, meshSphere);



  for (int i = 0; i < nMaxCamCount; i++)
  {
      pCamProp[i] = new CammeraProperty(ViewID[i], ViewRotID[i]);
  }
 

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

  shaderNodeHetroObj.image_hetro_ = pGeometry->m_Meshes[0].getTextures()[0].data_.data();

  shaderNodeHetroObj.setUniformsFromFile(FileTools::findFile("data/parameters_hetro.csv"));
  shaderNodeHetroObj.setUniformsFromSeq("baseColorTexture");
  shaderNodeHetroObj.setUniformsFromMIVTex();
  
  //shaderNodeHetroSphere.setUniformsFromFile(FileTools::findFile("data/parameters_hetro.csv"));

  meshDummyImagePlane.createQuad();
  
}

void Renderer::resize(int w, int h) {
  windowWidth = w;
  windowHeight = h;
}

void Renderer::display() {

  // You can manually overwrite the uniform variables, e.g.,
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

  shaderNodeHetroObj.image_hetro_background =  &SceneTex[nCamIndex].at(0);

  shaderNodeHetroObj.renderHetro(meshHetroObj,
      shaderSettingsMipmap.backgroundColor,
      mPlaneMeshTransform,
      mPlaneProjTransform, windowWidth, windowHeight, true);

  //shaderNodeHetroObj.renderHetro(meshHetroObj, 
  //    shaderSettingsMipmap.backgroundColor, 
  //    mPlaneMeshTransform, 
  //    mPlaneProjTransform, windowWidth, windowHeight, false);

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