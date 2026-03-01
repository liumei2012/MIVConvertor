// Copyright (C) Thorsten Thormaehlen, MIT License (see license file)

#ifndef DEF_LOADOBJ_H
#define DEF_LOADOBJ_H

#include <string>
#include <vector>
#include "Mesh.h"

namespace gsn {
  /*!
  \class LoadOBJ LoadOBJ.h
  \brief This class provides functions for reading meshes from the OBJ 3D file format
  */

  class LoadOBJ {

  public:
    //! loads the OBJ mesh from a given filename
    static bool load(std::string filename, Mesh& mesh);
    static bool loadHetro(std::vector<float>& verts, std::vector<float>& normals, std::vector<float>& texCoords, std::vector<unsigned int>& indices, Mesh& mesh);

  };

}

#endif
