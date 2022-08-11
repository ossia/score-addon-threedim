#pragma once
#include <ossia/detail/pod_vector.hpp>
#include <boost/container/vector.hpp>
#include <string_view>
#include <optional>
namespace Threedim
{
using float_vec = boost::container::vector<float, ossia::pod_allocator<float>>;

struct mesh {
  int64_t vertices{};
  bool texcoord{};
  bool normals{};
};

std::optional<mesh> ObjFromString(
    std::string_view obj_data
    , std::string_view mtl_data
    , float_vec& data);

std::optional<mesh> ObjFromString(
    std::string_view obj_data
    , float_vec& data);

}
