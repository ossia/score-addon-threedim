#pragma once
#include <Threedim/TinyObj.hpp>
#include <halp/controls.hpp>
#include <halp/file_port.hpp>
#include <halp/geometry.hpp>
#include <halp/meta.hpp>
#include <ossia/detail/mutex.hpp>

#include <thread>

namespace Threedim
{

class ObjLoader
{
public:
  halp_meta(name, "Object loader")
  halp_meta(category, "Visuals/3D")
  halp_meta(c_name, "obj_loader")
  halp_meta(uuid, "5df71765-505f-4ab7-98c1-f305d10a01ef")

  struct ins
  {
    struct obj_t : halp::file_port<"OBJ file">
    {
      halp_meta(extensions, "*.obj");

      static std::function<void(ObjLoader&)> process(file_type data);
    } obj;
  } inputs;

  struct
  {
    struct
    {
      halp_meta(name, "Geometry");
      std::vector<halp::dynamic_geometry> mesh;
      bool dirty = false;
    } geometry;
  } outputs;

  void operator()();

  void rebuild_geometry();

  std::vector<mesh> meshinfo{};
  float_vec complete;
};

}
