
#include "vMeshReadSequence.h"
#include "ReaderObjectMesh.h"

Sequence::Sequence() {}
Sequence::~Sequence() {
  m_eObject.clear();
  
 // m_pTypeName.clear();
}

Object& Sequence::getObject() {
  auto& eObject   =  m_eObject;
  int   iNumFrame = getNumFrames();
  if ( m_nFrameIndex < 0 && m_nFrameIndex > iNumFrame ) {
    printf( "Error: can't load object of frame index = %d is not in [%d;%d] \n", m_nFrameIndex, 0, iNumFrame );
    exit( -1 );
  }
  return *eObject[m_nFrameIndex];
}



const std::string& Sequence::getFilename() { return getObject().getFilename(); }
void               Sequence::load() { getObject().load(); }



void Sequence::normalize( int32_t iScaleMode, bool bCenter ) {
	
  if ( iScaleMode != 0 ) {
    for ( auto& eObject : m_eObject ) { eObject->scale( iScaleMode == 1 ? eObject->getBox() : m_hBox, m_fBoxSize ); }
    recomputeBoundingBox(); 
  }
}



void Sequence::recomputeBoundingBox() {
  if ( static_cast<int>( m_eObject.size() ) > 0 ) {
    m_hBox = Box();
    for ( auto& pObject : m_eObject ) {
      pObject->recomputeBoundingBox();
      m_hBox.update( pObject->getBox() );
    }
  }
}

void Sequence::add( std::shared_ptr<Object> pObject, bool bSource ) {
  m_hBox.update( pObject->getBox() );
  (  m_eObject ).push_back( pObject );
}


#include <windows.h>



void Sequence::readFile( const std::string& sFile, int iFrameIndex, int iFrameNumber, bool eBinaryFile, bool bSource ) {
  if ( !sFile.empty() ) {
    std::string sExtension = getExtension( sFile );
    auto &eObject =  m_eObject;

    if ( sExtension == "obj" || ( sExtension == "ply" && eObject.size() == 0 ) ) {
      int  iNumRead  = 0;
      bool bReadDone = false;
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = std::make_shared<vMeshObj>();
        eObject.push_back( pObject );
      }
#pragma omp parallel for
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = (std::dynamic_pointer_cast<vMeshObj>)( eObject[i] );
        if ( pObject->read( sFile, iFrameIndex + i ) ) {
          bReadDone = true;
          //PROGRESSBAR( iNumRead, iFrameNumber, "Read Ply files %3d", iFrameIndex + iNumRead );
          iNumRead++;
        }
      }
      if ( !bReadDone ) { eObject.clear(); }
    }
    for ( auto& element : eObject ) { m_hBox.update( element->getBox() ); }

  }
}

