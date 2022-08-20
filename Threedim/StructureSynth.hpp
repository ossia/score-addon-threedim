#pragma once

#include <Threedim/TinyObj.hpp>
#include <boost/container/vector.hpp>
#include <halp/controls.hpp>
#include <halp/geometry.hpp>
#include <halp/meta.hpp>
#include <ossia/detail/mutex.hpp>
#include <ossia/detail/pod_vector.hpp>

#include <thread>

namespace Threedim
{

class StrucSynth
{
public:
  halp_meta(name, "Structure Synth")
  halp_meta(category, "Visuals/3D")
  halp_meta(c_name, "structure_synth")
  halp_meta(uuid, "bb8f3d77-4cfd-44ce-9c43-b64c54a748ab")

  struct ins
  {
    struct edit : halp::lineedit<"Text", "">
    {
      // Request a computation according to the currently defined program
      void update(StrucSynth& g) { g.worker.request(this->value); }
    } program;

    PositionControl position;
    RotationControl rotation;
    ScaleControl scale;
  } inputs;

  struct
  {
    struct : halp::mesh
    {
      halp_meta(name, "Geometry");
      halp::position_normals_geometry mesh;
    } geometry;
  } outputs;

  void operator()();

  struct w
  {
    std::function<void(std::string)> request;

    // Called back in a worker thread
    // The returned function will be later applied in this object's processing thread
    static std::function<void(StrucSynth&)> work(std::string_view s);
  } worker;

  using float_vec = boost::container::vector<float, ossia::pod_allocator<float>>;
  float_vec m_vertexData;
};

}
