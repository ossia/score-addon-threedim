#include "AnEffectModel.hpp"
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "../3rdparty/tiny_obj_loader.h"

#include <ssynth/Model/Builder.h>
#include <ssynth/Model/Rendering/ObjRenderer.h>
#include <ssynth/Parser/EisenParser.h>
#include <ssynth/Parser/Preprocessor.h>
#include <ssynth/Parser/Tokenizer.h>
#include <QString>
#include <iostream>

namespace StructureSynth
{
static auto CreateObj()
{
  QString input = R"_(set maxdepth 2000
{ a 0.9 hue 30 } R1

rule R1 w 10 {
{ x 1  rz 3 ry 5  } R1
{ s 1 1 0.1 sat 0.9 } box
}

rule R1 w 10 {
{ x 1  rz -3 ry 5  } R1
{ s 1 1 0.1 } box
}
)_";
  ssynth::Parser::Preprocessor p;
  auto preprocessed = p.Process(input);

  ssynth::Parser::Tokenizer t{std::move(preprocessed)};
  ssynth::Parser::EisenParser e{t};

  auto ruleset = std::unique_ptr<ssynth::Model::RuleSet>{e.parseRuleset()};
  ruleset->resolveNames();
  ruleset->dumpInfo();

  ssynth::Model::Rendering::ObjRenderer obj{10, 10, true, false};
  ssynth::Model::Builder b(&obj, ruleset.get(), true);
  b.build();

  QByteArray data;
  {
    QTextStream ts(&data);
    obj.writeToStream(ts);
    ts.flush();
  }

  return data.toStdString();

}
std::vector<float> positions;
std::vector<float> normals;
std::vector<float> complete;
void Generator::operator()()
{
  if(done)
    return;
  done = true;

  auto input = CreateObj();
  std::string inputfile = "cornell_box.obj";
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./"; // Path to material files

  tinyobj::ObjReader reader;

  std::string default_mtl = R"(newmtl default
Ka  0.1986  0.0000  0.0000
Kd  0.5922  0.0166  0.0000
Ks  0.5974  0.2084  0.2084
illum 2
Ns 100.2237
)";
  if (!reader.ParseFromString(input, default_mtl,  reader_config)) {
    if (!reader.Error().empty()) {
      qDebug() << "TinyObjReader: " << reader.Error().c_str();
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    qDebug() << "TinyObjReader: " << reader.Warning().c_str();
  }

  qDebug() << reader.Valid();
  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  //auto& materials = reader.GetMaterials();

  positions.clear();
  normals.clear();
  positions.reserve(215784);
  normals.reserve(215784);
  complete.clear();
  complete.reserve(positions.capacity() + normals.capacity());
  // Loop over shapes
  for (size_t s = 0; s < std::max(1UL, shapes.size()); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t fv : shapes[s].mesh.num_face_vertices)
    {
      Q_ASSERT(fv == 3);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
        tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
        tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
        positions.push_back(vx);
        positions.push_back(vy);
        positions.push_back(vz);

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
          tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
          tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
          normals.push_back(nx);
          normals.push_back(ny);
          normals.push_back(nz);
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        // if (idx.texcoord_index >= 0) {
        //   tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
        //   tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
        // }

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
      }
      index_offset += fv;

      // per-face material
      // shapes[s].mesh.material_ids[f];
    }
  }
  Q_ASSERT(positions.size() == normals.size());
  Q_ASSERT(positions.size() % 9 == 0);
  qDebug() << "parsed obj successfully ! "
           << positions.size()
           << "Vertices: " << positions.size() / 3
           << "Triangles: " << positions.size() / (3 * 3)
      ;

  complete.insert(complete.end(), positions.begin(), positions.end());
  complete.insert(complete.end(), normals.begin(), normals.end());

  qDebug() << "complete.size()" << complete.size()  << complete.size() * sizeof(float);

  outputs.geometry.buffers.main_buffer.data = complete.data();
  outputs.geometry.buffers.main_buffer.size = complete.size();
  outputs.geometry.buffers.main_buffer.dirty = true;

  outputs.geometry.input.input1.offset = positions.size();
  outputs.geometry.vertices = positions.size() / 3;
  outputs.geometry.dirty = true;

  /*
  if(shapes.size() > 0)
  {
    int k = 0;
    for(auto& indice : shapes[0].mesh.indices)
    {
      qDebug() << k++ << indice.vertex_index << indice.normal_index << indice.texcoord_index;
    }
    // shapes[0].lines.indices;
    // shapes[0].mesh.indices;
    // shapes[0].points.indices;
  }
*/
}
}
