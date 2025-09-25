
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


using namespace std;
using namespace gsn;

static Renderer* renderer;

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
    //float Trs[16];
    Matrix meshTranformMat = renderer->shaderNodeHetroObj.uniforms.at("meshTransform").matVal;

    if (key == 'a')
    {
        //
        meshTranformMat.e[12] += 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "glutKeyboard event 'a' pressed" << std::endl;
    }

    if (key == 's')
    {
        meshTranformMat.e[14] += 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "glutKeyboard event 's' pressed" << std::endl;
    }
    if (key == 'd')
    {
        meshTranformMat.e[12] -= 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "glutKeyboard event 'd' pressed" << std::endl;
    }
    if(key == 'w')
    {
        meshTranformMat.e[14] -= 0.1;
        renderer->shaderNodeHetroObj.setUniformMatrix("meshTransform", meshTranformMat);
        std::cout << "glutKeyboard event 'w' pressed" << std::endl;
    }

    if (key == 'u')
    {
        renderer->nCamIndex++;
        if (renderer->nCamIndex == renderer->nMaxCamCount)
        {
            renderer->nCamIndex = 0;
        }

        std::cout << "glutKeyboard event 'u' pressed " << std::endl;
    }

    if (key == 'y')
    {
        renderer->nCamIndex--;
        if (renderer->nCamIndex == -1)
        {
            renderer->nCamIndex = 23;
        }

        std::cout << "glutKeyboard event 'y' pressed " << std::endl;
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
int main(int argc, char** argv)
{
    printf("################## Software manual ###########################\n");
    printf("################## argv:1 MIV Sequence Path (Data)############\n");
    printf("############ argv:2 Json Path (Cam info) #####################\n");
    printf("############ argv:3 Out put path (obj, ply, YUV) #############\n");
    printf("###########  argv:4 Mesh/point cloud resolution ##############\n");
    printf("########### argv:5 Heterogeneous object (obj, ply) ###########\n");
    printf("######### Heterogeneous object control key (a,s,d,f) #########\n");
    printf("#################  Camera control key (u,v) ##################\n");
    printf("#################  MIV contents capture (c) ##################\n");
    printf("#################  Eanble guide background (b)################\n");
    printf("#################  Depth mode on/off (v) #####################\n");
    printf("#####################  End ###################################\n");

    std::string strMIVSequencePath = argv[1];
    std::string strJsonPath = argv[2];
    std::string strOutputPath = argv[3];
    std::string strOutputType = argv[4];
    std::string strHeterObjPath = argv[5];

    renderer = new Renderer;

    renderer->strMIVSequencePath = strMIVSequencePath;
    renderer->strJsonPath = strJsonPath;
    renderer->strOutputPath = strOutputPath;
    renderer->strOutputType = strOutputType;
    renderer->strHeterObjPath = strHeterObjPath;


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1024, 1024);

    glutCreateWindow("Environment Map Heterogeneous Injection");

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