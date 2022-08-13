#include "score_addon_threedim.hpp"
#include <Threedim/StructureSynth.hpp>
#include <Threedim/ObjLoader.hpp>
#include <Threedim/Primitive.hpp>

#include <Threedim/ModelDisplay/Executor.hpp>
#include <Threedim/ModelDisplay/Process.hpp>

#include <Avnd/Factories.hpp>
#include <score/plugins/FactorySetup.hpp>
#include <score_plugin_engine.hpp>

/**
 * This file instantiates the classes that are provided by this plug-in.
 */
score_addon_threedim::score_addon_threedim() = default;
score_addon_threedim::~score_addon_threedim() = default;

std::vector<std::unique_ptr<score::InterfaceBase>>
score_addon_threedim::factories(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key) const
{
  auto fixed = Avnd::instantiate_fx<
      Threedim::StrucSynth
      , Threedim::ObjLoader
      , Threedim::Primitive
      >(ctx, key);
  auto add = instantiate_factories<
             score::ApplicationContext,
      FW<Process::ProcessModelFactory,
         Gfx::ModelDisplay::ProcessFactory
         >,
      FW<Execution::ProcessComponentFactory,
         Gfx::ModelDisplay::ProcessExecutorComponentFactory
         >>(ctx, key);
  fixed.insert(fixed.end(), std::make_move_iterator(add.begin()), std::make_move_iterator(add.end()));
  return fixed;
}

std::vector<score::PluginKey> score_addon_threedim::required() const
{
  return {score_plugin_engine::static_key()};
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_addon_threedim)
