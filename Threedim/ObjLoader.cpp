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
    std::vector<mesh> new_meshes;
    {
      std::lock_guard<std::mutex> lck{swap_mutex};
      std::swap(new_meshes, this->meshinfo); // FIXME delete
      std::swap(swap, complete);
    }

    if (!outputs.geometry.mesh.empty())
    {
      //geom = std::move(outputs.geometry.mesh[0]);
      outputs.geometry.mesh.clear();
    }

    for (auto& m : new_meshes)
    {
      if (m.vertices <= 0)
        continue;

      halp::dynamic_geometry geom;

      geom.buffers.clear();
      geom.bindings.clear();
      geom.attributes.clear();
      geom.input.clear();
      geom.topology = halp::dynamic_geometry::triangles;
      geom.cull_mode = halp::dynamic_geometry::back;
      geom.front_face = halp::dynamic_geometry::counter_clockwise;
      geom.index = {};

      geom.vertices = m.vertices;
      geom.dirty = true;

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
      geom.input.push_back(
          input_t{.buffer = 0, .offset = m.pos_offset * (int)sizeof(float)});

      if (m.texcoord)
      {
        geom.input.push_back(
            input_t{.buffer = 0, .offset = m.texcoord_offset * (int)sizeof(float)});
      }

      if (m.normals)
      {
        geom.input.push_back(
            input_t{.buffer = 0, .offset = m.normal_offset * (int)sizeof(float)});
      }

      outputs.geometry.mesh.push_back(std::move(geom));
      outputs.geometry.dirty = true;
    }
  }
}

void ObjLoader::load(halp::text_file_view tv)
{
  if (tv.filename == cur_filename)
    return;
  cur_filename = tv.filename;
  // FIXME have an LV2-like thread-pool worker API
  if (compute_thread.joinable())
    compute_thread.join();

  compute_thread = std::thread(
      [this, tv]() mutable
      {
        Threedim::float_vec buf;
        if (auto mesh = Threedim::ObjFromString(tv.bytes, buf); !mesh.empty())
        {
          std::lock_guard<std::mutex> lck{swap_mutex};
          std::swap(this->meshinfo, mesh);
          std::swap(buf, swap);
        }
        done.store(true, std::memory_order_seq_cst);
      });
}
}
