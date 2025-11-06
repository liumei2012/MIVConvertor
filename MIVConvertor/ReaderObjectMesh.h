

#define _OBJECT_MESH_RENDERER_APP_H_

#include "vMeshReaderDef.h"
#include "ReaderObject.h"

struct Vertex {
  Vec3 position_;
  Vec4 color_;
  Vec2 texCoords_;
  Vec3 normal_;
};

struct Texture {
  unsigned int               id_;
  std::string          type_;
  std::string          path_;
  std::vector<uint8_t> data_;
  int32_t              width_;
  int32_t              height_;




};

class Mesh {
 public:
  Mesh();

  ~Mesh();

  void                  load();
  void                  unload();

  std::vector<Vertex>&  getVertices() { return m_Vertices; }
  std::vector<unsigned int>&  getIndices() { return m_Indices; }
  std::vector<Texture>& getTextures() { return m_Texture; }
  Vertex&               getVertex( size_t i ) { return m_Vertices[i]; }
  unsigned int&               getIndice( size_t i ) { return m_Indices[i]; }
  Texture&              getTexture( size_t i ) { return m_Texture[i]; }
  const Box&            getBox() { return m_hBox; }
  void                  recomputeBoundingBox();
  void                  center( Vec3 center );
  void                  scale( Vec3 center, float fScale );
  bool                  getUseColorPerVertex() { return m_bUseColorPerVertex; }
  void                  setUseColorPerVertex( bool bValue ) { m_bUseColorPerVertex = bValue; }
  size_t                getNumberOfVertices() { return m_Vertices.size(); }
  size_t                getNumberOfFaces() { return m_Indices.size() / 3; }
  void                  computeFaceNormals( bool normalize = true );
  void                  computeVertexNormals( bool normalize = true, bool noSeams = true );
  //void                  createBox( Box& box, Color4& eColor );

 private:
  inline void normalizeNormal( Vec3& normal ) const {
    normal = glm::normalize( normal );
    if ( std::isnan( normal[0] ) ) { normal = glm::vec3( 0.0F, 0.0F, 1.0F ); }
  }
  std::vector<Vertex>  m_Vertices;
  std::vector<Vec3>    m_FaceNormals;
  std::vector<unsigned int>  m_Indices;
  std::vector<Texture> m_Texture;

  Box                  m_hBox;
  bool                 m_bLoad;
  bool                 m_bUseColorPerVertex;
};

class vMeshObj : public Object {
 public:
  vMeshObj();
  ~vMeshObj();
  ObjectType         getType() { return ObjectType::MESH; }
  void               load();


  void               recomputeBoundingBox();
  vMeshObj* getGeometry(){ return this; };
  void               center( Box box, float fBoxSize );
  void               scale( Box box, float fBoxSize );
  bool               read( std::string path, int32_t framesIndex );
  std::vector<Mesh>& getMeshes() { return m_Meshes; }

  void               computeVertexNormals();

 public:
  bool                        readObj( std::string path, int32_t framesIndex );

  void                        readTextures( const std::string& name, std::string type, std::vector<Texture>& textures );
  std::vector<Mesh>           m_Meshes;
  std::string                 m_rDirectory;
  int32_t                     m_nNumVertices = 0;
  int32_t                     m_nNumFaces    = 0;
  int32_t                     m_nNumTextures = 0;
};


