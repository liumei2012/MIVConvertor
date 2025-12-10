
#include <GL/glew.h>
#include <GL/freeglut.h> // we use glut here as window manager
#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Renderer.h"
#include "json.hpp"

using json = nlohmann::json;

using namespace std;
using namespace gsn;

static Renderer* renderer;
static float fangleY = 0.0;
static void glutDisplay()
{
    renderer->display();
    glutSwapBuffers();
    glutReportErrors();
}

static void glutKeyboard(unsigned char key, int x, int y)
{
    bool redraw = false;
    std::string modeStr;
    std::stringstream ss;
    if (key >= '1' && key <= '9') {
        renderer->selectedOutput = int(key) - int('1');
        
    }

    Matrix meshTranformMat = renderer->shaderNodeHetroObj.uniforms.at("meshTransform").matVal;

    if (key == 'a')
    {
        meshTranformMat.e[12] += 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'a' action=pressed" << std::endl;
    }

    if (key == 's')
    {
        meshTranformMat.e[14] += 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 's' action=pressed" << std::endl;
    }
    if (key == 'd')
    {
        meshTranformMat.e[12] -= 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'd' action=pressed " << std::endl;
    }
    if(key == 'w')
    {
        meshTranformMat.e[14] -= 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'w' action=pressed " << std::endl;
    }
    if (key == 'l')
    {
        renderer->bEnableEnvironmentRelighting = !renderer->bEnableEnvironmentRelighting;
        std::cout << "[INFO] event=glutKeyboard key= 'l' action=pressed " << std::endl;
    }

    if (key == 'u')
    {
        renderer->nCamIndex++;
        if (renderer->nCamIndex == renderer->nMaxCamCount + 1)
        {
            renderer->nCamIndex = 0;
        }

        std::cout << "[INFO] event=glutKeyboard key= 'u' action=pressed " << std::endl;
    }

    if (key == 'y')
    {
        renderer->nCamIndex--;
        if (renderer->nCamIndex == -1)
        {
            renderer->nCamIndex = renderer->nMaxCamCount - 1;
        }

        std::cout << "[INFO] event=glutKeyboard key= 'y' action=pressed " << std::endl;
    }
    if (key == 'b')
    {
        bool bFlag = renderer->shaderNodeHetroObj.bIsDepth;
        renderer->shaderNodeHetroObj.bIsDepth = !bFlag;
        std::cout << "[INFO] event=glutKeyboard key= 'b' action=pressed " << bFlag << std::endl;
    }
    if (key == 'c')
    {
        //renderer->shaderNodeHetroObj.bCaptureing = true;
        renderer->nCamIndex = 0;
        renderer->bAutoCapture = true;
        std::cout << "[INFO] event=glutKeyboard key= 'c' action=pressed " << std::endl;
    }

    if (key == 'r')
    {
        fangleY += 0.1;
      
        meshTranformMat.e[0] = cos(fangleY); meshTranformMat.e[4] = 0.0; meshTranformMat.e[8] = sin(fangleY); meshTranformMat.e[12] = meshTranformMat.e[12];
        meshTranformMat.e[1] = 0.0; meshTranformMat.e[5] = 1.0; meshTranformMat.e[9] = 0.0; meshTranformMat.e[13] = meshTranformMat.e[13];
        meshTranformMat.e[2] = -sin(fangleY); meshTranformMat.e[6] = 0.0; meshTranformMat.e[10] = cos(fangleY); meshTranformMat.e[14] = meshTranformMat.e[14];
        meshTranformMat.e[3] = 0.0; meshTranformMat.e[7] = 0.0; meshTranformMat.e[11] = 0.0; meshTranformMat.e[15] = 1.0;

        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'r' action=pressed" << std::endl;
    }

    if (key == 'q')
    {
        meshTranformMat.e[13] += 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'q' action=pressed" << std::endl;
    }

    if (key == 'e')
    {
        meshTranformMat.e[13] -= 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "[INFO] event=glutKeyboard key= 'e' action=pressed" << std::endl;
    }
}

static void glutResize(int w, int h)
{
    renderer->resize(w, h);
}

static void glutClose()
{
    renderer->dispose();
    delete renderer;
}

static void timer(int v)
{
    float offset = 0.0;
    renderer->t += offset;
    glutDisplay();
    glutTimerFunc(unsigned(20), timer, ++v);
}


#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void print_manual(int nMode) {
    if (nMode == 0) {
        printf(CYAN BOLD "###################### Software Manual ########################\n" RESET);
        printf(YELLOW "argv:1 " RESET "Program Mode (0: Heterogeneous Object → MIV [OpenGL Viewer], 1: MIV → Point Cloud Conversion)\n");
        printf(YELLOW "argv:2 " RESET "Path to the MIV sequence data(.yuv)\n");
        printf(YELLOW "argv:3 " RESET "Path to the MIV sequence description file (.json)\n");
        printf(YELLOW "argv:4 " RESET "Prefix string for texture files(.yuv)\n");
        printf(YELLOW "argv:5 " RESET "Prefix string for depth files(.yuv)\n");
        printf(YELLOW "argv:6 " RESET "Texture bit-depth format (e.g., 10-bit → 10le)\n");
        printf(YELLOW "argv:7 " RESET "Depth bit-depth format (e.g., 16-bit → 16le)\n");
        printf(YELLOW "argv:8 " RESET "Path to heterogeneous object (.obj)\n");
        printf(YELLOW "argv:9 " RESET "Output folder \n");
        printf(YELLOW "argv:10 " RESET "Enable composition (0 or 1)\n");
        printf(YELLOW "argv:11 " RESET "Path for composited results \n");

        printf(GREEN "Keys: " RESET "a, s, d, f, r, q, e → Control heterogeneous object\n");
        printf(GREEN "Keys: " RESET "u, v → Control camera\n");
        printf(GREEN "Keys: " RESET "c → Capture MIV content\n");
        printf(GREEN "Keys: " RESET "b → Toggle guide background\n");
        printf(GREEN "Keys: " RESET "v → Toggle depth mode\n");
        printf(GREEN "Keys: " RESET "l → Toggle re-lighting mode\n");
        printf(RED   BOLD "########################## End ###############################\n" RESET);
        printf(RED   BOLD "Press any key to start. \n" RESET);

    }
    else if (nMode == 1)
    {
        printf(CYAN BOLD "###################### Software Manual ########################\n" RESET);
        printf(YELLOW "argv:1 " RESET "Program Mode (0: Heterogeneous Object → MIV [OpenGL Viewer], 1: MIV → Point Cloud)\n");
        printf(YELLOW "argv:2 " RESET "Path to the MIV sequence data(.yuv)\n");
        printf(YELLOW "argv:3 " RESET "Path to the MIV sequence description file (.json) (.json)\n");
        printf(YELLOW "argv:4 " RESET "Prefix string for texture files\n");
        printf(YELLOW "argv:5 " RESET "Prefix string for depth files\n");
        printf(YELLOW "argv:6 " RESET "Texture bit-depth format (e.g., 10-bit → 10le)\n");
        printf(YELLOW "argv:7 " RESET "Depth bit-depth format (e.g., 16-bit → 16le)\n");
        printf(YELLOW "argv:8 " RESET "Output folder path\n");
        printf(RED   BOLD "########################## End ###############################\n" RESET);
        printf(RED   BOLD "Press any key to start. \n" RESET);

    }
}

namespace fs = std::filesystem;
void CheckDirectory(std::string path)
{
    try {
        if (!fs::exists(path)) {
            if (fs::create_directories(path)) {
                std::cout << "[INFO] Create directory: " << path << std::endl;
            }
            else {
                std::cout << "[ERROR] Fail to create directory." << std::endl;
            }
        }
        else {
            std::cout << "[INFO] Directory already exist: " << path << std::endl;
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] File system error: " << e.what() << std::endl;
    }
}

void JsonParser(std::string jsonfile, std::vector<Camera>& cameras)
{
    std::ifstream ifs(jsonfile.data());
    if (!ifs) { std::cerr << "Can not open json.\n"; exit(0) ; }

    json root;
    try { ifs >> root; }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] JSON error " << e.what() << "\n"; exit(0) ;
    }

    int nCamCount = 0;
    bool bIntrestCam = false;
    //
    if (root.contains("cameras") && root["cameras"].is_array()) {
        for (const auto& c : root["cameras"])
        {

            std::string name = c.at("Name").get<std::string>();

            std::array<double, 3> position = c.at("Position").get<std::array<double, 3>>();
            std::array<double, 3> rotation = c.at("Rotation").get<std::array<double, 3>>();
            std::array<int, 2> resolution = c.at("Resolution").get<std::array<int, 2>>();

            std::array<int, 2> HorRange;
            std::array<int, 2> VerRange;

            std::array<double, 2> depthrange = c.at("Depth_range").get<std::array<double, 2>>();
            int bit_depth_color = c.at("BitDepthColor").get<int>();
            int bit_depth_depth = c.at("BitDepthDepth").get<int>();


            if (c.contains("Hor_range") && c.contains("Ver_range") && !(name == "viewport" || name == "center"))
            {
                Camera cam;
                cam.name = name;
                cam.position = position;
                cam.rotation = rotation;
                cam.resolution = resolution;

                HorRange = c.at("Hor_range").get<std::array<int, 2>>();
                VerRange = c.at("Ver_range").get<std::array<int, 2>>();

                cam.HorRange = HorRange;
                cam.VerRange = VerRange;
                cam.depth_range = depthrange;
                cam.bit_depth_color = bit_depth_color;
                cam.bit_depth_depth = bit_depth_depth;

                cameras.push_back(cam);

                bIntrestCam = true;
                nCamCount++;
            }
        }
    }
}

int main(int argc, char** argv)
{
    std::string strSW;
    std::string strMode;
    std::string strMIVSequencePath;
    std::string strMIVSequenceJsonPath;
    std::string strPostTexFix;
    std::string strPostGeoFix;
    std::string strPostEntityFix;

    std::string strBitTexDepth;
    std::string strBitGeoDepth;
    std::string strHeterObjPath;
    std::string strNoCam;

    std::string strHetroObjTexWidth;
    std::string strHetroObjTexHeight;
    std::string strBGTexWidth;
    std::string strBGTexHeight;
    std::string strHeterObjOutPath;
    std::string strPointCloudOutPath;
    std::string strEnvironmentMap;

    std::string strFieldOfView0;
    std::string strFieldOfView1;
    std::string strNearPlane;
    std::string strFarPlane;

   // bool bAutoCapture = false;
    bool bAutoComposition = false;
    std::string strCompositedRetOutPath;

    strSW = argv[0];
    strMode = argv[1];
    strMIVSequencePath = argv[2];
    strMIVSequenceJsonPath = argv[3];
    strPostTexFix = argv[4];
    strPostGeoFix = argv[5];
    strPostEntityFix = argv[6];
    strBitTexDepth = argv[7];
    strBitGeoDepth = argv[8];

    CheckDirectory("Output");
    CheckDirectory("CompositedResults");

    renderer = new Renderer;
    print_manual(atoi(strMode.data()));
    std::string strCommnd = strSW + " " 
        + strMode 
        + " " 
        + strMIVSequencePath
        + " " 
        + strMIVSequenceJsonPath
        + " " 
        + strPostTexFix
        + " " 
        + strPostGeoFix 
        + " "
        + strPostEntityFix
        + " "
        + strBitTexDepth
        + " "
        + strBitGeoDepth
        ;

    if (argc == 10)
    {
        strPointCloudOutPath = argv[9]; 
        strCommnd = strCommnd + " " + strPointCloudOutPath;
    }

    if (argc == 14)
    {
        strHeterObjPath = argv[9];
        strHeterObjOutPath = argv[10];
       
        bAutoComposition = !!atoi(argv[11]);
        strCompositedRetOutPath = argv[12];
        strEnvironmentMap = argv[13];

        strCommnd = strCommnd + " " + strHeterObjPath + " " 
            + strHeterObjOutPath 
            //+ " " + std::to_string(bAutoCapture) 
            + " " + std::to_string(bAutoComposition)
            + " " + strCompositedRetOutPath
            + " " + strEnvironmentMap; 
    }

    std::cout << strCommnd << std::endl;

    getchar();

    JsonParser(strMIVSequenceJsonPath.data(), renderer->cameras);

    renderer->strMIVSequencePath = strMIVSequencePath;
    renderer->nProgMode = atoi(strMode.data());

    renderer->strPostfixTex = strPostTexFix;
    renderer->strPostfixGeo = strPostGeoFix;
    renderer->strPostfixEntity = strPostEntityFix;

    renderer->strTexBitDepth = strBitTexDepth;
    renderer->strGeoBitDepth = strBitGeoDepth;
    renderer->strHeterObjPath = strHeterObjPath;
    renderer->nMaxCamCount = renderer->cameras.size()-1;

    renderer->shaderNodeHetroObj.nHetroBGImageDimWidth = renderer->cameras.at(0).resolution[0];
    renderer->shaderNodeHetroObj.nHetroBGImageDimHeight = renderer->cameras.at(0).resolution[1];

    renderer->shaderNodeHetroObj.strHetroObjOutputPath = strHeterObjOutPath;
    renderer->bAutoCapture = false;

    int nHorDegree = -renderer->cameras.at(0).HorRange[0] + renderer->cameras.at(0).HorRange[1];
    double dRadianHor = (nHorDegree == 360) ? 3.14159 : 1.5708;
    renderer->fFieldOfView0 = dRadianHor;

    int nVerDegree = -renderer->cameras.at(0).VerRange[0] + renderer->cameras.at(0).VerRange[1];
    double dRadianVer = (nVerDegree == 180) ? 1.5708 : 3.14159;
    renderer->fFieldOfView1 = dRadianVer;

    renderer->fNearPlane = atof(strNearPlane.data());
    renderer->fNearPlane = renderer->cameras.at(0).depth_range[0];

    renderer->fFarPlane = atof(strFarPlane.data());
    renderer->fFarPlane = renderer->cameras.at(0).depth_range[1];

    renderer->bAutoComposition = bAutoComposition;
    renderer->strCompositedRetOutPath = strCompositedRetOutPath;
    renderer->strPointCloudOutPath = strPointCloudOutPath;
    renderer->strEnvironmentmapFile = strEnvironmentMap;

    renderer->Preinit();

    if (renderer->nProgMode == 1)
    {       
        delete renderer;
        renderer = NULL;
        exit(0);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);

    int nWidonwSize[2] = { renderer->shaderNodeHetroObj.nHetroBGImageDimWidth * 0.25, renderer->shaderNodeHetroObj.nHetroBGImageDimHeight * 0.25 };

    glutInitWindowSize(nWidonwSize[0], nWidonwSize[1]);

    glutCreateWindow("MIV contents convertor");

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
    }


    glutDisplayFunc(glutDisplay);
    glutReshapeFunc(glutResize);
    glutCloseFunc(glutClose);
    glutKeyboardFunc(glutKeyboard);

    renderer->init();

    glutTimerFunc(unsigned(20), timer, 0);
    glutMainLoop();
}