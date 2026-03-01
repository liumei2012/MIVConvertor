// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#include <string>
#include <iostream> 
#include <iomanip>
#include <fstream>
#include <sstream>
#include <tuple>
#include <map>
#include <algorithm>

using namespace std;

#include <GL/glew.h>

#include "ShaderNode.h"
#include "StringTools.h"
#include "FileTools.h"
#include "Mesh.h"
using namespace gsn;
unsigned short BrightnessMap[8388608 + 4194304];
unsigned char BrightnessMapByte[8388608][4];
char* buffer = NULL;
#define CLIP(x) (x < 0 ? 0 : (x > 255 ? 255 : x))
// some static helper functions
namespace gsn {
  void printShaderInfoLog(GLuint obj) {
    int infoLogLength = 0;
    int returnLength = 0;
    char* infoLog;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
      infoLog = (char*)malloc(infoLogLength);
      glGetShaderInfoLog(obj, infoLogLength, &returnLength, infoLog);
      printf("%s\n", infoLog);
      free(infoLog);
      exit(1);
    }
  }

  void printProgramInfoLog(GLuint obj) {
    int infoLogLength = 0;
    int returnLength = 0;
    char* infoLog;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
      infoLog = (char*)malloc(infoLogLength);
      glGetProgramInfoLog(obj, infoLogLength, &returnLength, infoLog);
      printf("%s\n", infoLog);
      free(infoLog);
      exit(1);
    }
  }

  bool checkFramebufferStatus() {
    GLenum status;
    status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
      return true;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      printf("Framebuffer incomplete, incomplete attachment\n");
      return false;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      printf("Unsupported framebuffer format\n");
      return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      printf("Framebuffer incomplete, missing attachment\n");
      return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      printf("Framebuffer incomplete, missing draw buffer\n");
      return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      printf("Framebuffer incomplete, missing read buffer\n");
      return false;
    }
    return false;
  }

  string findAttributeInComments(const std::string& line, const std::string& attr)
  {
    size_t index = StringTools::toLower(line).find(attr);
    if (index != std::string::npos) {
      string part = line.substr(index);
      size_t start = part.find("\"");
      if (start != std::string::npos) {
        string nextpart = part.substr(start + 1);
        size_t end = nextpart.find("\"");
        if (end != std::string::npos) {
          return nextpart.substr(0, end);
        }
      }

    }
    return "";
  }

  int strToBendFunc(const std::string& inputStr) {
    string str = StringTools::toLower(inputStr);
    int ret = GL_ONE;
    if (str.find("zero") != std::string::npos) {
      ret = GL_ZERO;
    }
    if (str.find("one") != std::string::npos) {
      ret = GL_ONE;
    }
    if (str.find("src_color") != std::string::npos && str.find("one_minus_src_") == std::string::npos) {
      ret = GL_SRC_COLOR;
    }
    if (str.find("one_minus_src_color") != std::string::npos) {
      ret = GL_ONE_MINUS_SRC_COLOR;
    }
    if (str.find("dst_color") != std::string::npos) {
      ret = GL_DST_COLOR;
    }
    if (str.find("one_minus_dst_color") != std::string::npos) {
      ret = GL_ONE_MINUS_DST_COLOR;
    }
    if (str.find("src_alpha") != std::string::npos) {
      ret = GL_SRC_ALPHA;
    }
    if (str.find("one_minus_src_alpha") != std::string::npos) {
      ret = GL_ONE_MINUS_SRC_ALPHA;
    }
    if (str.find("dst_alpha") != std::string::npos) {
      ret = GL_DST_ALPHA;
    }
    if (str.find("one_minus_dst_alpha") != std::string::npos) {
      ret = GL_ONE_MINUS_DST_ALPHA;
    }

    return ret;
  }

  bool beforeGLSLversion1_2(const std::string &code) {
    const std::vector<string>& lines = StringTools::split(code, "\n");
    for (int i = 0; i < int(lines.size()); i++) {
      string line = lines[i];
      const std::vector<string>& lineParts = StringTools::split(line);
      if (lineParts.size() > 0) {
        if (lineParts[0] == "attribute") {
          return true;
        }
        if (lineParts[0] == "varying") {
          return true;
        }
      }
    }
    return false;
  }

}


ShaderNode::~ShaderNode()
{
  if (progID > 0) {
    glDeleteProgram(progID);
  }
  if (vertID > 0) {
    glDeleteShader(vertID);
  }
  if (fragID > 0) {
    glDeleteShader(fragID);
  }

  for (auto& i : uniforms) {
    UniformVariable& u = i.second;
    if (u.texVal.texID > 0) {
      if (!u.texVal.underExternalControl) {
        glDeleteTextures(1, &u.texVal.texID);
      }
    }
  }

  for (int r = 0; r < int(renderTargets.size()); r++) {
    RenderTarget& renderTarget = renderTargets[r];
    if (renderTarget.texID > 0) {
      glDeleteTextures(1, &renderTarget.texID);
    }
  }

  if (fbo > 0) {
    glDeleteFramebuffers(1, &fbo);
  }
}

void ShaderNode::setShaderSource(const std::string& _vertexShader, const std::string& _fragmentShader)
{
  vertexShader = _vertexShader;
  fragmentShader = _fragmentShader;

  if (vertexShader.size() == 0) {
    // if vertex source is not provided, use default
    if (beforeGLSLversion1_2(fragmentShader)) {
      vertexShader = "attribute vec3 position;\n";
      vertexShader += "attribute vec2 texcoord;\n";
      vertexShader += "varying vec2 tc;\n";
      vertexShader += "void main()\n";
      vertexShader += "{\n";
      vertexShader += "  tc = texcoord;\n";
      vertexShader += "  gl_Position = vec4(position, 1.0);\n";
      vertexShader += "}\n";
    }
    else {
      vertexShader = "#version 300 es\n";
      vertexShader += "in vec3 position;\n";
      vertexShader += "in vec2 texcoord;\n";
      vertexShader += "out vec2 tc;\n";
      vertexShader += "void main()\n";
      vertexShader += "{\n";
      vertexShader += "  tc = texcoord;\n";
      vertexShader += "  gl_Position = vec4(position, 1.0);\n";
      vertexShader += "}\n";
    }
  }

  findAllUniforms(vertexShader);
  findAllUniforms(fragmentShader);
  findRenderOptions(vertexShader + "\n" + fragmentShader);
  findAllRenderTargets(fragmentShader);

  setupShaders();
  findAllUniformLocations();
}




void ShaderNode::setShaderSourceFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
  string vertexShader = "";
  if (vertexShaderFilename.size() > 0) {
    vertexShader = FileTools::readTextFile(vertexShaderFilename);
  }
  string fragmentShader = FileTools::readTextFile(fragmentShaderFilename);
  setShaderSource(vertexShader, fragmentShader); 
}

void ShaderNode::findTexturePara(UniformVariable& u,  const std::string& line) const{

  u.texVal.wrap_s = GL_CLAMP_TO_EDGE; // default
  string wraps = findAttributeInComments(line, "wrap_s");
  if (int(wraps.size()) > 0) {
    string str = StringTools::toLower(wraps);
    size_t pos = str.find("border");
    if (pos != std::string::npos) {
      u.texVal.wrap_s = GL_CLAMP_TO_BORDER;
    }
    pos = str.find("repeat");
    if (pos != std::string::npos) {
      u.texVal.wrap_s = GL_REPEAT;
    }
    pos = str.find("mirror");
    if (pos != std::string::npos) {
      u.texVal.wrap_s = GL_MIRRORED_REPEAT;
    }
  }

  u.texVal.wrap_t = GL_CLAMP_TO_EDGE; // default
  string wrapt = findAttributeInComments(line, "wrap_t");
  if (int(wrapt.size()) > 0) {
    string str = StringTools::toLower(wrapt);
    size_t pos = str.find("border");
    if (pos != std::string::npos) {
      u.texVal.wrap_t = GL_CLAMP_TO_BORDER;
    }
    pos = str.find("repeat");
    if (pos != std::string::npos) {
      u.texVal.wrap_t = GL_REPEAT;
    }
    pos = str.find("mirror");
    if (pos != std::string::npos) {
      u.texVal.wrap_t = GL_MIRRORED_REPEAT;
    }
  }

  u.texVal.mag_filter = GL_NEAREST;
  string magfilter = findAttributeInComments(line, "mag_filter");
  if (int(magfilter.size()) > 0) {
    string str = StringTools::toLower(magfilter);
    size_t pos = str.find("linear");
    if (pos != std::string::npos) {
      u.texVal.mag_filter = GL_LINEAR;
    }
  }

  u.texVal.min_filter = GL_LINEAR_MIPMAP_NEAREST;
  string minfilter = findAttributeInComments(line, "min_filter");
  if (int(minfilter.size()) > 0) {
    string str = StringTools::toLower(minfilter);
    size_t posnear = str.find("nearest");
    size_t posmip = str.find("mipmap");
    size_t poslinmip = str.find("linear_mipmap_linear");
    size_t poslin = str.find("linear");

    if (posnear != std::string::npos && posmip == std::string::npos) {
      u.texVal.min_filter =  GL_NEAREST;
    }
    if (poslin != std::string::npos && posmip == std::string::npos) {
      u.texVal.min_filter = GL_LINEAR;
    }
    if (poslinmip != std::string::npos) {
      u.texVal.min_filter = GL_LINEAR_MIPMAP_LINEAR;
    }
  }
}

void ShaderNode::findAllUniforms(const std::string& str) {
  const std::vector<string>& lines = StringTools::split(str, "\n");
  for (int i = 0; i < int(lines.size()); i++) {
    string line = lines[i];
    const std::vector<string>& lineParts = StringTools::split(line);
    if (int(lineParts.size()) >= 3) {
      if (lineParts[0] == "uniform") {
        string type = lineParts[1];
        string name = lineParts[2];
        std::size_t nameEndIndex = name.find(";");
        if (nameEndIndex != std::string::npos) {
          name = name.substr(0, nameEndIndex);
        }

        UniformVariable u;
        u.location = -1;
        string attr = findAttributeInComments(line, "defaultval");
        // 0 = int, 1 = float, 2 = bool, 3 = Matrix, 4 = Image
        if (type == "int") {
          u.type = 0;
          if (attr.length() > 0) {
            u.intVal = StringTools::stringToInt(attr);
          }
        }
        if (type == "float") {
          u.type = 1;
          if (attr.length() > 0) {
            u.floatVal = StringTools::stringToFloat(attr);
          }
        }
        if (type == "bool") {
          u.type = 2;
          if (attr.length() > 0) {
            u.boolVal = StringTools::stringToBool(attr);
          }
        }
        if (type == "vec2") {
          u.type = 3;
          u.matVal.resize(2, 1);
        }
        if (type == "vec3") {
          u.type = 3;
          u.matVal.resize(3, 1);
        }
        if (type == "vec4") {
          u.type = 3;
          u.matVal.resize(4, 1);
        }
        if (type == "mat4") {
          u.type = 3;
          u.matVal.resize(4, 4);
        }
        if (type == "sampler2D") {
          u.type = 4;
          findTexturePara(u, line);
        }

        if (u.type == 3 && attr.length() > 0) {
          const std::vector<string>& elements = StringTools::split(attr, ",");
          int num = std::min(int(elements.size()), int(u.matVal.e.size()));
          for (int n = 0; n < num; n++) {
            u.matVal.e[n] = StringTools::stringToFloat(elements[n]);
          }
        }
        uniforms.insert(std::make_pair(name, u));
      }
    }
  }
}

void ShaderNode::findRenderOptions(const std::string& code)
{
  // find all render options variables
  renderOptions.depthtest = true;
  renderOptions.srcFactor = GL_ONE;
  renderOptions.dstFactor = GL_ONE_MINUS_SRC_ALPHA;
  renderOptions.rgbSrcFactor = -1;
  renderOptions.rbgDstFactor = -1;
  renderOptions.aSrcFactor = -1;
  renderOptions.aDstFactor = -1;
  renderOptions.cullFace = -1;
  const std::vector<string>& lines = StringTools::split(code, "\n");
  for (int i = 0; i < int(lines.size()); i++) {
    string line = lines[i];
    size_t index = StringTools::toLower(line).find("gsnshaderoption"); // also finds "gsnShaderOptions" with "s"
    if (index != std::string::npos) {
      string attr = StringTools::toLower(findAttributeInComments(line, "depth_test"));
      if (attr.find("false") != std::string::npos || attr.find("disable") != std::string::npos) {
        renderOptions.depthtest = false;
      }
      attr = findAttributeInComments(line, "blend_func");
      if (attr.length() > 0) {
        const std::vector<string>& elements = StringTools::split(attr, ",");
        if (int(elements.size()) >= 2) {
          renderOptions.srcFactor = strToBendFunc(elements[0]);
          renderOptions.dstFactor = strToBendFunc(elements[1]);
          renderOptions.rgbSrcFactor = -1;
          renderOptions.rbgDstFactor = -1;
          renderOptions.aSrcFactor = -1;
          renderOptions.aDstFactor = -1;
        }
      }
      attr = findAttributeInComments(line, "blend_func_separate");
      if (attr.length() > 0) {
        const std::vector<string>& elements = StringTools::split(attr, ",");
        if (int(elements.size()) >= 4) {
          renderOptions.srcFactor = -1;
          renderOptions.dstFactor = -1;
          renderOptions.rgbSrcFactor = strToBendFunc(elements[0]);
          renderOptions.rbgDstFactor = strToBendFunc(elements[1]);
          renderOptions.aSrcFactor = strToBendFunc(elements[2]);
          renderOptions.aDstFactor = strToBendFunc(elements[3]);
        }
      }
      attr = StringTools::toLower(findAttributeInComments(line, "cull_face"));
      if (attr.length() > 0) {
        if (attr.find("back") != std::string::npos) {
          renderOptions.cullFace = GL_BACK;
        }
        if (attr.find("front") != std::string::npos) {
          renderOptions.cullFace = GL_FRONT;
        }
        if (attr.find("front_and_back") != std::string::npos) {
          renderOptions.cullFace = GL_FRONT_AND_BACK;
        }
      }
    }
  }
}

void ShaderNode::findAllRenderTargets(const std::string& code)
{
  for (int r = 0; r < int(renderTargets.size()); r++) {
    RenderTarget& renderTarget = renderTargets[r];
    if (renderTarget.texID > 0) {
      glDeleteTextures(1, &renderTarget.texID);
    }
  }
  renderTargets.clear();

  const std::vector<string>& lines = StringTools::split(code, "\n");
  for (int i = 0; i < int(lines.size()); i++) {
    string line = lines[i];
    const std::vector<string>& lineParts = StringTools::split(line);
    for (int p = 0; p < int(lineParts.size()); p++) {
      if (lineParts[p] == "out") {
        if (int(lineParts.size()) > p + 2) {
          if (lineParts[p + 1] == "vec4") {
            string name = lineParts[p + 2];
            name = StringTools::trimRight(name, ";");
            RenderTarget renderTarget;
            renderTarget.name = name;
            renderTarget.texID = 0;
            renderTargets.push_back(renderTarget);
          }
        }
      }
    }
  }

  if (renderTargets.size() == 0) {
    RenderTarget renderTarget;
    renderTarget.name = "default";
    renderTarget.texID = 0;
    renderTargets.push_back(renderTarget);
  }

  RenderTarget renderTargetDepth;
  renderTargetDepth.name = "depth";
  renderTargetDepth.texID = 0;
  renderTargets.push_back(renderTargetDepth);

}

void ShaderNode::setupShaders() {

  if (vertID > 0) {
    glDeleteShader(vertID);
  }
  if (fragID > 0) {
    glDeleteShader(fragID);
  }

  // create shader
  vertID = glCreateShader(GL_VERTEX_SHADER);
  fragID = glCreateShader(GL_FRAGMENT_SHADER);

  // specify shader source
  const char* vss = vertexShader.c_str();
  const char* fss = fragmentShader.c_str();
  glShaderSource(vertID, 1, &(vss), NULL);
  glShaderSource(fragID, 1, &(fss), NULL);

  // compile the shader
  glCompileShader(vertID);
  glCompileShader(fragID);

  // check for errors
  printShaderInfoLog(vertID);
  printShaderInfoLog(fragID);


  if (progID > 0) {
    glDeleteProgram(progID);
  }

  // create program and attach shaders
  progID = glCreateProgram();
  glAttachShader(progID, vertID);
  glAttachShader(progID, fragID);

  // link the program
  glLinkProgram(progID);
  // output error messages
  printProgramInfoLog(progID);

  positionAttr = glGetAttribLocation(progID, "position");
  normalAttr = glGetAttribLocation(progID, "normal");
  texCoordAttr = glGetAttribLocation(progID, "texcoord");
  colorAttr = glGetAttribLocation(progID, "color");
}

void ShaderNode::resizeFBO(int width, int height) {

  if (width == fboWidth && height == fboHeight) {
    return;
  }
  else {
    fboWidth = width;
    fboHeight = height;
  }

  for (int r = 0; r < int(renderTargets.size()); r++) {
    RenderTarget& renderTarget = renderTargets[r];
    if (renderTarget.texID > 0) {
      glDeleteTextures(1, &renderTarget.texID);
      renderTarget.texID = 0;
    }
  }

  if (fbo > 0) {
    glDeleteFramebuffers(1, &fbo);
  }

  // create a frame buffer object
  glGenFramebuffers(1, &fbo);

  // bind the frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  int colorBufferCounter = 0;
  for (int r = 0; r < int(renderTargets.size()); r++) {
    RenderTarget& renderTarget = renderTargets[r];

    if (renderTarget.name != "depth") {
      GLuint texID;
      glGenTextures(1, &texID);
      glBindTexture(GL_TEXTURE_2D, texID);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fboWidth, fboHeight, 0, GL_RGBA, GL_FLOAT, NULL);

      // Attach the texture to the fbo
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorBufferCounter, GL_TEXTURE_2D, texID, 0);
      if (!checkFramebufferStatus()) exit(1);
      renderTarget.texID = texID;
      colorBufferCounter++;
    } else {
      GLuint depthID;
      // Generate depth render texture
      glGenTextures(1, &depthID);
      glBindTexture(GL_TEXTURE_2D, depthID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, fboWidth, fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

      // Attach the texture to the fbo
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthID, 0);
      if (!checkFramebufferStatus()) exit(1);
      renderTarget.texID = depthID;
    }
  }
  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);
  //unbind fbo
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShaderNode::findAllUniformLocations()
{
  for (auto& i : uniforms) {
    UniformVariable& u = i.second;
    u.location = glGetUniformLocation(progID, i.first.c_str());
    if (u.location < 0) {
      
    }
  }
}

void ShaderNode::setAllUniforms()
{
  int texCount = 0;
  for (auto& i : uniforms) {
    UniformVariable& u = i.second;
    if (u.location != -1) {
      if (u.type == 0) { // "int"
        glUniform1i(u.location, u.intVal);
      }
      if (u.type == 1) { // "float"
        glUniform1f(u.location, u.floatVal);
      }
      if (u.type == 2) { // "bool"
        if (u.boolVal) {
          glUniform1i(u.location, 1);
        }
        else {
          glUniform1i(u.location, 0);
        }
      }
      if (u.type == 3) {
        if (u.matVal.rows == 2 && u.matVal.cols == 1) { // "vec2"
          glUniform2fv(u.location, 1, &u.matVal.e[0]);
        }
        if (u.matVal.rows == 3 && u.matVal.cols == 1) { // "vec3"
          glUniform3fv(u.location, 1, &u.matVal.e[0]);
        }
        if (u.matVal.rows == 4 && u.matVal.cols == 1) { // "vec4"
          glUniform4fv(u.location, 1, &u.matVal.e[0]);
        }
        if (u.matVal.rows == 4 && u.matVal.cols == 4) { // "mat4"
          glUniformMatrix4fv(u.location, 1, false, &u.matVal.e[0]);
        }
      }
      if (u.type == 4) {
        if (u.texVal.texID > 0) {
          glActiveTexture(GL_TEXTURE0 + texCount);
          // set texture parameters
          glGenerateMipmap(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, u.texVal.texID);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, u.texVal.wrap_s);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, u.texVal.wrap_t);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, u.texVal.mag_filter);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, u.texVal.min_filter);
          glUniform1i(u.location, texCount);
          texCount++;
        }
      }
    }
  }
}

void ShaderNode::setUniformsFromMIVTex() {
    //request textureID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nHetroImageDimWidth, nHetroImageDimHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    setUniformImage("MIVBackgroundGuide", textureID, false);
}

void ShaderNode::setUniformsFromSeq(std::string strname) {
    //request textureID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    nTexIDSeq = textureID;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nHetroImageDimWidth, nHetroImageDimHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image_hetro_);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    setUniformImage(strname, textureID, false);
}

void ShaderNode::renderHetro( const Mesh& meshHetroObj, 
    const Matrix& background, 
    Matrix& backgroundMeshTransformint, 
    Matrix& backgroundProjTransform, 
    int width, int height, 
    bool renderToFBO,
    std::string strGeoOutput,
    std::string strTexOutput,
    std::string strEntityOutput
    )
{
    if (renderToFBO)
    {
        width = nHetroBGImageDimWidth;
        height = nHetroBGImageDimHeight;
    }

    setUniformInteger("width", width);
    setUniformInteger("height", height);

    if (renderToFBO) {
        resizeFBO(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        std::vector<unsigned int> renderTar(renderTargets.size() - 1);
        for (int r = 0; r < int(renderTargets.size() - 1); r++) {
            renderTar[r] = GL_COLOR_ATTACHMENT0 + r;
        }
        // set the color attachments to write to
        glDrawBuffers(int(renderTargets.size()) - 1, &renderTar[0]);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    if (background.rows == 4 && background.cols == 1) {
        glClearColor(background.e[0], background.e[1], background.e[2], background.e[3]);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    if (renderOptions.depthtest) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    glEnable(GL_BLEND);
    if (renderOptions.srcFactor >= 0) {
        glBlendFunc(renderOptions.srcFactor, renderOptions.dstFactor);
    }
    if (renderOptions.rgbSrcFactor >= 0) {
        glBlendFuncSeparate(renderOptions.rgbSrcFactor, renderOptions.rbgDstFactor, renderOptions.aSrcFactor, renderOptions.aDstFactor);
    }
    if (renderOptions.cullFace >= 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(renderOptions.cullFace);
    }

    glViewport(0, 0, width, height);

    glUseProgram(progID);

    int i = -1;

    for (int r = 0; r < int(renderTargets.size()); r++) {
        RenderTarget& renderTarget = renderTargets[r];
        if (renderTarget.texID > 0) {
            glBindTexture(GL_TEXTURE_2D, renderTarget.texID);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    std::vector <int> enabledAttribArrayHetro;
    for (int i = 0; i < int(meshHetroObj.vertexBuffers.size()); i++) {
        const Mesh::VertexBuffer& vb = meshHetroObj.vertexBuffers[i];
        int loc = -1;
        if (vb.semantic == "position")
            loc = positionAttr;
        if (vb.semantic == "normal")
            loc = normalAttr;
        if (vb.semantic == "texcoord")
            loc = texCoordAttr;
        if (vb.semantic == "color")
            loc = colorAttr;
        if (loc >= 0) {
            
            glBindBuffer(GL_ARRAY_BUFFER, meshHetroObj.arrayBuffers[vb.arrayBufferID].arrayBufferOpenGL);
            //glBufferData(GL_ARRAY_BUFFER, 
            //    mesh2.arrayBuffers[vb.arrayBufferID].arrayBuffer.size() +
            //    mesh2.arrayBuffers[2].arrayBuffer.size() + 
            //    3 * 4 * sizeof(GLfloat) + 
            //    2 * 4 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

            //glBufferSubData(GL_ARRAY_BUFFER, 0, mesh2.arrayBuffers[vb.arrayBufferID].arrayBuffer.size() * sizeof(GLfloat), verticesPlane);
            //glBufferSubData(GL_ARRAY_BUFFER, mesh2.arrayBuffers[vb.arrayBufferID].arrayBuffer.size() * sizeof(GLfloat),3 * 4 * sizeof(GLfloat), verticesPlane);

            glVertexAttribPointer(loc, vb.elementSize, GL_FLOAT, false, vb.stride * sizeof(float), (GLvoid*)(vb.offset * sizeof(float)));
            glEnableVertexAttribArray(loc);
            enabledAttribArrayHetro.push_back(loc);
        }
    }
    
    i = 0;
    {
        setUniformBool("IsBackGroundGuide", false);
        setUniformBool("IsDepth", bIsDepth);
        setAllUniforms();

        const Mesh::ElementArrayBuffer& eb = meshHetroObj.elementArrayBuffers[0];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb.elementArrayBufferOpenGL);
        glDrawElements(GL_TRIANGLES, meshHetroObj.elementArrayBuffers[0].elementArrayBuffer.size(), GL_UNSIGNED_INT, NULL /*(GLvoid*) (tg.offset * 3)*/);
        //glDrawElements(GL_TRIANGLES, 30 * 30 * 2 * 3, GL_UNSIGNED_SHORT, indices_in);
    }

    for (int i = 0; i < int(enabledAttribArrayHetro.size()); i++) {
        glDisableVertexAttribArray(enabledAttribArrayHetro[i]);
    }
   
    if (renderToFBO) {
        if(bCaptureing)
        {
            std::vector<unsigned short> pData, pYUVOutput;
            std::vector<unsigned char>  pEntityColor;
            unsigned short* pSrc;
            pData.resize(4 * nHetroBGImageDimWidth * nHetroBGImageDimHeight);
            pYUVOutput.resize(nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 2);
            pEntityColor.resize(nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 2);

            glReadPixels(0, 0, nHetroBGImageDimWidth, nHetroBGImageDimHeight, GL_RGB, GL_UNSIGNED_SHORT, pData.data());

            pSrc = pData.data();

            char strView[64];
            FILE* pFile = NULL;
            FILE* pEntityFile = NULL;

            int nEntityID = -1;

            int nInitValue = 0;
            if (bIsDepth)
            {
                char path[128];
                std::string strGeoFileFormat = strHetroObjOutputPath + strGeoOutput;
                pFile = fopen(strGeoFileFormat.data(), "ab+");
                nInitValue = 0;

                std::string strEntityFileFormat = strHetroObjOutputPath + strEntityOutput;
                pEntityFile = fopen(strEntityFileFormat.data(), "ab+");

            }
            else
            {
                char path[128];
                std::string strTexFileFormat = strHetroObjOutputPath + strTexOutput;
                pFile = fopen(strTexFileFormat.data(), "ab+");
                nInitValue = 512;
            }

            unsigned short* pY = &(pYUVOutput[nHetroBGImageDimWidth * nHetroBGImageDimHeight - 1]);
            unsigned char* EntitypY = &(pEntityColor[nHetroBGImageDimWidth * nHetroBGImageDimHeight - 1]);
            unsigned short* pU = &(pYUVOutput[nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 4 - 1]);
            unsigned short* pV = &(pYUVOutput[nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 2 - 1]);

            for (int i = 0; i < nHetroBGImageDimHeight; i++) {
                unsigned short* pTempSrc = pSrc + ((i + 1) * nHetroBGImageDimWidth * 3 - 1);
                for (int j = 0; j < nHetroBGImageDimWidth; j++, pTempSrc -= 3)
                {
                    unsigned short r = *(pTempSrc - 2);
                    unsigned short g = *(pTempSrc - 1);
                    unsigned short b = *(pTempSrc);

                    float fr = (float)r;
                    float fg = (float)g;
                    float fb = (float)b;

                    if (!bIsDepth)
                    {
                        unsigned short temp = unsigned short(fr * 0.257 + fg * 0.504 + fb * 0.098);
                        float fTemp = ((float)temp) / 65535.0 * 1024.0;
                        if (fTemp == 0.0)
                        {
                            nInitValue = 512;
                        }
                        else
                        {
                            nInitValue = 0;
                        }
                        unsigned short RGB10bit = unsigned short(fTemp + nInitValue);
                        *(pY) = RGB10bit;
                    }

                    else if (bIsDepth)
                    {
                        //*(pY) = unsigned short(fr * 0.257 + fg * 0.504 + fb * 0.098) + nInitValue;
                        *(pY) = unsigned short(fr) + nInitValue;
                        if (*(pY) != 0)
                            *(EntitypY) = 254;
                        else {
                            *(EntitypY) = nInitValue;
                        }
                    }

                    pY--;
                    EntitypY--;

                    if (i % 2 == 0 && j % 2 == 0 && !false)
                    {
                        unsigned short tempu = unsigned short(fr * (-0.147) + fg * (-0.291) + fb * 0.439) + 32767;
                        unsigned short tempv = unsigned short(fr * 0.439 + fg * (-0.368) + fb * (-0.071)) + 32767;

                        float fTempu = ((float)tempu) / 65535.0 * 1024.0;
                        unsigned short RGB10bitu = unsigned short(fTempu);
                        *(pU--) = RGB10bitu;

                        float fTempv = ((float)tempv) / 65535.0 * 1024.0;
                        unsigned short RGB10bitv = unsigned short(fTempv);
                        *(pV--) = RGB10bitv;
                    }
                }
            }


            fwrite(pYUVOutput.data(), (nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 2) * sizeof(unsigned short), 1, pFile);
            if (pEntityFile)
                fwrite(pEntityColor.data(), (nHetroBGImageDimWidth * nHetroBGImageDimHeight + nHetroBGImageDimWidth * nHetroBGImageDimHeight / 2) * sizeof(unsigned char), 1, pEntityFile);

            fclose(pFile);
            if (pEntityFile)
                fclose(pEntityFile);

            pData.clear();
            pYUVOutput.clear();
            pEntityColor.clear();
            bCaptureing = false;

        }
 

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

     if (!renderToFBO)
     {
        float verticesPlane[] = {
            // Front face
            0, 1.0, -1.0,
            0, 1.0,  1.0,
             0,  -1.0,  -1.0,
            0,  -1.0,  1.0
        };

        float texCoordPlane[] = {
            // Front face
            0.0,0.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 1.0
        };


        GLuint planeVbo[1];
        glGenBuffers(1, planeVbo);
        glBindBuffer(GL_ARRAY_BUFFER, planeVbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat) + 2 * 4 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * 4 * sizeof(GLfloat), verticesPlane);
        glBufferSubData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), 2 * 4 * sizeof(GLfloat), texCoordPlane);

        int loc = -1;
        loc = positionAttr;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
        loc = texCoordAttr;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(3 * 4 * sizeof(GLfloat)));

        setUniformBool("IsBackGroundGuide", true);
        //
        setUniformMatrix("meshTransformBackground", backgroundMeshTransformint);
        setUniformMatrix("ProjTransformBackground", backgroundProjTransform);

        UniformVariable& u = uniforms.at("MIVBackgroundGuide");
        glBindTexture(GL_TEXTURE_2D, u.texVal.texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nHetroBGImageDimWidth, nHetroBGImageDimHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image_hetro_background);
        setUniformImage("MIVBackgroundGuide", u.texVal.texID, false);

        setAllUniforms();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}



void ShaderNode::renderBgHDRImage(const Mesh& mesh, const Matrix& background, int width, int height/*, bool wireframe*/, bool renderToFBO)
{
    setUniformInteger("width", width);
    setUniformInteger("height", height);

    if (renderToFBO) {
        resizeFBO(width, height);


        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        //std::vector<unsigned int> renderTar(renderTargets.size() - 1);
        //for (int r = 0; r < int(renderTargets.size() - 1); r++) {
        //    renderTar[r] = GL_COLOR_ATTACHMENT0 + r;
        //}
        //// set the color attachments to write to
        //glDrawBuffers(int(renderTargets.size()) - 1, &renderTar[0]);
    }
    //else {
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //}
    if (background.rows == 4 && background.cols == 1) {
        glClearColor(background.e[0], background.e[1], background.e[2], background.e[3]);
        //glClearColor(1.0, 0.0, 0.0, 1.0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    if (renderOptions.depthtest) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    glEnable(GL_BLEND);
    if (renderOptions.srcFactor >= 0) {
        glBlendFunc(renderOptions.srcFactor, renderOptions.dstFactor);
    }
    if (renderOptions.rgbSrcFactor >= 0) {
        glBlendFuncSeparate(renderOptions.rgbSrcFactor, renderOptions.rbgDstFactor, renderOptions.aSrcFactor, renderOptions.aDstFactor);
    }
    if (renderOptions.cullFace >= 0) {
        glEnable(GL_CULL_FACE);
        glCullFace(renderOptions.cullFace);
    }

    glViewport(0, 0, width, height);

    glUseProgram(progID);

    std::vector <int> enabledAttribArray;
    for (int i = 0; i < int(mesh.vertexBuffers.size()); i++) {
        const Mesh::VertexBuffer& vb = mesh.vertexBuffers[i];
        int loc = -1;
        if (vb.semantic == "position")
            loc = positionAttr;
        if (vb.semantic == "normal")
            loc = normalAttr;
        if (vb.semantic == "texcoord")
            loc = texCoordAttr;
        if (vb.semantic == "color")
            loc = colorAttr;
        if (loc >= 0) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.arrayBuffers[vb.arrayBufferID].arrayBufferOpenGL);
            glVertexAttribPointer(loc, vb.elementSize, GL_FLOAT, false, vb.stride * sizeof(float), (GLvoid*)(vb.offset * sizeof(float)));
            glEnableVertexAttribArray(loc);
            enabledAttribArray.push_back(loc);
        }
    }


    for (int i = 0; i < int(mesh.triangleGroups.size()); i++) {

        setUniformInteger("gsnMeshGroup", i);
        setAllUniforms();

        const Mesh::TriangleGroup& tg = mesh.triangleGroups[i];
        const Mesh::ElementArrayBuffer& eb = mesh.elementArrayBuffers[tg.elementArrayBufferID];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb.elementArrayBufferOpenGL);
        glDrawElements(GL_TRIANGLES, tg.noOfTriangles * 3, GL_UNSIGNED_INT, NULL /*(GLvoid*) (tg.offset * 3)*/);
    }

    for (int i = 0; i < int(enabledAttribArray.size()); i++) {
        glDisableVertexAttribArray(enabledAttribArray[i]);
    }


    //if (renderToFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //}


    for (int r = 0; r < int(renderTargets.size()); r++) {
        RenderTarget& renderTarget = renderTargets[r];
        if (renderTarget.texID > 0) {
            glBindTexture(GL_TEXTURE_2D, renderTarget.texID);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void ShaderNode::render(const Mesh& mesh, const Matrix& background, int width, int height/*, bool wireframe*/, bool renderToFBO)
{
  setUniformInteger("width", width);
  setUniformInteger("height", height);

  if (renderToFBO) {
    resizeFBO(width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<unsigned int> renderTar(renderTargets.size() - 1);
    for (int r = 0; r < int(renderTargets.size() - 1); r++) {
      renderTar[r] = GL_COLOR_ATTACHMENT0 + r;
    }
    // set the color attachments to write to
    glDrawBuffers(int(renderTargets.size()) - 1, &renderTar[0]);
  }
  else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  if (background.rows == 4 && background.cols == 1) {
    glClearColor(background.e[0], background.e[1], background.e[2], background.e[3]);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);

  if (renderOptions.depthtest) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }

  glEnable(GL_BLEND);
  if (renderOptions.srcFactor >= 0) {
    glBlendFunc(renderOptions.srcFactor, renderOptions.dstFactor);
  }
  if (renderOptions.rgbSrcFactor >= 0) {
    glBlendFuncSeparate(renderOptions.rgbSrcFactor, renderOptions.rbgDstFactor, renderOptions.aSrcFactor, renderOptions.aDstFactor);
  }
  if (renderOptions.cullFace >= 0) {
    glEnable(GL_CULL_FACE);
    glCullFace(renderOptions.cullFace);
  }

  glViewport(0, 0, width, height);

  glUseProgram(progID);

  std::vector <int> enabledAttribArray;
  for (int i = 0; i < int(mesh.vertexBuffers.size()); i++) {
    const Mesh::VertexBuffer& vb = mesh.vertexBuffers[i];
    int loc = -1;
    if (vb.semantic == "position")
      loc = positionAttr;
    if (vb.semantic == "normal")
      loc = normalAttr;
    if (vb.semantic == "texcoord")
      loc = texCoordAttr;
    if (vb.semantic == "color")
      loc = colorAttr;
    if (loc >= 0) {
      glBindBuffer(GL_ARRAY_BUFFER, mesh.arrayBuffers[vb.arrayBufferID].arrayBufferOpenGL);
      glVertexAttribPointer(loc, vb.elementSize, GL_FLOAT, false, vb.stride * sizeof(float), (GLvoid *) (vb.offset * sizeof(float)));
      glEnableVertexAttribArray(loc);
      enabledAttribArray.push_back(loc);
    }
  }

  
  for (int i = 0; i < int(mesh.triangleGroups.size()); i++) {

    setUniformInteger("gsnMeshGroup", i);
    setAllUniforms();

    const Mesh::TriangleGroup& tg = mesh.triangleGroups[i];
    const Mesh::ElementArrayBuffer& eb = mesh.elementArrayBuffers[tg.elementArrayBufferID];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb.elementArrayBufferOpenGL);
    glDrawElements(GL_TRIANGLES, tg.noOfTriangles * 3, GL_UNSIGNED_INT, NULL /*(GLvoid*) (tg.offset * 3)*/);
  }

  for (int i = 0; i < int(enabledAttribArray.size()); i++) {
    glDisableVertexAttribArray(enabledAttribArray[i]);
  }

  if (renderToFBO) {

  }

  if (renderToFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  for (int r = 0; r < int(renderTargets.size()); r++) {
    RenderTarget& renderTarget = renderTargets[r];
    if (renderTarget.texID > 0) {
      glBindTexture(GL_TEXTURE_2D, renderTarget.texID);
      glGenerateMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

}

void ShaderNode::setUniformInteger(const std::string& name, int value) 
{
  if (uniforms.count(name) > 0) {
    if (uniforms.at(name).type == 0) {
      uniforms.at(name).intVal = value;
    }
  }
}

void ShaderNode::setUniformFloat(const std::string& name, float value)
{
  if (uniforms.count(name) > 0) {
    if (uniforms.at(name).type == 1) {
      uniforms.at(name).floatVal = value;
    }
  }
}

void ShaderNode::setUniformBool(const std::string& name, bool value)
{
  if (uniforms.count(name) > 0) {
    if (uniforms.at(name).type == 2) {
      uniforms.at(name).boolVal = value;
    }
  }
}

void ShaderNode::setUniformMatrix(const std::string& name, const Matrix& value)
{
  if (uniforms.count(name) > 0) {
    if (uniforms.at(name).type == 3) {
      uniforms.at(name).matVal = value;
    }
  }
  if (uniforms.count(name + "Inverse") > 0) {
    if (uniforms.at(name + "Inverse").type == 3) {
      uniforms.at(name + "Inverse").matVal = value.invert();
    }
  }
  if (uniforms.count(name + "TransposedInverse") > 0) {
    if (uniforms.at(name + "TransposedInverse").type == 3) {
      uniforms.at(name + "TransposedInverse").matVal = value.transpose().invert();
    }
  }
  if (uniforms.count(name + "InverseTransposed") > 0) { 
    if (uniforms.at(name + "InverseTransposed").type == 3) {
      uniforms.at(name + "InverseTransposed").matVal = value.invert().transpose();
    }
  }
}

void ShaderNode::setUniformImage(const std::string& name, unsigned int textureID, bool underExternalControl) {
  if (uniforms.count(name) > 0) {
    UniformVariable& u = uniforms.at(name);
    if (u.type == 4) {
      u.texVal.texID = textureID;
      u.texVal.underExternalControl = underExternalControl;
    }
  }
  if (uniforms.count(name + "Width") > 0) {
    UniformVariable& u = uniforms.at(name + "Width");
    if (u.type == 0) {
      int w;
      glBindTexture(GL_TEXTURE_2D, textureID);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
      glBindTexture(GL_TEXTURE_2D, 0);
      u.intVal = w;
    }
  }
  if (uniforms.count(name + "Height") > 0) {
    UniformVariable& u = uniforms.at(name + "Height");
    if (u.type == 0) {
      int h;
      glBindTexture(GL_TEXTURE_2D, textureID);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
      glBindTexture(GL_TEXTURE_2D, 0);
      u.intVal = h;
    }
  }
}

void ShaderNode::setUniformsFromFile(const std::string& filename, const std::string& filename2) {
  std::string content = FileTools::readTextFile(filename);
  const std::vector <string> lines = StringTools::split(content, "\n");
  for (int i = 0; i < int(lines.size()); i++) {
    const std::string& line = lines[i];
    const std::vector <string> items = StringTools::split(line, ",");
    if (items.size() >= 2) {
      string name = items[0];
      string type = StringTools::toLower(items[1]);
      if (type == "integer" && items.size() >= 3) {
        int val = StringTools::stringToInt(items[2]);
        setUniformInteger(name, val);
      }
      if (type == "float" && items.size() >= 3) {
        float val = StringTools::stringToFloat(items[2]);
        setUniformFloat(name, val);
      }
      if (type == "boolean" && items.size() >= 3) {
        bool val = StringTools::stringToBool(items[2]);
         setUniformBool(name, val);
      }
      if (type == "color" && items.size() >= 6) {
        float x = StringTools::stringToFloat(items[2]);
        float y = StringTools::stringToFloat(items[3]);
        float z = StringTools::stringToFloat(items[4]);
        float w = StringTools::stringToFloat(items[5]);
        Matrix mat(x, y, z, w);
        setUniformMatrix(name, mat);
      }
      if (type == "matrix" && items.size() >= 4) {
        int rows = StringTools::stringToInt(items[2]);
        int cols = StringTools::stringToInt(items[3]);
        Matrix mat(rows, cols);
        if (int(items.size()) >= 4 + (rows * cols)) {
          for (int j = 0; j < int(mat.e.size()); j++) {
            mat.e[j] = StringTools::stringToFloat(items[4 + j]);
          }
          setUniformMatrix(name, mat);
        }
      }
      if (type == "image") {
          string imgFileName = FileTools::getDirectory(filename) + "/" + name + ".pfm";
          std::vector<float> data;
          int width = 0;
          int height = 0;
          bool bReadFileFail = FileTools::loadAlphaPFM(imgFileName, width, height, data);
          
          if (bReadFileFail == false)
          {
             string imgFileName = FileTools::getDirectory(filename) + "/" + name + ".raw";
             
             std::vector<float> _data;
             
             int width = 1024;
             int height = 512;
             
             _data.resize(width * height * 4);

             FILE* file = fopen(imgFileName.data(), "rb");
             if (file == NULL)
             {
                 continue;
             }

            // fclose(file);

            // FileTools::YUVToRGBTexFile(imgFileName, _data, width, height, true);
             if (imgFileName != filename2)
             {
                 fclose(file);
                 file = fopen(filename2.data(), "rb");

             }

             fread(&_data[0], sizeof(GLfloat), width * height * 4, file);
             GLfloat* pixelscpy = new GLfloat[width * height * 4];
             int nTestCount = 0;
             for (int i = height - 1; i >= 0; i--) {

                 GLfloat* pTempSrc = &_data[i * width * 4];
                 for (int j = 0; j < width; j++, pTempSrc += 4)
                 {
                     GLfloat r = *(pTempSrc);
                     GLfloat g = *(pTempSrc + 1);
                     GLfloat b = *(pTempSrc + 2);
                     GLfloat a = *(pTempSrc + 3);

                     pixelscpy[nTestCount] = r; nTestCount++;
                     pixelscpy[nTestCount] = g; nTestCount++;
                     pixelscpy[nTestCount] = b; nTestCount++;
                     pixelscpy[nTestCount] = a; nTestCount++;
                 }
             }

             GLuint textureID;
             glGenTextures(1, &textureID);
             glBindTexture(GL_TEXTURE_2D, textureID);
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &pixelscpy[0]);

             glGenerateMipmap(GL_TEXTURE_2D);

             glBindTexture(GL_TEXTURE_2D, 0);
             setUniformImage("MyTex", textureID, false);

             delete[] pixelscpy;
             _data.clear();

             continue;
          }

          //request textureID
          GLuint textureID;
          glGenTextures(1, &textureID);

          // bind texture
          glBindTexture(GL_TEXTURE_2D, textureID);
          // specify the 2D texture map
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &data[0]);
          glGenerateMipmap(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, 0);
          setUniformImage(name, textureID, false);
      }
    }
  }
}

unsigned ShaderNode::getRenderTarget(int index) const {
  int i = index % int(renderTargets.size());
  return renderTargets[i].texID;
}

unsigned ShaderNode::getRenderTargetByName(const std::string& name) const {
  for (int i = 0; i < int(renderTargets.size()); i++) {
    if (renderTargets[i].name == name) {
      return renderTargets[i].texID;
    }
  }

  return -1;
}
