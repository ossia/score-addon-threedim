#pragma once

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
  halp_meta(category, "Gfx")
  halp_meta(c_name, "structure_synth")
  halp_meta(uuid, "bb8f3d77-4cfd-44ce-9c43-b64c54a748ab")

  struct ins
  {
    struct edit : halp::lineedit<"Text", "">
    {
      void update(StrucSynth& g) { g.recompute(); }
    } hehe;
  } inputs;

  struct
  {
    struct
    {
      halp_meta(name, "Geometry");
      halp::position_normals_geometry mesh;
    } geometry;
  } outputs;

  ~StrucSynth();

  void operator()();

  void recompute();

  std::atomic_bool done{};
  std::mutex swap_mutex; // FIXME
  using float_vec = boost::container::vector<float, ossia::pod_allocator<float>>;
  float_vec complete;
  float_vec swap TS_GUARDED_BY(swap_mutex);
  std::thread compute_thread;
};

}
