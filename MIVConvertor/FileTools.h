// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#ifndef DEF_FILETOOLS_H
#define DEF_FILETOOLS_H

#include <string>
#include <vector>
#include <iostream> 

#define IMAGEWIDTH 2048
#define IMAGEHEIGHT 2048

namespace gsn {
  /*!
  \class FileTools FileToolss.h
  \brief This class holds a collection of useful functions for files.
  */

  class FileTools {



  public:

    //! returns true if a file exists
    static bool fileExists(const std::string& filename);

    //! finds a file relative to the directory in which the application is started
    static std::string findFile(const std::string& filename, int depth = 5);

    //! returns the directory from a path
    static std::string getDirectory(const std::string& path);

    //! returns the file name from a path
    static std::string getFileName(const std::string& path);

    //! reads the file content as a std::string (exits if file can not be read)
    static std::string readTextFile(const std::string& filename);

    //! loads a PFM (PortableFloatMap) file
    static bool loadPFM(const std::string& filename, int& width, int& height, int& channels, std::vector<float>& data);

    //! loads a PFM (PortableFloatMap) file with a separate alpha channel PFM file
    static bool loadAlphaPFM(const std::string& filename, int& width, int& height, std::vector<float>& data);

    static void TextureUpDown(unsigned short* pShortBuf, std::vector<unsigned short>& data, int nTexWidth, int nTexHeight);

    static int ReadYUV(std::string pStrFile, char*& buffer, int nFilePosition, int nWidht, int nHeight);
    static void YUVToRGBTex(std::string strYUVPath, 
        std::string strPostfixTex,
        std::string strBitDepth, 
        int nNoofView, std::vector<unsigned char>& data, 
        int nTexWidth, int nTexHeight, bool bPointCloudConversion);

    static void YUVToRGBTexFile(
        std::string strFile,
        std::vector<unsigned char>& data,
        int nTexWidth, int nTexHeight, bool bPointCloudConversion);

    static void YUVToGeoTex(std::string strYUVPath, 
        std::string strPostfixTex, 
        std::string strBitDepth, 
        int nNoofView, 
        std::vector<unsigned short>& data, 
        int nTexWidth, int nTexHeight);

  };

}

#endif
