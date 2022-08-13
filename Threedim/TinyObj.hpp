#pragma once
#include <ossia/detail/pod_vector.hpp>
#include <boost/container/vector.hpp>
#include <string_view>
#include <vector>
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

}
