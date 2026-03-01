

#include "ReaderObjectMesh.h"
#include "tinyply.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


Mesh::Mesh() : m_bLoad( false ), m_bUseColorPerVertex( true ) {}
Mesh::~Mesh() {
  m_Vertices.clear();
  m_Indices.clear();
  m_Texture.clear();
  m_FaceNormals.clear();
}


void Mesh::load() {
  if ( !m_bLoad ) {
 
    m_bLoad = true;
  }
}

void Mesh::unload() {
  if ( m_bLoad ) {
    m_bLoad = false;
  }
}

void Mesh::scale( Vec3 center, float scale ) {
  for ( auto& point : m_Vertices ) { point.position_ = ( point.position_ - center ) * scale; }
}

void Mesh::center( Vec3 center ) {
  for ( auto& ePoint : m_Vertices ) { ePoint.position_ += center; }
}

void Mesh::recomputeBoundingBox() {
  m_hBox = Box();
  for ( auto& point : m_Vertices ) { m_hBox.update( point.position_ ); }
}

inline void Mesh::computeFaceNormals( bool normalize ) {
  m_FaceNormals.resize( getNumberOfFaces() );
  for ( size_t i = 0; i < getNumberOfFaces(); i++ ) {
    m_FaceNormals[i] =
        glm::cross( m_Vertices[m_Indices[i * 3 + 1]].position_ - m_Vertices[m_Indices[i * 3]].position_,
                    m_Vertices[m_Indices[i * 3 + 2]].position_ - m_Vertices[m_Indices[i * 3]].position_ );
    if ( normalize ) { normalizeNormal( m_FaceNormals[i] ); }
  }
}

void Mesh::computeVertexNormals( bool normalize, bool noSeams ) {
  if ( m_FaceNormals.size() == 0 ) { computeFaceNormals( false ); }
 {
    auto cmp = []( const Vec3& a, const Vec3& b ) {
      if ( a.x != b.x ) { return a.x < b.x; }
      if ( a.y != b.y ) { return a.y < b.y; }
      return a.z < b.z;
    };
    typedef std::map<int, Vec3>                                                             TmpVertNormals;
    typedef std::map<Vec3, TmpVertNormals, std::function<bool( const Vec3&, const Vec3& )>> TmpPosNormals;
    TmpPosNormals                                                                           tmpNormals( cmp );

    for ( size_t t = 0; t < getNumberOfFaces(); t++ ) {
      const auto& faceNormal = m_FaceNormals[t];
      for ( size_t i = 0; i < 3; i++ ) {
        const unsigned int idx     = m_Indices[t * 3 + i];
        const Vec3   pos     = m_Vertices[idx].position_;
        auto         posIter = tmpNormals.find( pos );
        bool         found   = false;
        if ( posIter != tmpNormals.end() ) {
          auto vertIter = posIter->second.find( idx );
          if ( vertIter != posIter->second.end() ) {
            vertIter->second = vertIter->second + faceNormal;
            found            = true;
          }
        }
        if ( !found ) { tmpNormals[pos][idx] = faceNormal; }
      }
    }

    for ( auto p = tmpNormals.begin(); p != tmpNormals.end(); p++ ) {
      glm::vec3 normal( 0.0f, 0.0f, 0.0f );
      for ( auto v = p->second.begin(); v != p->second.end(); v++ ) { normal += v->second; }
      if ( normalize ) { normalizeNormal( normal ); }
      for ( auto v = p->second.begin(); v != p->second.end(); v++ ) { m_Vertices[v->first].normal_ = normal; }
    }
  }
  m_FaceNormals.clear();
}

vMeshObj::vMeshObj() {}
vMeshObj::~vMeshObj() { m_Meshes.clear(); }

void vMeshObj::load() {
  if ( !m_bLoad ) {
    for ( auto& mesh : m_Meshes ) { mesh.load(); }
    m_bLoad = true;
  }
}


void vMeshObj::computeVertexNormals() {
  for ( auto& mesh : m_Meshes ) {
      mesh.computeVertexNormals(); 
  }
}


bool vMeshObj::read( std::string path, int32_t framesIndex ) {
  auto ext = getExtension( path );
  if ( ext == "obj" ) return readObj( path, framesIndex );
  return false;
}

bool vMeshObj::readObj( std::string path, int32_t framesIndex ) {
  m_nNumVertices = 0;
  m_nNumFaces    = 0;
  m_nNumTextures = 0;
  m_rFilename    = createFilename( path, framesIndex );
  m_rDirectory   = getDirectory( m_rFilename );
  tinyobj::ObjReaderConfig readerConfig;
  tinyobj::ObjReader       reader;
  readerConfig.mtl_search_path = m_rDirectory;
  if ( !reader.ParseFromFile( m_rFilename, readerConfig ) ) {
    if ( !reader.Error().empty() ) { std::cerr << "TinyObjReader: " << reader.Error(); }
    exit( 1 );
  }
  if ( !reader.Warning().empty() ) { std::cout << "TinyObjReader: " << reader.Warning(); }
  auto& attrib    = reader.GetAttrib();
  auto& shapes    = reader.GetShapes();
  auto& materials = reader.GetMaterials();
  // PrintInfo( attrib, shapes, materials );
  m_Meshes.resize( shapes.size() );
  bool bUVCoordinates = false;
  for ( size_t s = 0; s < shapes.size(); s++ ) {  // Loop over shapes
    auto&  vertices    = m_Meshes[s].getVertices();
    auto&  indices     = m_Meshes[s].getIndices();
    size_t numVertices = 0;
    for ( auto& f : shapes[s].mesh.num_face_vertices ) { numVertices += f; }
    m_nNumVertices += (int)numVertices;
    m_nNumFaces += (int)numVertices;
    vertices.resize( numVertices );
    size_t index     = 0;
    bool   bUVSupOne = false;
    for ( size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++ ) {          // Loop over faces
      for ( size_t v = 0; v < size_t( shapes[s].mesh.num_face_vertices[f] ); v++ ) {  // Loop over vertices
        auto& idx        = shapes[s].mesh.indices[index];
        auto& vertex     = vertices[index];
        vertex.position_ = Vec3( attrib.vertices[3 * size_t( idx.vertex_index ) + 0],
                                 attrib.vertices[3 * size_t( idx.vertex_index ) + 1],
                                 attrib.vertices[3 * size_t( idx.vertex_index ) + 2] );
        if ( idx.texcoord_index >= 0 ) {
          vertex.color_     = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
          vertex.texCoords_ = Vec2( attrib.texcoords[2 * size_t( idx.texcoord_index ) + 0],
                                    attrib.texcoords[2 * size_t( idx.texcoord_index ) + 1] );
          bUVCoordinates    = true;
          if ( vertex.texCoords_[0] > 1.0f && vertex.texCoords_[1] > 1.f ) { bUVSupOne = true; }
        } else {
          vertex.texCoords_ = Vec2( 0.0f, 0.0f );
          vertex.color_     = Vec4( attrib.colors[3 * size_t( idx.vertex_index ) + 0],
                                attrib.colors[3 * size_t( idx.vertex_index ) + 1],
                                attrib.colors[3 * size_t( idx.vertex_index ) + 2], 1.f );
        }
        indices.push_back( (unsigned int)index );
        index++;
      }
    }
    m_Meshes[s].setUseColorPerVertex( !bUVCoordinates );
    int tid = shapes[s].mesh.material_ids[0] >= 0
                  ? shapes[s].mesh.material_ids[0]
                  : shapes.size() == 1 && materials.size() == 1 && shapes[s].mesh.material_ids[0] == -1 ? 0 : -1;
    if ( bUVCoordinates && tid == -1 ) { tid = 0; }
    if ( tid >= 0 ) {
      std::string diffuse_texname = materials.size() > 0 ? materials[tid].diffuse_texname : std::string();
      if ( bUVCoordinates && diffuse_texname.empty() ) {
        diffuse_texname = std::string( getBasename( getRemoveExtension( m_rFilename ) ) + ".png" );
      }
      readTextures( diffuse_texname, "texture_diffuse", m_Meshes[s].getTextures() );
      if ( bUVSupOne ) {
        int  width  = m_Meshes[s].getTextures().back().width_;
        int  height = m_Meshes[s].getTextures().back().height_;
        Vec2 maxUV( width - 1, height - 1 );
        for ( auto& v : vertices ) {
          if ( maxUV[0] < v.texCoords_[0] ) { maxUV[0] = v.texCoords_[0]; }
          if ( maxUV[1] < v.texCoords_[1] ) { maxUV[1] = v.texCoords_[1]; }
        }
        maxUV[0] = (float)( ( ( (int)maxUV[0] / width ) + 1 ) * width - 1 );
        maxUV[1] = (float)( ( ( (int)maxUV[1] / height ) + 1 ) * height - 1 );
        for ( auto& v : vertices ) { v.texCoords_ /= maxUV; }
      }
      m_nNumTextures++;
    }
    m_Meshes[s].recomputeBoundingBox();
    m_hBox.update( m_Meshes[s].getBox() );
  }
  computeVertexNormals();
  return true;
}




void vMeshObj::readTextures( const std::string& name, std::string type, std::vector<Texture>& textures ) {
  bool alreadyLoad = false;
  for ( unsigned int j = 0; j < textures.size(); j++ ) {
    if ( textures[j].path_ == name ) {
      textures.push_back( textures[j] );
      alreadyLoad = true;
      break;
    }
  }
  if ( !alreadyLoad ) {
    int      comp;
    Texture  texture;
    auto     filename = m_rDirectory + getSeparator() + name;
    uint8_t* image    = stbi_load( filename.c_str(), &texture.width_, &texture.height_, &comp, STBI_rgb );
    texture.data_.resize( 3 * texture.width_ * texture.height_ );
    std::memcpy( texture.data_.data(), image, 3 * texture.width_ * texture.height_ * sizeof( uint8_t ) );
    stbi_image_free( image );
    texture.type_ = type;
    texture.path_ = name;
    textures.push_back( texture );
  }
}

void vMeshObj::recomputeBoundingBox() {
  m_hBox = Box();
  for ( auto& mesh : m_Meshes ) {
    mesh.recomputeBoundingBox();
    m_hBox.update( mesh.getBox() );
  }
}

void vMeshObj::center( Box box, float fBoxSize ) {
  Vec3 center = fBoxSize / 2.f - ( box.min() + ( box.max() - box.min() ) / 2.f );
  for ( auto& mesh : m_Meshes ) { mesh.center( center ); }
}

void vMeshObj::scale( Box box, float fBoxSize ) {
  if ( fBoxSize != 0.f ) {
    float fScale = fBoxSize / box.getMaxSize();
    for ( auto& mesh : m_Meshes ) { mesh.scale( box.min(), fScale ); }
  }
}

