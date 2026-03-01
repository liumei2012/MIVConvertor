
#include "ReaderSequenceParameters.h"

static inline std::istream& operator>>( std::istream& stream, Vec3u8& v ) {
  uint16_t a = 0, b = 0, c = 0;
  stream >> a >> b >> c;
  v = Vec3u8( ( std::min )( a, (uint16_t)std::numeric_limits<uint8_t>::max() ),
              ( std::min )( b, (uint16_t)std::numeric_limits<uint8_t>::max() ),
              ( std::min )( c, (uint16_t)std::numeric_limits<uint8_t>::max() ) );
  return stream;
}
static inline std::istream& operator>>( std::istream& stream, Vec3& v ) {
  stream >> v[0] >> v[1] >> v[2];
  return stream;
}
static std::ostream& operator<<( std::ostream& stream, Vec3u8 const& v ) {
  stream << "'" << (int)v[0] << " " << (int)v[1] << " " << (int)v[2] << "'";
  return stream;
}
static std::ostream& operator<<( std::ostream& stream, Vec3 const& v ) {
  stream << "'" << v[0] << " " << v[1] << " " << v[2] << "'";
  return stream;
}

vMeshParameters::vMeshParameters()  = default;
vMeshParameters::~vMeshParameters() = default;


