#include "ObjLoader.hpp"

#include <miniply.h>

#include <QDebug>
#include <QMatrix4x4>
#include <QString>

#include <iostream>

namespace Threedim
{

void ObjLoader::operator()() { }

void ObjLoader::rebuild_geometry()
{
  std::vector<mesh>& new_meshes = this->meshinfo;

  if (!outputs.geometry.mesh.empty())
  {
    outputs.geometry.mesh.clear();
  }

  for (auto& m : new_meshes)
  {
    if (m.vertices <= 0)
      continue;

    halp::dynamic_geometry geom;

    geom.buffers.clear();
    geom.bindings.clear();
    geom.attributes.clear();
    geom.input.clear();
    if (m.points)
    {
      geom.topology = halp::dynamic_geometry::points;
      geom.cull_mode = halp::dynamic_geometry::none;
      geom.front_face = halp::dynamic_geometry::counter_clockwise;
    }
    else
    {
      geom.topology = halp::dynamic_geometry::triangles;
      geom.cull_mode = halp::dynamic_geometry::back;
      geom.front_face = halp::dynamic_geometry::counter_clockwise;
    }
    geom.index = {};

    geom.vertices = m.vertices;

    geom.buffers.push_back(halp::dynamic_geometry::buffer{
        .data = this->complete.data(),
        .size = int64_t(this->complete.size() * sizeof(float)),
        .dirty = true});

    // Bindings
    geom.bindings.push_back(halp::dynamic_geometry::binding{
        .stride = 3 * sizeof(float),
        .step_rate = 1,
        .classification = halp::dynamic_geometry::binding::per_vertex});

    if (m.texcoord)
    {
      geom.bindings.push_back(halp::dynamic_geometry::binding{
          .stride = 2 * sizeof(float),
          .step_rate = 1,
          .classification = halp::dynamic_geometry::binding::per_vertex});
    }

    if (m.normals)
    {
      geom.bindings.push_back(halp::dynamic_geometry::binding{
          .stride = 3 * sizeof(float),
          .step_rate = 1,
          .classification = halp::dynamic_geometry::binding::per_vertex});
    }

    // Attributes
    geom.attributes.push_back(halp::dynamic_geometry::attribute{
        .binding = 0,
        .location = halp::dynamic_geometry::attribute::position,
        .format = halp::dynamic_geometry::attribute::float3,
        .offset = 0});

    if (m.texcoord)
    {
      geom.attributes.push_back(halp::dynamic_geometry::attribute{
          .binding = 1,
          .location = halp::dynamic_geometry::attribute::tex_coord,
          .format = halp::dynamic_geometry::attribute::float2,
          .offset = 0});
    }

    if (m.normals)
    {
      geom.attributes.push_back(halp::dynamic_geometry::attribute{
          .binding = m.texcoord ? 2 : 1,
          .location = halp::dynamic_geometry::attribute::normal,
          .format = halp::dynamic_geometry::attribute::float3,
          .offset = 0});
    }

    // Vertex input
    using input_t = struct halp::dynamic_geometry::input;
    geom.input.push_back(
        input_t{.buffer = 0, .offset = m.pos_offset * (int)sizeof(float)});

    if (m.texcoord)
    {
      geom.input.push_back(
          input_t{.buffer = 0, .offset = m.texcoord_offset * (int)sizeof(float)});
    }

    if (m.normals)
    {
      geom.input.push_back(
          input_t{.buffer = 0, .offset = m.normal_offset * (int)sizeof(float)});
    }

    outputs.geometry.mesh.push_back(std::move(geom));
    outputs.geometry.dirty_mesh = true;
  }
}

static bool check_file_extension(std::string_view filename, std::string_view expected)
{
  if (filename.size() < expected.size())
    return false;
  auto ext = filename.substr(filename.size() - expected.size(), expected.size());
  for (int i = 0; i < expected.size(); i++)
    if (std::tolower(ext[i]) != std::tolower(expected[i]))
      return false;
  return true;
}

bool print_ply_header(const char* filename)
{
  static const char* kPropertyTypes[] = {
      "char",
      "uchar",
      "short",
      "ushort",
      "int",
      "uint",
      "float",
      "double",
  };

  miniply::PLYReader reader(filename);
  if (!reader.valid())
  {
    return false;
  }
  using namespace miniply;
  for (uint32_t i = 0, endI = reader.num_elements(); i < endI; i++)
  {
    const miniply::PLYElement* elem = reader.get_element(i);
    fprintf(stderr, "element %s %u\n", elem->name.c_str(), elem->count);
    for (const miniply::PLYProperty& prop : elem->properties)
    {
      if (prop.countType != miniply::PLYPropertyType::None)
      {
        fprintf(
            stderr,
            "property list %s %s %s\n",
            kPropertyTypes[uint32_t(prop.countType)],
            kPropertyTypes[uint32_t(prop.type)],
            prop.name.c_str());
      }
      else
      {
        fprintf(
            stderr,
            "property %s %s\n",
            kPropertyTypes[uint32_t(prop.type)],
            prop.name.c_str());
      }
    }
  }
  fprintf(stderr, "end_header\n");

  return true;
}
// Very basic triangle mesh struct, for example purposes
struct TriMesh
{
  // Per-vertex data
  float* pos = nullptr;   // has 3 * numVerts elements.
  float* uv = nullptr;    // if non-null, has 2 * numVerts elements.
  float* norm = nullptr;  // if non-null, has 3 * numVerts elements.
  float* color = nullptr; // if non-null, has 3 * numVerts elements.
  uint32_t numVerts = 0;

  // Per-index data
  int* indices = nullptr;
  uint32_t numIndices = 0; // number of indices = 3 times the number of triangles.
};

bool load_vert_from_ply(miniply::PLYReader& reader, TriMesh* trimesh)
{
  uint32_t indexes[3];
  if (reader.element_is(miniply::kPLYVertexElement) && reader.load_element()
      && reader.find_pos(indexes))
  {
    trimesh->numVerts = reader.num_rows();
    if (trimesh->numVerts <= 0)
      return false;

    trimesh->pos = new float[trimesh->numVerts * 3];
    reader.extract_properties(indexes, 3, miniply::PLYPropertyType::Float, trimesh->pos);
    if (reader.find_texcoord(indexes))
    {
      trimesh->uv = new float[trimesh->numVerts * 2];
      reader.extract_properties(
          indexes, 2, miniply::PLYPropertyType::Float, trimesh->uv);
    }
    if (reader.find_normal(indexes))
    {
      trimesh->uv = new float[trimesh->numVerts * 3];
      reader.extract_properties(
          indexes, 3, miniply::PLYPropertyType::Float, trimesh->uv);
    }
    if (reader.find_color(indexes))
    {
      trimesh->color = new float[trimesh->numVerts * 3];
      reader.extract_properties(
          indexes, 3, miniply::PLYPropertyType::Float, trimesh->color);
    }
    return true;
  }
  return false;
}

bool load_face_from_ply(miniply::PLYReader& reader, TriMesh* trimesh, bool gotVerts)
{
  uint32_t indexes[3];
  if (reader.element_is(miniply::kPLYFaceElement) && reader.load_element()
      && reader.find_indices(indexes))
  {
    bool polys = reader.requires_triangulation(indexes[0]);
    if (polys && !gotVerts)
    {
      fprintf(stderr, "Error: need vertex positions to triangulate faces.\n");
      return false;
    }
    if (polys)
    {
      trimesh->numIndices = reader.num_triangles(indexes[0]) * 3;
      if (trimesh->numIndices <= 0)
        return false;
      trimesh->indices = new int[trimesh->numIndices];
      reader.extract_triangles(
          indexes[0],
          trimesh->pos,
          trimesh->numVerts,
          miniply::PLYPropertyType::Int,
          trimesh->indices);
    }
    else
    {
      trimesh->numIndices = reader.num_rows() * 3;
      trimesh->indices = new int[trimesh->numIndices];
      reader.extract_list_property(
          indexes[0], miniply::PLYPropertyType::Int, trimesh->indices);
    }
    return true;
  }
  return false;
}

TriMesh* load_trimesh_from_ply(miniply::PLYReader& reader)
{
  bool gotVerts = false, gotFaces = false;

  TriMesh* trimesh = new TriMesh();
  while (reader.has_element() && (!gotVerts || !gotFaces))
  {
    if (auto verts = load_vert_from_ply(reader, trimesh))
      gotVerts = verts;
    else if (auto faces = load_face_from_ply(reader, trimesh, gotVerts))
      gotFaces = faces;

    if (gotVerts && gotFaces)
    {
      break;
    }
    reader.next_element();
  }

  if (!gotVerts || !gotFaces)
  {
    delete trimesh;
    return nullptr;
  }

  return trimesh;
}

TriMesh* load_vertices_from_ply(miniply::PLYReader& reader)
{
  TriMesh* trimesh = new TriMesh();
  while (reader.has_element())
  {
    if (load_vert_from_ply(reader, trimesh))
      return trimesh;
    reader.next_element();
  }

  delete trimesh;
  return nullptr;
}

std::function<void(ObjLoader&)> ObjLoader::ins::obj_t::process(file_type tv)
{
  if (check_file_extension(tv.filename, "obj"))
  {
    // This part happens in a separate thread
    Threedim::float_vec buf;
    if (auto mesh = Threedim::ObjFromString(tv.bytes, buf); !mesh.empty())
    {
      return [mesh = std::move(mesh), buf = std::move(buf)](ObjLoader& o) mutable
      {
        // This part happens in the execution thread
        std::swap(o.meshinfo, mesh);
        std::swap(o.complete, buf);

        o.rebuild_geometry();
      };
    }
  }
  else if (check_file_extension(tv.filename, "ply"))
  {
    print_ply_header(tv.filename.data());

    std::vector<mesh> mesh;
    miniply::PLYReader reader{tv.filename.data()};
    if (!reader.valid())
      return {};

    auto res = load_vertices_from_ply(reader);
    if (!res->pos)
      return {};
    Threedim::float_vec buf;
    buf.assign(res->pos, res->pos + res->numVerts * 3);
    delete res;

    mesh.push_back({.vertices = res->numVerts, .points = true});

    if (!mesh.empty())
    {
      return [mesh = std::move(mesh), buf = std::move(buf)](ObjLoader& o) mutable
      {
        // This part happens in the execution thread
        std::swap(o.meshinfo, mesh);
        std::swap(o.complete, buf);

        o.rebuild_geometry();
      };
    }
  }
  return {};
}

}
