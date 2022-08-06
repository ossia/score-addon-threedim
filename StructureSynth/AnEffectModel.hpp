#pragma once

#include <halp/geometry.hpp>

#include <halp/controls.hpp>
#include <halp/meta.hpp>

namespace StructureSynth
{

class Generator
{
public:
  halp_meta(name, "Structure Synth")
  halp_meta(category, "Gfx")
  halp_meta(c_name, "structure_synth")
  halp_meta(uuid, "bb8f3d77-4cfd-44ce-9c43-b64c54a748ab")

  struct ins
  {
  } inputs;

  struct
  {
    struct : halp::position_normals_geometry {
      halp_meta(name, "Geometry");
    } geometry;
  } outputs;


  // Defined in the .cpp
  void operator()();

  bool done{};
};

}
