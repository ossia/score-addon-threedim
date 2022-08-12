#pragma once

#include <Gfx/Graph/Node.hpp>
#include <ossia/detail/packed_struct.hpp>

#include <QFont>
#include <QPen>

namespace score::gfx
{
/**
 * @brief A node that renders a model to screen.
 */
struct ModelDisplayNode : NodeModel
{
public:
  explicit ModelDisplayNode();
  virtual ~ModelDisplayNode();

  score::gfx::NodeRenderer* createRenderer(RenderList& r) const noexcept override;

  void process(Message&& msg) override;
  class Renderer;
  ModelCameraUBO ubo;

  ossia::vec3f position, rotation, scale;
  float cameraDistance{50.f};
};

}
