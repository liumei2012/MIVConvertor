

#ifndef _PARAMETERS_vMesh_
#define _PARAMETERS_vMesh_

#include "vMeshReaderDef.h"

class vMeshParameters {
 public:
  vMeshParameters();
  virtual ~vMeshParameters();

  inline std::string getFile() const { return m_pFile; }
 
  inline int         getFrameNumber() const { return m_nFrameNumber; }
  inline int         getFrameIndex() const { return m_nFrameIndex; }
  inline int         getBoxSize() const { return m_iBoxSize; }
  inline bool        getCenter() const { return m_bCenter; }
  inline bool        getBinFile() const { return m_bCreateBinaryFiles; }

  std::string m_pFile;

  int         m_nFrameNumber;
  int         m_nFrameIndex;
  int         m_iBoxSize;
  bool        m_bCenter;
  bool        m_bCreateBinaryFiles;
  
};

#endif 
