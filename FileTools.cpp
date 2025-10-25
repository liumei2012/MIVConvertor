// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "FileTools.h"

using namespace std;
using namespace gsn;

#define CLIP(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

bool FileTools::fileExists(const std::string& filename)
{
  ifstream myfile(filename.c_str());
  if (!myfile.is_open()) {
    return false;
  }
  myfile.close();
  return true;
}

std::string FileTools::findFile(const std::string& filename, int depth)
{
  int counter = 0;
  std::string path("");

  while (counter < depth) {
    if (FileTools::fileExists(path + filename)) return path + filename;
    path += "../";
    counter++;
  }
  return filename;
}

std::string FileTools::readTextFile(const std::string& filename) {
  std::ifstream is(filename);
  if (is.is_open()) {
    std::stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
  }
  cerr << "Unable to open file " << filename << endl;
  exit(1);
}

std::string FileTools::getDirectory(const std::string& path) {
  size_t pos = path.find_last_of("\\/");
  if (pos != std::string::npos) {
    return path.substr(0, pos);
  }
  return path;
 }

std::string FileTools::getFileName(const std::string& path) {
  size_t pos = path.find_last_of("\\/");
  if (pos != std::string::npos) {
    return path.substr(pos + 1);
  }
  return path;
}


namespace gsn {
  void getNextStringPNMHeader(FILE* fp, char* line) // gets next string ignoring comments marked by "#"
  {
    int i;
    line[0] = '\0';
    while (line[0] == '\0') {
      fscanf(fp, "%s", line);
      i = -1;
      do {
        i++;
        if (line[i] == '#') {
          line[i] = '\0';
          while (fgetc(fp) != '\n');
        }
      } while (line[i] != '\0');
    }
  }
}

bool FileTools::loadPFM(const std::string& filename, int& width, int& height, int& channels, std::vector<float>& data)
{
  const int identiferLength = 1000;
  char identifer[identiferLength];

  FILE* file = fopen(filename.c_str(), "rb");

  if (file == NULL) {
    //fprintf(stderr, "ERROR: [FileTools::loadPFM] could not open file %s", filename.c_str());
    return false;
  }

  // read "magic number" for identifying the file type.
  char letter;
  char letter2;
  getNextStringPNMHeader(file, identifer);
  sscanf(identifer, "%c%c", &letter, &letter2);

  if (letter != 'P') {
    //fprintf(stderr, "ERROR: [LoadPFG::load] The input data is not PFM.\n");
    fclose(file);
    return false;
  }

  bool found = false;

  if (letter2 == 'f') {
    channels = 1;
    found = true;
  }
  if (letter2 == 'F') {
    channels = 3;
    found = true;
  }
  if (letter2 == 'g') {
    sscanf(identifer, "%c%c%d", &letter, &letter2, &channels);
    if (channels > 1 && channels < 256) {
      found = true;
    }
  }

  if (!found) {
    //fprintf(stderr, "ERROR: [LoadPFG::load] The input data is not PFM.\n");
    fclose(file);
    return false;
  }

  // read width and height of the image
  getNextStringPNMHeader(file, identifer);
  sscanf(identifer, "%d", &width);
  getNextStringPNMHeader(file, identifer);
  sscanf(identifer, "%d", &height);
  float byteOrder;
  getNextStringPNMHeader(file, identifer);
  sscanf(identifer, "%f", &byteOrder);

  if (byteOrder >= 0.0) {
    //fprintf(stderr, "ERROR: [FileTools::loadPFM] only little-endian byte order is supported\n");
    fclose(file);
    return false;
  }

  // A single whitespace character (usually a newline). 
  fgetc(file);

  data.resize(width * height * channels);
  fread(&data[0], sizeof(float), width * height * channels, file);
  
  float pixelmax = -1e37f;
  float pixelmin = 1e37f;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      for (int z = 0; z < channels; z++) {
        float value = data[y * width * channels + x * channels + z];
        if (pixelmax < value) pixelmax = value;
        if (pixelmin > value) pixelmin = value;
      }
    }
  }

  //cout << "FileTools::loadPFM: " << filename << " Info: width=" << width << " height=" << height << " channels=" << channels << " min=" << pixelmin << " max=" << pixelmax << endl; 
  fclose(file);
  return true;
}

bool FileTools::loadAlphaPFM(const std::string& filename, int& width, int& height, std::vector<float>& data)
{
  size_t pos = filename.find_last_of(".");
  string filenameAlpha;
  if (pos != std::string::npos) {
    filenameAlpha = filename.substr(0, pos) + "_alpha" + filename.substr(pos);
  }
  int widthRGB = 0;
  int heightRGB = 0;
  int channelsRGB = 0;
  std::vector <float> dataRGB;
  bool ret = FileTools::loadPFM(filename, widthRGB, heightRGB, channelsRGB, dataRGB);
  if (!ret) {
    return false;
  }
  int widthAlpha = 0;
  int heightAlpha = 0;
  int channelsAlpha = 0;
  std::vector <float> dataAlpha;
  ret = FileTools::loadPFM(filenameAlpha, widthAlpha, heightAlpha, channelsAlpha, dataAlpha);
  if (!ret) {
    return false;
  }
  if (widthRGB != widthAlpha || heightRGB != heightAlpha || channelsRGB != 3 || channelsAlpha < 1) {
    return false;
  }
  width = widthRGB;
  height = heightRGB;
  int noOfPixels = width * height;
  data.resize(noOfPixels * 4);
  int countRGB = 0;
  int countAlpha = 0;
  for (int i = 0; i < noOfPixels; i++) {
    data[i * 4 + 0] = dataRGB[countRGB++];
    data[i * 4 + 1] = dataRGB[countRGB++];
    data[i * 4 + 2] = dataRGB[countRGB++];
    data[i * 4 + 3] = dataAlpha[countAlpha++];
  }

  return true;
}

int FileTools::ReadYUV(std::string pStrFile, char*& buffer, int nFilePosition, int nWidht, int nHeight)
{
    //int length = 12582912;
    int length = nWidht * nHeight * 2 + nWidht * nHeight ;
    std::ifstream is(pStrFile, std::ifstream::binary);
    if (is) {

        buffer = new char[length];
        unsigned long int n = nFilePosition * length;
        is.seekg(n);
        is.read(buffer, length);
        is.seekg(0, is.beg);
        is.close();

        // ...buffer contains the entire file...
    }
    else
    {
        //std::cout << "Fail to read YUV source" << std::endl;
    }

    return length;
}

void FileTools::YUVToGeoTex(std::string strYUVPath, 
    std::string strPostfixTex, 
    std::string strBitDepth, 
    int nNoofView, 
    std::vector<unsigned short>& data, 
    int nTexWidth, int nTexHeight)
{
    char* buffer = NULL;
    unsigned short* pShortBuf = NULL;

    std::string strFilename = strYUVPath + "v" + std::to_string(nNoofView) + strPostfixTex + strBitDepth + ".yuv";
    int nFileSize = ReadYUV(strFilename, buffer, 0, nTexWidth, nTexHeight);

    pShortBuf = (unsigned short*)buffer;
    data.resize(nTexWidth * nTexHeight);

    TextureUpDown(pShortBuf, data, nTexWidth, nTexHeight);

    delete buffer;
    buffer = NULL;
    pShortBuf = NULL;


}

void FileTools::TextureUpDown(unsigned short* pShortBuf, std::vector<unsigned short>& data, int nTexWidth, int nTexHeight)
{
    std::vector<unsigned short> TempData;
    TempData.resize(nTexWidth * nTexHeight);

    int nNewIndex = 0;

    for (int j = nTexHeight - 1; j >= 0; j--)
    {
        for (int i = 0; i < nTexWidth; i++)
        {
            int nindex = (i + j * nTexWidth);

            TempData[nNewIndex] = pShortBuf[nindex];
            nNewIndex++;
        }
    }

    for (int i = 0; i < nNewIndex; i++)
    {
        data[i] = TempData[i];
    }

    TempData.clear();
}

void FileTools::YUVToRGBTex(std::string strYUVPath, 
    std::string strPostfixTex, 
    std::string strBitDepth, 
    int nNoofView, 
    std::vector<unsigned char>& data, 
    int nTexWidth, int nTexHeight, bool bPointCloudConversion)
{
    const int nWidth = nTexWidth;
    const int nHeight = nTexHeight;

    int nImageWidth = nWidth;
    int nImageHeight = nHeight;

    std::vector<unsigned short> usYPlaneVec;
    std::vector<unsigned short> usUVPlaneVec[2];
    std::vector<unsigned short> TempShortBufVec;
    
    usYPlaneVec.resize(nImageWidth * nImageHeight);
    usUVPlaneVec[0].resize(nImageWidth / 2 * nImageHeight / 2);
    usUVPlaneVec[1].resize(nImageWidth / 2 * nImageHeight / 2);
    TempShortBufVec.resize(nImageWidth * nImageHeight + nImageWidth * nImageHeight / 2);

    char* buffer = NULL;
    data.resize(nWidth * nHeight * 3);

    std::string strFilename = strYUVPath + "v" + std::to_string(nNoofView) + strPostfixTex + strBitDepth + ".yuv";
    int nFileSize = ReadYUV(strFilename, buffer, 0, nWidth, nHeight);
    memcpy(&TempShortBufVec[0], buffer, nFileSize);




    {
        for (int i = 0; i < nImageHeight; i++)
        {
            for (int j = 0; j < nImageWidth; j++)
            {
                usYPlaneVec[i * nWidth + j] = TempShortBufVec[nWidth * i + j];
            }
        }


        nImageWidth /= 2;
        nImageHeight /= 2;

        for (int i = 0; i < nImageHeight; i++)
        {
            for (int j = 0; j < nImageWidth; j++)
            {
                {
                    usUVPlaneVec[0][i * nImageWidth + j] = TempShortBufVec[nImageHeight * i + j + nWidth * nHeight];
                    usUVPlaneVec[1][i * nImageWidth + j] = TempShortBufVec[nImageHeight * i + j + (nWidth /2 * nHeight /2) + nWidth * nHeight];
                }
            }
        }
    }


    unsigned short* pShortBuf = &usYPlaneVec[0];
    std::vector<unsigned short> usYPlaneVec_;
    usYPlaneVec_.resize(usYPlaneVec.size());
    if (bPointCloudConversion) {
        TextureUpDown(pShortBuf, usYPlaneVec_, nWidth, nHeight);
    }
    else {
        std::copy(usYPlaneVec.begin(), usYPlaneVec.end(), usYPlaneVec_.begin());
    }

    pShortBuf = &usUVPlaneVec[0][0];
    std::vector<unsigned short> usUPlaneVec_;
    usUPlaneVec_.resize(usUVPlaneVec[0].size());
    if (bPointCloudConversion) {
        TextureUpDown(pShortBuf, usUPlaneVec_, nImageWidth, nImageHeight);
    }
    else {
        std::copy(usUVPlaneVec[0].begin(), usUVPlaneVec[0].end(), usUPlaneVec_.begin());
    }

    pShortBuf = &usUVPlaneVec[1][0];
    std::vector<unsigned short> usVPlaneVec_;
    usVPlaneVec_.resize(usUVPlaneVec[1].size());
    if (bPointCloudConversion) {
        TextureUpDown(pShortBuf, usVPlaneVec_, nImageWidth, nImageHeight);
    }
    else {
        std::copy(usUVPlaneVec[1].begin(), usUVPlaneVec[1].end(), usVPlaneVec_.begin());
    }

    nImageWidth *= 2;
    nImageHeight *= 2;

    int nRGBIndex = 0;

    for (int i = 0; i < nImageHeight; i++)
    {
        for (int j = 0; j < nImageWidth; j++)
        {
            float Y = float(usYPlaneVec_[i * nImageWidth + j]) / 1024.0 * 256.0;
            float U = float(usUPlaneVec_[i / 2 * nImageWidth / 2 + j / 2]) / 1024.0 * 256.0;
            float V = float(usVPlaneVec_[i / 2 * nImageWidth / 2 + j / 2]) / 1024.0 * 256.0;

            float fR = 1.164 * (Y - 16.0) + 2.018 * (U - 128.0);
            float fG = 1.164 * (Y - 16.0) - 0.391 * (U - 128.0) - 0.813 * (V - 128.0);
            float fB = 1.164 * (Y - 16.0) + 1.596 * (V - 128.0);

            unsigned char R = unsigned char(CLIP(fR));
            unsigned char G = unsigned char(CLIP(fG));
            unsigned char B = unsigned char(CLIP(fB));

            data[nRGBIndex++] = B;
            data[nRGBIndex++] = G;
            data[nRGBIndex++] = R;
        }
    }

    delete buffer;
    buffer = NULL;


}