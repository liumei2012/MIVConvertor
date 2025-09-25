
#ifndef _SEQUENCE_READER_H_
#define _SEQUENCE_READER_H_

#include "vMeshReaderDef.h"
#include "ReaderObject.h"

class Sequence {
 public:

  Sequence();
  ~Sequence();

  inline int                getFrameIndex() { return m_nFrameIndex; }
  Object&                   getObject();

  inline Box&               getBox() { return m_hBox; }
  const std::string&        getFilename();

  float                     getBoxSize() { return m_fBoxSize; }

  void                      setFrameIndex( int32_t iFrameIndex ) { m_nFrameIndex = iFrameIndex; }
  void                      load();

  void readFile( const std::string& sFile, int iFrameIndex, int iFrameNumber, bool eBinary, bool bSource = false );
  void normalize( int32_t iScaleMode, bool bCenter );
  
  void recomputeBoundingBox();
  int  getNumFrames() { return (int)(  (int)m_eObject.size() ); }
  
  void setBoxSize( float fBoxSize ) { m_fBoxSize = fBoxSize == 0 ? m_hBox.getMaxSize() : fBoxSize; }
  
 private:
  void add( std::shared_ptr<Object> pObject, bool bSource );

public:

  int                                   m_nFrameIndex    = 0;
  float                                 m_fBoxSize       = 1024.0f;
  std::vector<std::shared_ptr<Object> > m_eObject;
  Box                                   m_hBox;
};
#endif  
