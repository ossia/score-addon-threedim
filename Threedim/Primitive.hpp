#pragma once

#include <halp/controls.hpp>
#include <halp/geometry.hpp>
#include <halp/meta.hpp>

namespace Threedim
{

class Primitive
{
public:
  halp_meta(name, "3D Primitive")
  halp_meta(category, "Visuals/3D")
  halp_meta(c_name, "3d_primitive")
  halp_meta(uuid, "cf8a328a-1ba6-47f8-929f-2168bdec90b0")

  struct ins
  {
    struct
    {
      halp__enum("Primitive", Cube, Cube, Sphere, Icosahedron, Cone, Cylinder, Torus);
      void update(Primitive& self) { self.update(); }
    } geometry;

    struct : halp::hslider_f32<"Scale", halp::range{0.01, 100, 1.}>
    {
      void update(Primitive& self) { self.update(); }
    } scale;
  } inputs;

  struct
  {
    struct
    {
      halp_meta(name, "Geometry");
      halp::position_normals_geometry mesh;
    } geometry;
  } outputs;

  Primitive();
  ~Primitive();

  void operator()();
  void update();

  // using embed_ui;
  // using editor_ui;

private:
  std::vector<float> complete;
};

}
