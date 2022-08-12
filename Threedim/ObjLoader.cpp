#include "ObjLoader.hpp"

#include <QDebug>
#include <QString>

#include <iostream>

namespace Threedim
{

ObjLoader::~ObjLoader()
{
  if (compute_thread.joinable())
    compute_thread.join();
}

void ObjLoader::operator()()
{
  if (done.exchange(false, std::memory_order_seq_cst))
  {
    mesh m;
    {
      std::lock_guard<std::mutex> lck{swap_mutex};
      m = this->meshinfo;
      std::swap(swap, complete);
    }

    halp::dynamic_geometry geom;
    if (!outputs.geometry.mesh.empty())
    {
      geom = std::move(outputs.geometry.mesh[0]);
      outputs.geometry.mesh.clear();
    }

    geom.buffers.clear();
    geom.bindings.clear();
    geom.attributes.clear();
    geom.input.clear();
    geom.vertices = m.vertices;
    geom.topology = halp::dynamic_geometry::triangles;
    geom.cull_mode = halp::dynamic_geometry::back;
    geom.front_face = halp::dynamic_geometry::counter_clockwise;
    geom.index = {};
    geom.dirty = true;

    if (m.vertices > 0)
    {
      geom.buffers.push_back(halp::dynamic_geometry::buffer{
          .data = this->complete.data(),
          .size = int64_t(this->complete.size() * sizeof(float)),
          .dirty = true});

      // Bindings
      geom.bindings.push_back(halp::dynamic_geometry::binding{
          .stride = 3 * sizeof(float),
          .classification = halp::dynamic_geometry::binding::per_vertex,
          .step_rate = 1});

      if (m.texcoord)
      {
        geom.bindings.push_back(halp::dynamic_geometry::binding{
            .stride = 2 * sizeof(float),
            .classification = halp::dynamic_geometry::binding::per_vertex,
            .step_rate = 1});
      }

      if (m.normals)
      {
        geom.bindings.push_back(halp::dynamic_geometry::binding{
            .stride = 3 * sizeof(float),
            .classification = halp::dynamic_geometry::binding::per_vertex,
            .step_rate = 1});
      }

      // Attributes
      geom.attributes.push_back(halp::dynamic_geometry::attribute{
          .binding = 0,
          .location = halp::dynamic_geometry::attribute::position,
          .format = halp::dynamic_geometry::attribute::float3,
          .offset = 0});

      if (m.texcoord)
      {
        geom.attributes.push_back(halp::dynamic_geometry::attribute{
            .binding = 1,
            .location = halp::dynamic_geometry::attribute::tex_coord,
            .format = halp::dynamic_geometry::attribute::float2,
            .offset = 0});
      }

      if (m.normals)
      {
        geom.attributes.push_back(halp::dynamic_geometry::attribute{
            .binding = m.texcoord ? 2 : 1,
            .location = halp::dynamic_geometry::attribute::normal,
            .format = halp::dynamic_geometry::attribute::float3,
            .offset = 0});
      }

      // Vertex input
      using input_t = struct halp::dynamic_geometry::input;
      int64_t cur_offset = 0;
      geom.input.push_back(input_t{.buffer = 0, .offset = cur_offset});
      cur_offset += 3 * meshinfo.vertices * sizeof(float);

      if (m.texcoord)
      {
        geom.input.push_back(input_t{.buffer = 0, .offset = cur_offset});
        cur_offset += 2 * meshinfo.vertices * sizeof(float);
      }

      if (m.normals)
      {
        geom.input.push_back(input_t{.buffer = 0, .offset = cur_offset});
      }

      outputs.geometry.mesh.push_back(std::move(geom));
      outputs.geometry.dirty = true;
    }
  }
}

void ObjLoader::load(halp::text_file_view tv)
{
  // FIXME have an LV2-like thread-pool worker API
  if (compute_thread.joinable())
    compute_thread.join();

  compute_thread = std::thread(
      [this, tv]() mutable
      {
        Threedim::float_vec buf;
        if (auto mesh = Threedim::ObjFromString(tv.bytes, buf))
        {
          std::lock_guard<std::mutex> lck{swap_mutex};
          this->meshinfo = *mesh;
          std::swap(buf, swap);
        }
        done.store(true, std::memory_order_seq_cst);
      });
}
}
