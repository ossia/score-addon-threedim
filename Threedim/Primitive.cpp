#include "Primitive.hpp"

#include <QString>
#include <QDebug>
#include <iostream>
#include <vcg/complex/algorithms/clean.h>
#include <vcg/complex/algorithms/create/platonic.h>

namespace
{
using namespace vcg;
class TFace;
class TVertex;

struct TUsedTypes : public vcg::UsedTypes< vcg::Use<TVertex>::AsVertexType, vcg::Use<TFace>::AsFaceType > { };

class TVertex : public Vertex<TUsedTypes,
                              vertex::BitFlags,
                              vertex::Coord3f,
                              vertex::Normal3f,
                              vertex::Mark > { };

class TFace : public Face<TUsedTypes,
                          face::VertexRef,
                          face::Normal3f,
                          face::BitFlags,
                          face::FFAdj
                          > {};

class TMesh : public vcg::tri::TriMesh<std::vector<TVertex>, std::vector<TFace>> { };
}

namespace Threedim
{

Primitive::Primitive() = default;
Primitive::~Primitive() = default;

void Primitive::operator()()
{
}

void Primitive::update()
{
  static thread_local TMesh mesh;
  mesh.Clear();

  switch(inputs.geometry) {
    case decltype(inputs.geometry)::Sphere:
      vcg::tri::Sphere(mesh, 4);
      break;
    case decltype(inputs.geometry)::Icosahedron:
      vcg::tri::Icosahedron(mesh);
      break;
    case decltype(inputs.geometry)::Cone:
      vcg::tri::Cone(mesh, 1., 5., 12.);
      break;
    case decltype(inputs.geometry)::Cylinder:
      vcg::tri::Cylinder(64, 64, mesh);
      break;
    case decltype(inputs.geometry)::Torus:
      vcg::tri::Torus(mesh, 7., 4.);
      break;
    case decltype(inputs.geometry)::Cube:
      vcg::Box3<float> box;
      box.min = {-1, -1, -1};
      box.max = {1, 1, 1};
      vcg::tri::Box(mesh, box);
      break;
  }

  vcg::tri::Clean<TMesh>::RemoveUnreferencedVertex(mesh);
  vcg::tri::Clean<TMesh>::RemoveZeroAreaFace(mesh);
  vcg::tri::UpdateTopology<TMesh>::FaceFace(mesh);
  vcg::tri::Clean<TMesh>::RemoveNonManifoldFace(mesh);
  vcg::tri::UpdateTopology<TMesh>::FaceFace(mesh);
  vcg::tri::UpdateNormal<TMesh>::PerVertexNelsonMaxWeighted(mesh);

  vcg::tri::RequirePerVertexNormal(mesh);

  complete.clear();
  const auto vertices = mesh.face.size() * 3;
  const auto floats = vertices * (3 + 3); // 3 float for pos, 3 float for normal
  complete.resize(floats);
  float* pos_start = complete.data();
  float* norm_start = complete.data() + vertices * 3;
  const float scale = inputs.scale;

  for(auto& fi : mesh.face){ // iterate each faces

    auto v0 = fi.V(0);
    auto v1 = fi.V(1);
    auto v2 = fi.V(2);

    auto p0 = v0->P();
    (*pos_start++) = p0.X() * scale;
    (*pos_start++) = p0.Y() * scale;
    (*pos_start++) = p0.Z() * scale;

    auto p1 = v1->P();
    (*pos_start++) = p1.X() * scale;
    (*pos_start++) = p1.Y() * scale;
    (*pos_start++) = p1.Z() * scale;

    auto p2 = v2->P();
    (*pos_start++) = p2.X() * scale;
    (*pos_start++) = p2.Y() * scale;
    (*pos_start++) = p2.Z() * scale;

    auto n0 = v0->N();
    (*norm_start++) = n0.X();
    (*norm_start++) = n0.Y();
    (*norm_start++) = n0.Z();

    auto n1 = v1->N();
    (*norm_start++) = n1.X();
    (*norm_start++) = n1.Y();
    (*norm_start++) = n1.Z();

    auto n2 = v2->N();
    (*norm_start++) = n2.X();
    (*norm_start++) = n2.Y();
    (*norm_start++) = n2.Z();
  }

  outputs.geometry.buffers.main_buffer.data = complete.data();
  outputs.geometry.buffers.main_buffer.size = complete.size();
  outputs.geometry.buffers.main_buffer.dirty = true;

  outputs.geometry.input.input1.offset = complete.size() / 2;
  outputs.geometry.vertices = vertices;
  outputs.geometry.dirty = true;
}

}
