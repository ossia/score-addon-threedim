#pragma once
#include <ossia/detail/pod_vector.hpp>
#include <halp/controls.hpp>
#include <boost/container/vector.hpp>
#include <QMatrix4x4>

#include <string_view>
#include <vector>
#include <cstring>
namespace Threedim
{
using float_vec = boost::container::vector<float, ossia::pod_allocator<float>>;

struct mesh {
  int64_t vertices{};
  int64_t pos_offset{}, texcoord_offset{}, normal_offset{};
  bool texcoord{};
  bool normals{};
};

std::vector<mesh> ObjFromString(
    std::string_view obj_data
    , std::string_view mtl_data
    , float_vec& data);

std::vector<mesh> ObjFromString(
    std::string_view obj_data
    , float_vec& data);

template <std::size_t N>
static void fromGL(float (&from)[N], auto& to)
{
  memcpy(to.data(), from, sizeof(float[N]));
}
template <std::size_t N>
static void toGL(auto& from, float (&to)[N])
{
  memcpy(to, from.data(), sizeof(float[N]));
}

inline void rebuild_transform(auto& inputs, auto& outputs)
{
  QMatrix4x4 model{};
  auto& pos = inputs.position;
  auto& rot = inputs.rotation;
  auto& sc = inputs.scale;

  model.translate(pos.value.x, pos.value.y, pos.value.z);
  model.rotate(QQuaternion::fromEulerAngles(rot.value.x, rot.value.y, rot.value.z));
  model.scale(sc.value.x, sc.value.y, sc.value.z);

  toGL(model, outputs.geometry.transform);
  outputs.geometry.dirty_transform = true;
}
struct PositionControl
    : halp::xyz_spinboxes_f32<"Position", halp::range{-1000., 1000., 0.}>
{
  void update(auto& o) { rebuild_transform(o.inputs, o.outputs); }
};
struct RotationControl
    : halp::xyz_spinboxes_f32<"Rotation", halp::range{0., 359.9999999, 0.}>
{
  void update(auto& o) { rebuild_transform(o.inputs, o.outputs); }
};
struct ScaleControl : halp::xyz_spinboxes_f32<"Scale", halp::range{0.00001, 1000., 1.}>
{
  void update(auto& o) { rebuild_transform(o.inputs, o.outputs); }
};

}
