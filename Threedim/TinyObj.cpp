#include "TinyObj.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
// #define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "../3rdparty/tiny_obj_loader.h"

#include <QDebug>
#include <QElapsedTimer>
namespace Threedim
{

std::optional<mesh> ObjFromString(
    std::string_view obj_data
    , std::string_view mtl_data
    , float_vec& buf)
{
  tinyobj::ObjReaderConfig reader_config;

  tinyobj::ObjReader reader;

  QElapsedTimer e;
  e.start();
  if (!reader.ParseFromString(obj_data, mtl_data, reader_config)) {
    if (!reader.Error().empty()) {
      qDebug() << "TinyObjReader: " << reader.Error().c_str();
    }
    return std::nullopt;
  }
  qDebug() << "Elapsed: " << e.nsecsElapsed() / 1e3;

  if (!reader.Warning().empty()) {
    qDebug() << "TinyObjReader: " << reader.Warning().c_str();
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  if(shapes.empty())
    return std::nullopt;

  const auto faces = shapes[0].mesh.num_face_vertices.size();
  const auto vertices = faces * 3;

  const bool texcoords = !attrib.texcoords.empty();
  const bool normals = !attrib.normals.empty();

  std::size_t float_count =
      vertices * 3
      + (normals   ? vertices * 3 : 0)
      + (texcoords ? vertices * 2 : 0);

  // 3 float per vertex for position, 3 per vertex for normal
  buf.resize(float_count, boost::container::default_init);

  float* pos = buf.data();
  float* tc = nullptr;
  float* norm = nullptr;
  if(texcoords)
  {
    tc = pos + vertices * 3;
    if(normals)
      norm = pos + vertices * 3 + vertices * 2;
  }
  else if(normals)
  {
    norm = pos + vertices * 3;
  }

  auto& shape = shapes[0];
  {
    size_t index_offset = 0;

    if(norm && tc)
    {
      for (size_t fv : shape.mesh.num_face_vertices)
      {
        if(fv != 3)
          return std::nullopt;
        for (size_t v = 0; v < 3; v++)
        {
          const auto idx = shape.mesh.indices[index_offset + v];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+0];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+1];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+2];

          *tc++ = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          *tc++ = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

          *norm++ = attrib.normals[3*size_t(idx.normal_index)+0];
          *norm++ = attrib.normals[3*size_t(idx.normal_index)+1];
          *norm++ = attrib.normals[3*size_t(idx.normal_index)+2];
        }
        index_offset += 3;
      }
    }
    else if(norm)
    {
      for (size_t fv : shape.mesh.num_face_vertices)
      {
        if(fv != 3)
          return std::nullopt;
        for (size_t v = 0; v < 3; v++)
        {
          const auto idx = shape.mesh.indices[index_offset + v];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+0];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+1];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+2];

          *norm++ = attrib.normals[3*size_t(idx.normal_index)+0];
          *norm++ = attrib.normals[3*size_t(idx.normal_index)+1];
          *norm++ = attrib.normals[3*size_t(idx.normal_index)+2];
        }
        index_offset += 3;
      }
    }
    else if(tc)
    {
      for (size_t fv : shape.mesh.num_face_vertices)
      {
        if(fv != 3)
          return std::nullopt;
        for (size_t v = 0; v < 3; v++)
        {
          const auto idx = shape.mesh.indices[index_offset + v];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+0];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+1];
          *pos++ = attrib.vertices[3*size_t(idx.vertex_index)+2];

          *tc++ = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          *tc++ = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
        }
        index_offset += 3;
      }
    }
  }

  return mesh{
        .vertices = int64_t(vertices)
      , .texcoord = texcoords
      , .normals = normals
  };
}

std::optional<mesh> ObjFromString(
    std::string_view obj_data
    , float_vec& data)
{

  std::string default_mtl = R"(newmtl default
Ka  0.1986  0.0000  0.0000
Kd  0.5922  0.0166  0.0000
Ks  0.5974  0.2084  0.2084
illum 2
Ns 100.2237
)";

  return ObjFromString(obj_data, default_mtl, data);
}

}
