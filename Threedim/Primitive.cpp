#include "Primitive.hpp"

#include <vcg/complex/algorithms/clean.h>
#include <vcg/complex/algorithms/create/platonic.h>

#include <QDebug>
#include <QString>

#include <iostream>

namespace
{
using namespace vcg;
class TFace;
class TVertex;

struct TUsedTypes
    : public vcg::UsedTypes<vcg::Use<TVertex>::AsVertexType, vcg::Use<TFace>::AsFaceType>
{
};

class TVertex
    : public Vertex<
          TUsedTypes,
          vertex::BitFlags,
          vertex::Coord3f,
          vertex::Normal3f,
          vertex::Mark>
{
};

class TFace
    : public Face<
          TUsedTypes,
          face::VertexRef,
          face::Normal3f,
          face::BitFlags,
          face::FFAdj>
{
};

class TMesh : public vcg::tri::TriMesh<std::vector<TVertex>, std::vector<TFace>>
{
};
}

namespace Threedim
{

static thread_local TMesh mesh;

static void
loadTriMesh(TMesh& mesh, std::vector<float>& complete, PrimitiveOutputs& outputs)
{
  vcg::tri::Clean<TMesh>::RemoveUnreferencedVertex(mesh);
  vcg::tri::Clean<TMesh>::RemoveZeroAreaFace(mesh);
  vcg::tri::UpdateTopology<TMesh>::FaceFace(mesh);
  vcg::tri::Clean<TMesh>::RemoveNonManifoldFace(mesh);
  vcg::tri::UpdateTopology<TMesh>::FaceFace(mesh);
  vcg::tri::UpdateNormal<TMesh>::PerVertexNormalized(mesh);

  vcg::tri::RequirePerVertexNormal(mesh);

  complete.clear();
  const auto vertices = mesh.face.size() * 3;
  const auto floats = vertices * (3 + 3); // 3 float for pos, 3 float for normal
  complete.resize(floats);
  float* pos_start = complete.data();
  float* norm_start = complete.data() + vertices * 3;

  for (auto& fi : mesh.face)
  { // iterate each faces

    auto v0 = fi.V(0);
    auto v1 = fi.V(1);
    auto v2 = fi.V(2);

    auto p0 = v0->P();
    (*pos_start++) = p0.X();
    (*pos_start++) = p0.Y();
    (*pos_start++) = p0.Z();

    auto p1 = v1->P();
    (*pos_start++) = p1.X();
    (*pos_start++) = p1.Y();
    (*pos_start++) = p1.Z();

    auto p2 = v2->P();
    (*pos_start++) = p2.X();
    (*pos_start++) = p2.Y();
    (*pos_start++) = p2.Z();

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
  outputs.geometry.mesh.buffers.main_buffer.data = complete.data();
  outputs.geometry.mesh.buffers.main_buffer.size = complete.size();
  outputs.geometry.mesh.buffers.main_buffer.dirty = true;

  outputs.geometry.mesh.input.input1.offset = complete.size() / 2;
  outputs.geometry.mesh.vertices = vertices;
  outputs.geometry.dirty_mesh = true;
}

void Cube::update()
{
  mesh.Clear();
  vcg::Box3<float> box;
  box.min = {-1, -1, -1};
  box.max = {1, 1, 1};
  vcg::tri::Box(mesh, box);
  loadTriMesh(mesh, complete, outputs);
}

void Sphere::update()
{
  mesh.Clear();
  vcg::tri::Sphere(mesh, inputs.subdiv);
  loadTriMesh(mesh, complete, outputs);
}

void Icosahedron::update()
{
  mesh.Clear();
  vcg::tri::Icosahedron(mesh);
  loadTriMesh(mesh, complete, outputs);
}

void Cone::update()
{
  mesh.Clear();
  vcg::tri::Cone(mesh, inputs.r1, inputs.r2, inputs.h, inputs.subdiv);
  loadTriMesh(mesh, complete, outputs);
}

void Cylinder::update()
{
  mesh.Clear();
  vcg::tri::Cylinder(inputs.slices, inputs.stacks, mesh, true);
  loadTriMesh(mesh, complete, outputs);
}

void Torus::update()
{
  mesh.Clear();
  vcg::tri::Torus(mesh, inputs.r1, inputs.r2, inputs.hdiv, inputs.vdiv);
  loadTriMesh(mesh, complete, outputs);
}

}
