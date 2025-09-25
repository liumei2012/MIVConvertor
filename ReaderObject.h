#pragma once


#ifndef _OBJECT_READER_H_
#define _OBJECT_READER_H_

#include "vMeshReaderDef.h"

enum ObjectType { POINTCLOUD = 0, MESH = 1 };

class Box {
 public:
  Box() {
    m_eMin = Vec3( ( std::numeric_limits<float>::max )(), ( std::numeric_limits<float>::max )(),
                   ( std::numeric_limits<float>::max )() );
    m_eMax = Vec3( -( std::numeric_limits<float>::max )(), -( std::numeric_limits<float>::max )(),
                   -( std::numeric_limits<float>::max )() );
  }
  Box( Vec3 eMin, Vec3 eMax ) : m_eMin( eMin ), m_eMax( eMax ) {}
  inline Vec3& min() { return m_eMin; }
  inline Vec3& max() { return m_eMax; }
  inline Vec3  center() { return m_eMin + ( m_eMax - m_eMin ) / 2.f; }
  inline Vec3  size() { return m_eMax - m_eMin; }
  inline float getMaxSize() {
    auto maxSize = m_eMax - m_eMin;
    return ( std::max )( maxSize[0], ( std::max )( maxSize[1], maxSize[2] ) );
  }
  void update( const Box& eBox ) {
    for ( uint8_t i = 0; i < 3; i++ ) {
      if ( m_eMin[i] > eBox.m_eMin[i] ) { m_eMin[i] = eBox.m_eMin[i]; }
      if ( m_eMax[i] < eBox.m_eMax[i] ) { m_eMax[i] = eBox.m_eMax[i]; }
    }
  }
  void update( const Vec3& point ) {
    for ( uint8_t i = 0; i < 3; i++ ) {
      if ( m_eMin[i] > point[i] ) { m_eMin[i] = point[i]; }
      if ( m_eMax[i] < point[i] ) { m_eMax[i] = point[i]; }
    }
  }

 private:
  Vec3 m_eMin;
  Vec3 m_eMax;
};

class Object {
 public:

  Object() {}
  ~Object() {}

  virtual ObjectType         getType()                                     = 0;
  virtual void               load()                                        = 0;


  virtual void               recomputeBoundingBox() = 0;
  virtual Object*               getGeometry()                        = 0;
  virtual void               center( Box box, float fBoxSize )             = 0;
  virtual void               scale( Box box, float fBoxSize )              = 0;

  inline int                 getFrameIndex() { return m_nFrameIndex; }
  inline Box&                getBox() { return m_hBox; }


  const std::string& getFilename() { return m_rFilename; }




 protected:
  int         m_nFrameIndex       = 0;
  bool        m_bLoad             = false;
  Box         m_hBox;
  std::string m_rFilename = "";

};

#endif  
