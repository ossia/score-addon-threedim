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
    struct : halp::file_port<"OBJ file">
    {
      halp_meta(extensions, "*.obj");

      void update(ObjLoader& self) { self.load(this->file); }
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

  ~ObjLoader();

  void operator()();

  void load(halp::text_file_view);

  std::atomic_bool done{};
  std::vector<mesh> meshinfo{};
  std::mutex swap_mutex; // FIXME
  float_vec complete;
  float_vec swap TS_GUARDED_BY(swap_mutex);
  std::thread compute_thread;
  std::string cur_filename;
};

}
