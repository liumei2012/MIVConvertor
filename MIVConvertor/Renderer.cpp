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

//#define IMAGPROCESSINGTEST 

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


void Renderer:: initMeshHetroObj(float fModelScale)
{
    float fMaxOfBox[3] = { -999999999.9, -999999999.9, -999999999.9 };
    float fMinOfBox[3] = { 999999999.9,999999999.9,999999999.9 };

    float fViewBoxMin[3] = { -0.314663142, -0.830880880, -0.198152751 };
    float fViewBoxMax[3] = { 0.314663142, 0.830880880, 0.198152751 };

    float fMaxValueAxis = -99999999.9;

    for (int j = 0; j < MAXFRAME; j++) {
        eVmeshSequence.setFrameIndex(j);
        vMeshObj* pGeometry = (vMeshObj*)(eVmeshSequence.getObject().getGeometry());
        Box b = pGeometry->m_Meshes[0].getBox();
        // = eVmeshSequence.getObject().getBox();
        //eVmeshSequence.getObject().scale(b1, 0.5);
        Box b2 = eVmeshSequence.getBox();
        float fModelScaleFactorRedandBlack = fModelScale;
        pGeometry->m_Meshes[0].scale(b2.center(), fModelScaleFactorRedandBlack);
        Vec3 vCenter = b2.center();
        Tmpvertices = pGeometry->m_Meshes[0].getVertices();
        Tmpindexes = pGeometry->m_Meshes[0].getIndices();
        Tmptextures = pGeometry->m_Meshes[0].getTextures();

        //Box b1 = pGeometry->m_Meshes[0].getBox();

        Hetro_vertices[j].clear();
        Hetro_normals[j].clear();
        Hetro_texcoords[j].clear();

        for (int i = 0; i < Tmpvertices.size(); i++)
        {
            getBoundingBox(fMaxOfBox, fMinOfBox, Tmpvertices[i].position_.x, Tmpvertices[i].position_.y, Tmpvertices[i].position_.z);
        }

        int nAxis = 0;
        for (int k = 0; k < 3; k++)
        {
            if (fMaxValueAxis <= fViewBoxMax[k])
            {
                fMaxValueAxis = fViewBoxMax[k];
                nAxis = k;
            }
        }
        
        fModelScale = fMaxValueAxis/fMaxOfBox[nAxis]  ;

        Tmpvertices.clear();
        Tmpindexes.clear();
        Tmptextures.clear();

        b2 = eVmeshSequence.getBox();
        fModelScaleFactorRedandBlack = fModelScale;
        pGeometry->m_Meshes[0].scale(b2.center(), fModelScaleFactorRedandBlack);
        vCenter = b2.center();

        Tmpvertices = pGeometry->m_Meshes[0].getVertices();
        Tmpindexes = pGeometry->m_Meshes[0].getIndices();
        Tmptextures = pGeometry->m_Meshes[0].getTextures();

        fMaxOfBox[0] = -999999999.9;
        fMaxOfBox[1] = -999999999.9;
        fMaxOfBox[2] = -999999999.9;

        fMinOfBox[0] = 999999999.9;
        fMinOfBox[1] = 999999999.9;
        fMinOfBox[2] = 999999999.9;

        for (int i = 0; i < Tmpvertices.size(); i++)
        {
            getBoundingBox(fMaxOfBox, fMinOfBox, Tmpvertices[i].position_.x, Tmpvertices[i].position_.y, Tmpvertices[i].position_.z);
        }

        for (int i = 0; i < Tmpvertices.size(); i++)
        {
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.x - fMinOfBox[0]);
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.y - fMinOfBox[1] /*+ fMinOfBoxMIV[1]*/);
            Hetro_vertices[j].push_back(Tmpvertices[i].position_.z - fMinOfBox[2]);

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

void Renderer::ComposeContents(std::string strYUVPath,
    std::string strYUVOutputPath,
    std::string strPostfixTex,
    std::string strPostfixGeo,
    std::string strPostfixEntity,
    std::string strBitDepthTex,
    std::string strBitDepthGeo, 
    std::string strCompositionRetOutPath,
   /* int nNoofView,*/
    int nTexWidth, int nTexHeight, bool bEnableCompositionTool)
{

    if (!bEnableCompositionTool)
    {
        return;
    }

    std::cout << "[INFO] Contents composition start" << std::endl;

    for (int nCam = 0; nCam <= nMaxCamCount; nCam++)
    {
        printProgressBar(nCam, nMaxCamCount);
        std::string strCompositionRetTexPath = "";
        std::string strCompositionRetGeoPath = "";
        std::string strCompositionRetEntityPath = "";

        bool bEntityActivation = false;
        int nImageWidth = nTexWidth;
        int nImageHeight = nTexHeight;

        char* buffer = NULL;

        //unsigned short* pShortGeoBufSource = NULL;
        //unsigned short* pShortTexBufSource = NULL;
        //unsigned short* pShortGeoBufTarget = NULL;
        //unsigned short* pShortTexBufTarget = NULL;

        std::vector<unsigned short> TempShortBufVec0;
        std::vector<unsigned short> TempShortBufVec1;
        std::vector<unsigned short> TempShortBufVec2;
        std::vector<unsigned short> TempShortBufVec3;
        std::vector<unsigned short> TempShortBufVec4; // entity extention
        std::vector<unsigned short> TempShortBufVec5; // entity extention

        std::vector<unsigned short> usYPlaneVecGeoSource;
        std::vector<unsigned short> usYPlaneVecTexSource;

        std::vector<unsigned short> usUVPlaneVecGeoSource[2];
        std::vector<unsigned short> usUVPlaneVecTexSource[2];

        std::vector<unsigned short> usYPlaneVecGeoTarget;
        std::vector<unsigned short> usYPlaneVecTexTarget;

        std::vector<unsigned short> usUVPlaneVecGeoTarget[2];
        std::vector<unsigned short> usUVPlaneVecTexTarget[2];

        std::vector<unsigned short> usYPlaneVecGeoOut;
        std::vector<unsigned short> usYPlaneVecTexOut;

        std::vector<unsigned short> usUVPlaneVecGeoOut[2];
        std::vector<unsigned short> usUVPlaneVecTexOut[2];

        TempShortBufVec0.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);
        TempShortBufVec1.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);
        TempShortBufVec2.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);
        TempShortBufVec3.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);

        usYPlaneVecGeoSource.resize(nTexWidth * nTexHeight);
        usUVPlaneVecGeoSource[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecGeoSource[1].resize(nTexWidth / 2 * nTexHeight / 2);

        usYPlaneVecGeoTarget.resize(nTexWidth * nTexHeight);
        usUVPlaneVecGeoTarget[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecGeoTarget[1].resize(nTexWidth / 2 * nTexHeight / 2);

        usYPlaneVecTexSource.resize(nTexWidth * nTexHeight);
        usUVPlaneVecTexSource[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecTexSource[1].resize(nTexWidth / 2 * nTexHeight / 2);

        usYPlaneVecTexTarget.resize(nTexWidth * nTexHeight);
        usUVPlaneVecTexTarget[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecTexTarget[1].resize(nTexWidth / 2 * nTexHeight / 2);


        usYPlaneVecGeoOut.resize(nTexWidth * nTexHeight);
        usUVPlaneVecGeoOut[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecGeoOut[1].resize(nTexWidth / 2 * nTexHeight / 2);

        usYPlaneVecTexOut.resize(nTexWidth * nTexHeight);
        usUVPlaneVecTexOut[0].resize(nTexWidth / 2 * nTexHeight / 2);
        usUVPlaneVecTexOut[1].resize(nTexWidth / 2 * nTexHeight / 2);

        std::string strFilenameTexTarget = strYUVPath + "v" + std::to_string(nCam) + strPostfixTex + strBitDepthTex + ".yuv";
        std::string strFilenameGeoTarget = strYUVPath + "v" + std::to_string(nCam) + strPostfixGeo + strBitDepthGeo + ".yuv";
        std::string strFilenameEntityTarget = strYUVPath + "v" + std::to_string(nCam) + strPostfixEntity + strBitDepthGeo + ".yuv";

        std::string strFilenameTexComposited =  "v" + std::to_string(nCam) + strPostfixTex + strBitDepthTex + ".yuv";
        std::string strFilenameGeoComposited =  "v" + std::to_string(nCam) + strPostfixGeo + strBitDepthGeo + ".yuv";
        std::string strFilenameEntityComposited = "v" + std::to_string(nCam) + strPostfixEntity + ".yuv";

        std::string strFilenameTexSource = strYUVOutputPath + "v" + std::to_string(nCam) + strPostfixTex + strBitDepthTex + ".yuv";
        std::string strFilenameGeoSource = strYUVOutputPath + "v" + std::to_string(nCam) + strPostfixGeo + strBitDepthGeo + ".yuv";
        std::string strFilenameEntitySource = strYUVOutputPath + "v" + std::to_string(nCam) + strPostfixEntity + strBitDepthGeo + ".yuv";

        int nFileSize = FileTools::ReadYUV(strFilenameTexSource, buffer, 0, nTexWidth, nTexHeight);
        memcpy(&TempShortBufVec0[0], buffer, nFileSize);
        delete buffer;
        buffer = NULL;

        nFileSize = FileTools::ReadYUV(strFilenameGeoSource, buffer, 0, nTexWidth, nTexHeight);
        memcpy(&TempShortBufVec1[0], buffer, nFileSize);
        delete buffer;
        buffer = NULL;

        nFileSize = FileTools::ReadYUV(strFilenameTexTarget, buffer, 0, nTexWidth, nTexHeight);
        memcpy(&TempShortBufVec2[0], buffer, nFileSize);
        delete buffer;
        buffer = NULL;

        nFileSize = FileTools::ReadYUV(strFilenameGeoTarget, buffer, 0, nTexWidth, nTexHeight);
        memcpy(&TempShortBufVec3[0], buffer, nFileSize);
        delete buffer;
        buffer = NULL;

        nFileSize = FileTools::ReadYUV(strFilenameEntityTarget, buffer, 0, nTexWidth, nTexHeight);
        if (nFileSize > 0) {
            //memcpy(&TempShortBufVec3[0], buffer, nFileSize);
            bEntityActivation = true;
            delete buffer;
            buffer = NULL;
        }


        nFileSize = FileTools::ReadYUV(strFilenameEntityTarget, buffer, 0, nTexWidth, nTexHeight);
        if (nFileSize > 0) {
            //memcpy(&TempShortBufVec3[0], buffer, nFileSize);
            bEntityActivation = true;
            delete buffer;
            buffer = NULL;
        }

        for (int i = 0; i < nTexHeight; i++)
        {
            for (int j = 0; j < nTexWidth; j++)
            {
                usYPlaneVecTexSource[i * nTexWidth + j] = TempShortBufVec0[nTexWidth * i + j];
                usYPlaneVecGeoSource[i * nTexWidth + j] = TempShortBufVec1[nTexWidth * i + j];
                usYPlaneVecTexTarget[i * nTexWidth + j] = TempShortBufVec2[nTexWidth * i + j];
                usYPlaneVecGeoTarget[i * nTexWidth + j] = TempShortBufVec3[nTexWidth * i + j];

            }
        }

        nImageWidth /= 2;
        nImageHeight /= 2;

        for (int i = 0; i < nImageHeight; i++)
        {
            for (int j = 0; j < nImageWidth; j++)
            {
                
                usUVPlaneVecTexSource[0][i * nImageWidth + j] = TempShortBufVec0[nImageHeight * i + j + nTexWidth * nTexHeight];
                usUVPlaneVecTexSource[1][i * nImageWidth + j] = TempShortBufVec0[nImageHeight * i + j + (nTexWidth / 2 * nTexHeight / 2) + nTexWidth * nTexHeight];

                usUVPlaneVecGeoSource[0][i * nImageWidth + j] = TempShortBufVec1[nImageHeight * i + j + nTexWidth * nTexHeight];
                usUVPlaneVecGeoSource[1][i * nImageWidth + j] = TempShortBufVec1[nImageHeight * i + j + (nTexWidth / 2 * nTexHeight / 2) + nTexWidth * nTexHeight];

                usUVPlaneVecTexTarget[0][i * nImageWidth + j] = TempShortBufVec2[nImageHeight * i + j + nTexWidth * nTexHeight];
                usUVPlaneVecTexTarget[1][i * nImageWidth + j] = TempShortBufVec2[nImageHeight * i + j + (nTexWidth / 2 * nTexHeight / 2) + nTexWidth * nTexHeight];

                usUVPlaneVecGeoTarget[0][i * nImageWidth + j] = TempShortBufVec3[nImageHeight * i + j + nTexWidth * nTexHeight];
                usUVPlaneVecGeoTarget[1][i * nImageWidth + j] = TempShortBufVec3[nImageHeight * i + j + (nTexWidth / 2 * nTexHeight / 2) + nTexWidth * nTexHeight];
                
            }
        }

        TempShortBufVec0.clear();
        TempShortBufVec1.clear();
        TempShortBufVec2.clear();
        TempShortBufVec3.clear();

        TempShortBufVec0.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);
        TempShortBufVec1.resize(nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2);

        for (int i = 0; i < nTexHeight; i++)
        {
            for (int j = 0;j < nTexWidth; j++)
            {
                int nHalfWidhth = nTexWidth / 2;
                int nHalfHeight = nTexHeight / 2;

                if (usYPlaneVecGeoSource[i * nTexWidth + j] < usYPlaneVecGeoTarget[i * nTexWidth + j])
                {
                    usYPlaneVecTexOut[i * nTexWidth + j] = usYPlaneVecTexTarget[i * nTexWidth + j];
                    usYPlaneVecGeoOut[i * nTexWidth + j] = usYPlaneVecGeoTarget[i * nTexWidth + j];
                    
                    usUVPlaneVecTexOut[0][i / 2 * nHalfWidhth + j/2] = usUVPlaneVecTexTarget[0][i / 2 * nHalfWidhth + j/2];
                    usUVPlaneVecTexOut[1][i/2 * nHalfWidhth + j/2] = usUVPlaneVecTexTarget[1][i/2 * nHalfWidhth + j/2];

                }
                else
                {
                    usYPlaneVecTexOut[i * nTexWidth + j] = usYPlaneVecTexSource[i * nTexWidth + j];
                    usYPlaneVecGeoOut[i * nTexWidth + j] = usYPlaneVecGeoSource[i * nTexWidth + j];

                    usUVPlaneVecTexOut[0][i / 2 * nHalfWidhth + j/2] = usUVPlaneVecTexSource[0][i / 2 * nHalfWidhth + j/2];
                    usUVPlaneVecTexOut[1][i / 2 * nHalfWidhth + j/2] = usUVPlaneVecTexSource[1][i / 2 * nHalfWidhth + j/2];
                }
            }
        }
        //copy 

        std::memcpy(TempShortBufVec0.data(), usYPlaneVecTexOut.data(), usYPlaneVecTexOut.size() * sizeof(unsigned short));
        std::memcpy(TempShortBufVec0.data() + usYPlaneVecTexOut.size(), usUVPlaneVecTexOut[0].data(), usUVPlaneVecTexOut[0].size() * sizeof(unsigned short));
        std::memcpy(TempShortBufVec0.data() + usYPlaneVecTexOut.size() + usUVPlaneVecTexOut[0].size(), usUVPlaneVecTexOut[1].data(), usUVPlaneVecTexOut[1].size() * sizeof(unsigned short));

        std::memcpy(TempShortBufVec1.data(), usYPlaneVecGeoOut.data(), usYPlaneVecGeoOut.size() * sizeof(unsigned short));
        std::memcpy(TempShortBufVec1.data() + usYPlaneVecGeoOut.size(), usUVPlaneVecGeoOut[0].data(), usUVPlaneVecGeoOut[0].size() * sizeof(unsigned short));
        std::memcpy(TempShortBufVec1.data() + usYPlaneVecGeoOut.size() + usUVPlaneVecGeoOut[0].size(), usUVPlaneVecGeoOut[1].data(), usUVPlaneVecGeoOut[1].size() * sizeof(unsigned short));


        strCompositionRetTexPath = strCompositedRetOutPath + strFilenameTexComposited + ".yuv";
        strCompositionRetGeoPath = strCompositedRetOutPath + strFilenameGeoComposited + ".yuv";
        strCompositionRetEntityPath = strCompositedRetOutPath + strFilenameEntityComposited  + ".yuv";


        // for texture
        fstream outfile;
        outfile.open(strCompositionRetTexPath, ios::out | ios::binary);
        int nSizeInByte = (nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2) * 2;
        outfile.write((char*) & TempShortBufVec0[0], nSizeInByte);
        outfile.close();

        // for geometry
        outfile.open(strCompositionRetGeoPath, ios::out | ios::binary);
        nSizeInByte = (nTexWidth * nTexHeight + nTexWidth * nTexHeight / 2) * 2;
        outfile.write((char*)&TempShortBufVec1[0], nSizeInByte);
        outfile.close();
    }
}

void Renderer::getBoundingBox(float fMaxOfBox[3], float fMinOfBox[3], float fx, float fy, float fz)
{
    if (fMinOfBox[0] > fx)
    {
        fMinOfBox[0] = fx;
    }

    if (fMinOfBox[1] > fy)
    {
        fMinOfBox[1] = fy;
    }

    if (fMinOfBox[2] > fz)
    {
        fMinOfBox[2] = fz;
    }

    if (fMaxOfBox[0] <= fx)
    {
        fMaxOfBox[0] = fx;
    }

    if (fMaxOfBox[1] <= fy)
    {
        fMaxOfBox[1] = fy;
    }

    if (fMaxOfBox[2] <= fz)
    {
        fMaxOfBox[2] = fz;
    }
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


    for (int nView = 0; nView <= nMaxCamCount; nView++)
    {
       /* if (bPointCloudConversion) {*/
            FileTools::YUVToGeoTex(strMIVSequencePath, strPostfixGeo, strGeoBitDepth, nView, SceneGeo[nView],
                shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight);
       // }
        FileTools::YUVToRGBTex(strMIVSequencePath,
            strPostfixTex,
            strTexBitDepth,
            nView,
            SceneTex[nView],
            shaderNodeHetroObj.nHetroBGImageDimWidth, shaderNodeHetroObj.nHetroBGImageDimHeight, bPointCloudConversion);

    }

    for (int i = 0; i <= nMaxCamCount; i++)
    {
        ViewID[i][0] = cameras.at(i).position[0];
        ViewID[i][1] = cameras.at(i).position[2];
        ViewID[i][2] = cameras.at(i).position[1];

        ViewRotID[i][0] = cameras.at(i).rotation[0];
        ViewRotID[i][1] = cameras.at(i).rotation[1];
        ViewRotID[i][2] = cameras.at(i).rotation[2];

        pCamProp[i] = new CammeraProperty(ViewID[i], ViewRotID[i]);
    }

    //if (bPointCloudConversion) {
        struct PointCloudAttribute
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };

        int nFactorResolution = 4;

        for (int c = 0; c <= nMaxCamCount; c++)
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

                            
                            getBoundingBox(fMaxOfBoxMIV, fMinOfBoxMIV, fOutputPoints[0], fOutputPoints[1], fOutputPoints[2]);

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
            if (bPointCloudConversion) {
                
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
               
                std::string strPointCLoudPath = strPointCloudOutPath + std::to_string(c) + ".ply";
                pcc::ply::PropertyNameMap propNames;
                propNames.position = { "x", "y", "z" };
                pcc::ply::write(OutputPointCloud, propNames, 1.0, 0.0, strPointCLoudPath, 1);
            }
            pointCloud.clear();
            OutputPointCloud.clear();
            attrVec.clear();
            attrpointCloud.clear();
        }
     
    //}

    for (int i = 0; i <= nMaxCamCount; i++)
    {
        SceneTex[i].clear();
        SceneGeo[i].clear();

        delete pCamProp[i];
        pCamProp[i] = NULL;
    }

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

  
  for (int nView = 0; nView <= nMaxCamCount; nView++)
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



  for (int i = 0; i <= nMaxCamCount; i++)
  {
      ViewID[i][0] = cameras.at(i).position[0];
      ViewID[i][1] = cameras.at(i).position[2];
      ViewID[i][2] = cameras.at(i).position[1];

      ViewRotID[i][0] = cameras.at(i).rotation[0];
      ViewRotID[i][1] = cameras.at(i).rotation[1];
      ViewRotID[i][2] = cameras.at(i).rotation[2];

      pCamProp[i] = new CammeraProperty(ViewID[i], ViewRotID[i]);
  }

  readSequence(params, eVmeshSequence);

  initMeshHetroObj(fObjScale);
  initBufers_in();

  eVmeshSequence.setFrameIndex(0);
  vMeshObj* pGeometry = (vMeshObj*)(eVmeshSequence.getObject().getGeometry());

  initShader();

  LoadOBJ::loadHetro(Hetro_vertices[0], Hetro_normals[0], Hetro_texcoords[0], Hetro_indexes[0], meshHetroObj);
  //LoadOBJ::loadHetro(vertex_in, normal_in, texcoords_in, indices_in, meshSphere);

#ifdef IMAGPROCESSINGTEST
   //shaderNodeInputTex.TestImage = SceneTex[0];
  //shaderNodeInputTex.setUniformsFromFile(FileTools::findFile("data/parameters.csv"));
  //shaderNodeInputTexMipmap.setUniformsFromFile(FileTools::findFile("data/parameters_lod.csv"));
  //shaderNodeGray.setUniformsFromFile(FileTools::findFile("data/parameters_gray.csv"));
  //shaderNodeRowAvg.setUniformsFromFile(FileTools::findFile("data/parameters_RowAvg.csv"));
  //shaderNodeColAvg.setUniformsFromFile(FileTools::findFile("data/parameters_ColAvg.csv"));
  //shaderNodePDFJoint.setUniformsFromFile(FileTools::findFile("data/parameters_PDFJoint.csv"));
  //shaderNodePDFMarg.setUniformsFromFile(FileTools::findFile("data/parameters_PDFMarg.csv"));
  //shaderNodePDFCond.setUniformsFromFile(FileTools::findFile("data/parameters_PDFCond.csv"));
  //shaderNodeCDFMarg.setUniformsFromFile(FileTools::findFile("data/parameters_CDFMarg.csv"));
  //shaderNodeCDFCond.setUniformsFromFile(FileTools::findFile("data/parameters_CDFCond.csv"));
  //shaderNodeEnv.setUniformsFromFile(FileTools::findFile("data/parameters_Env.csv"));
  std::string strImageProcessingParaPath = strMIVSequencePath;
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
#else
  std::string strImageProcessingParaPath = strMIVSequencePath;

  std::string strImageProcUniform = "Museum/B,Image,,,,\n\
 Gray, Image,,,,\n\
 RowAvg, Image,,,,\n\
 ColAvg, Image,,,,\n\
 PDFMarg, Image,,,,\n\
 PDFJoint, Image,,,,\n\
 PDFCon, Image,,,,\n\
 CDFCon, Image,,,,\n\
 CDFMarg, Image,,,,\n\
 level, Float, 0,,,\n\
 Background, Color, 0, 0, 0, 1\n\
 width, Integer, 1024,,,\n\
 height, Integer, 512,,,";


  shaderNodeInputTex.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeInputTexMipmap.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeGray.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeRowAvg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeColAvg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodePDFJoint.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodePDFMarg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodePDFCond.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeCDFMarg.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeCDFCond.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
  shaderNodeEnv.setUniformsFromFile(FileTools::findFile("data/ImageProcessingUniform.csv"), strEnvironmentmapFile);
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
  //fragSrc += "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n";
  fragSrc += "  } else {\n";
  fragSrc += "    discard;\n";
  fragSrc += "  }\n";
  fragSrc += "}\n";
  shaderNodeImageWindowPlane.setShaderSource("", fragSrc);

#else

  shaderNodeHetroObj.image_hetro_ = pGeometry->m_Meshes[0].getTextures()[0].data_.data();
  shaderNodeHetroObj.nHetroImageDimWidth = pGeometry->m_Meshes[0].getTextures()[0].width_;
  shaderNodeHetroObj.nHetroImageDimHeight = pGeometry->m_Meshes[0].getTextures()[0].height_;

  std::string strContents = "baseColorTexture,Image,,,,,,,,,,,,,,,,,,\n\
 MyTex, Image,,,,,,,,,,,,,,,,,,\n\
 envmapDiffuse, Image,,,,,,,,,,,,,,,,,,\n\
 brdfIntegrationMap, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel0, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel1, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel2, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel3, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel4, Image,,,,,,,,,,,,,,,,,,\n\
 envmapSpecularLevel5, Image,,,,,,,,,,,,,,,,,,\n\
 MIVBackgroundGuide, Image,,,,,,,,,,,,,,,,,,\n\
 NodeClassName, Text, ShaderPluginNode,,,,,,,,,,,,,,,,,\n\
 NodeName, Text, Shader,,,,,,,,,,,,,,,,,\n\
 Background, Color, 0, 0, 0, 1,,,,,,,,,,,,,,\n\
 IsDepth, Boolean, FALSE,,,,,,,,,,,,,,,,,\n\
 IsBackGroundGuide, Boolean, FALSE,,,,,,,,,,,,,,,,,\n\
 Wireframe, Boolean, FALSE,,,,,,,,,,,,,,,,,\n\
 mipLevelCount, Integer, 0,,,,,,,,,,,,,,,,,\n\
 metallic, Float, 0,,,,,,,,,,,,,,,,,\n\
 reflectance, Float, 0,,,,,,,,,,,,,,,,,\n\
 irradiPerp, Float, 10,,,,,,,,,,,,,,,,,\n\
 ProjTransformBackground, Matrix, 4, 4, 1, 0, 0, 0, 0.00E+00, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1\n\
 meshTransformBackground, Matrix, 4, 4, 1, 0, 0, 0, 0.00E+00, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1\n\
 meshTransform, Matrix, 4, 4, 1, 0, 0, 0, 0.00E+00, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1\n\
 uView, Matrix, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0";

  shaderNodeHetroObj.setUniformsFromFile(FileTools::findFile("data/parameters_hetro.csv"), "");
  shaderNodeHetroObj.setUniformsFromSeq("baseColorTexture");
  shaderNodeHetroObj.setUniformsFromMIVTex();
  
#endif

  meshDummyImagePlane.createQuad();
  
}

void Renderer::resize(int w, int h) {
  windowWidth = w;
  windowHeight = h;
}

void Renderer::printProgressBar(int progress, int total) {
    const int barWidth = 50; // Width of the progress bar in characters
    float percentage = static_cast<float>(progress) / total;
    int filledWidth = static_cast<int>(barWidth * percentage);

    std::cout << "\r["; // Carriage return to start from the beginning of the line
    for (int i = 0; i < filledWidth; ++i) {
        std::cout << "=";
    }
    for (int i = filledWidth; i < barWidth; ++i) {
        std::cout << " ";
    }
    if (percentage > 1.0) percentage = 1.0;
    std::cout << "] " << static_cast<int>(percentage * 100.0) << "%";
    std::cout.flush(); // Ensure the output is immediately written to the console
}

void Renderer::ImageProcessingInBg() {

    shaderNodeInputTex.render(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height/*, shaderSettingsBGImgProc.wireframe*/, true);
    shaderNodeInputTexMipmap.render(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height/*, shaderSettingsBGImgProc.wireframe*/, true);

    shaderNodeGray.setUniformImage("MyTex", shaderNodeInputTex.getRenderTarget(selectedOutput));
    shaderNodeGray.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height/*, shaderSettingsBGImgProc.wireframe*/, true);

    shaderNodeRowAvg.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodeRowAvg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height/*, shaderSettingsBGImgProc.wireframe*/, true);

    shaderNodeColAvg.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodeColAvg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodePDFJoint.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodePDFJoint.setUniformImage("ColAvg", shaderNodeColAvg.getRenderTarget(selectedOutput));
    shaderNodePDFJoint.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodePDFMarg.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodePDFMarg.setUniformImage("ColAvg", shaderNodeColAvg.getRenderTarget(selectedOutput));
    shaderNodePDFMarg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodePDFCond.setUniformImage("Gray", shaderNodeGray.getRenderTarget(selectedOutput));
    shaderNodePDFCond.setUniformImage("RowAvg", shaderNodeRowAvg.getRenderTarget(selectedOutput));
    shaderNodePDFCond.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodeCDFMarg.setUniformImage("PDFMarg", shaderNodePDFMarg.getRenderTarget(selectedOutput));
    shaderNodeCDFMarg.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodeCDFCond.setUniformImage("PDFCon", shaderNodePDFCond.getRenderTarget(selectedOutput));
    shaderNodeCDFCond.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

    shaderNodeEnv.setUniformImage("MyTex", shaderNodeInputTex.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("CDFMarg", shaderNodeCDFMarg.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("PDFJoint", shaderNodePDFJoint.getRenderTarget(selectedOutput));
    shaderNodeEnv.setUniformImage("CDFCon", shaderNodeCDFCond.getRenderTarget(selectedOutput));
    shaderNodeEnv.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, true);

}
void Renderer::display() {

  ImageProcessingInBg();
#ifdef IMAGPROCESSINGTEST
  // compute aspect ratio for shaderNodeImageWindowPlane
  float aspectX = 1.0;
  float aspectY = 1.0;
  float aspectShader = float(1024) / float(512);
  float aspectWindow = float(windowWidth) / float(windowHeight);
  if (aspectShader >= aspectWindow) {
    aspectY = aspectShader / 2;
  } else {
    aspectX = 2 / aspectShader;
  }
  shaderNodeImageWindowPlane.setUniformFloat("aspectX", aspectX);
  shaderNodeImageWindowPlane.setUniformFloat("aspectY", aspectY);

  shaderNodeImageWindowPlane.setUniformImage("img", shaderNodeInputTexMipmap.getRenderTarget(selectedOutput));
  shaderNodeImageWindowPlane.renderBgHDRImage(meshDummyImagePlane, shaderSettingsBGImgProc.backgroundColor, shaderSettingsBGImgProc.width, shaderSettingsBGImgProc.height, false);

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
  shaderNodeHetroObj.setUniformBool("IsEnvLight", bEnableEnvironmentRelighting);
  shaderNodeHetroObj.setUniformImage("envmapDiffuse", shaderNodeEnv.getRenderTarget(selectedOutput));
  shaderNodeHetroObj.setUniformImage("envmapSpecularLevel5", shaderNodeInputTexMipmap.getRenderTarget(selectedOutput));
  shaderNodeHetroObj.setUniformFloat("fFieldOfViewH", fFieldOfView0);
  shaderNodeHetroObj.setUniformFloat("fFieldOfViewV", fFieldOfView1);


  shaderNodeHetroObj.image_hetro_background =  &SceneTex[nCamIndex].at(0);

  shaderNodeHetroObj.nCurCamIndex = nCamIndex;

  strGeoOutputFile = "v" + std::to_string(nCamIndex) + strPostfixGeo + strGeoBitDepth + ".yuv";
  strTexOutputFile = "v" + std::to_string(nCamIndex) + strPostfixTex + strTexBitDepth + ".yuv";
  strEntityOutputFile = "v" + std::to_string(nCamIndex) + strPostfixEntity + ".yuv";
  
  if (bAutoCapture) {
      shaderNodeHetroObj.bCaptureing = true;
  }

  shaderNodeHetroObj.renderHetro(meshHetroObj,
      shaderSettingsBGImgProc.backgroundColor,
      mPlaneMeshTransform,
      mPlaneProjTransform, windowWidth, windowHeight, true, strGeoOutputFile, strTexOutputFile, strEntityOutputFile);

  shaderNodeHetroObj.renderHetro(meshHetroObj,
      shaderSettingsBGImgProc.backgroundColor, 
      mPlaneMeshTransform, 
      mPlaneProjTransform, windowWidth, windowHeight, false, strGeoOutputFile, strTexOutputFile, strEntityOutputFile);

  if (bAutoCapture) {

      nCamIndex++;
      if (nCamIndex == nMaxCamCount + 1 && !shaderNodeHetroObj.bIsDepth)
      {
          nCamIndex = 0;
          shaderNodeHetroObj.bIsDepth = true;
      }

      printProgressBar(nCamIndex, nMaxCamCount);

      if (nCamIndex == nMaxCamCount + 1 && shaderNodeHetroObj.bIsDepth)
      {
          std::cout << std::endl;
          std::cout << "[INFO] Capture process completed successfully!" << std::endl;

          int nWidth = shaderNodeHetroObj.nHetroBGImageDimWidth;
          int nHeight = shaderNodeHetroObj.nHetroBGImageDimHeight;
          std::string strOutputPath = shaderNodeHetroObj.strHetroObjOutputPath;

          if (nProgMode == 0) {
              
              ComposeContents(strMIVSequencePath, strOutputPath, strPostfixTex, strPostfixGeo, strPostfixEntity, strTexBitDepth, strGeoBitDepth, strCompositedRetOutPath, nWidth, nHeight, bAutoComposition);

              for (int i = 0; i <= nMaxCamCount; i++)
              {
                  SceneTex[i].clear();
                  SceneGeo[i].clear();

                  delete pCamProp[i];
                  pCamProp[i] = NULL;
              }
          }
          std::cout << "[INFO] Composition process completed successfully!" << std::endl;
          exit(0);
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
      //if (type == "text" && name == "NodeClassName" && items.size() >= 3) {
      //  settings.nodeClassName = items[2];
      //}
      //if (type == "text" && name == "NodeName" && items.size() >= 3) {
      //  settings.nodeName = items[2];
      //}
    }
  }

  return settings;
}