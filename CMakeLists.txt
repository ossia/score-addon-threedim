cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(score_addon_threedim LANGUAGES CXX)

if(NOT TARGET score_lib_base)
  include(ScoreExternalAddon)
endif()

if(NOT TARGET score_plugin_avnd)
  return()
endif()
if(NOT TARGET score_plugin_gfx)
  return()
endif()

find_package(${QT_VERSION} REQUIRED COMPONENTS Xml)


add_library(
  score_addon_threedim

  3rdparty/libssynth/src/ssynth/Parser/EisenParser.cpp
  3rdparty/libssynth/src/ssynth/Parser/Preprocessor.cpp
  3rdparty/libssynth/src/ssynth/Parser/Tokenizer.cpp

  3rdparty/libssynth/src/ssynth/Model/Action.cpp
  3rdparty/libssynth/src/ssynth/Model/AmbiguousRule.cpp
  3rdparty/libssynth/src/ssynth/Model/Builder.cpp
  3rdparty/libssynth/src/ssynth/Model/CustomRule.cpp
  3rdparty/libssynth/src/ssynth/Model/PrimitiveRule.cpp
  3rdparty/libssynth/src/ssynth/Model/RuleSet.cpp
  3rdparty/libssynth/src/ssynth/Model/State.cpp
  3rdparty/libssynth/src/ssynth/Model/Transformation.cpp

  3rdparty/libssynth/src/ssynth/Model/Rendering/TemplateRenderer.cpp
  3rdparty/libssynth/src/ssynth/Model/Rendering/ObjRenderer.cpp

  3rdparty/libssynth/src/ssynth/ColorPool.cpp
  3rdparty/libssynth/src/ssynth/ColorUtils.cpp
  3rdparty/libssynth/src/ssynth/Logging.cpp
  3rdparty/libssynth/src/ssynth/MiniParser.cpp
  3rdparty/libssynth/src/ssynth/RandomStreams.cpp

  Threedim/TinyObj.cpp

  Threedim/StructureSynth.hpp
  Threedim/StructureSynth.cpp

  Threedim/ObjLoader.hpp
  Threedim/ObjLoader.cpp

  Threedim/Primitive.hpp
  Threedim/Primitive.cpp

  Threedim/Noise.hpp
  Threedim/Noise.cpp


  Threedim/ModelDisplay/ModelDisplayNode.hpp
  Threedim/ModelDisplay/ModelDisplayNode.cpp
  Threedim/ModelDisplay/Executor.hpp
  Threedim/ModelDisplay/Executor.cpp
  Threedim/ModelDisplay/Metadata.hpp
  Threedim/ModelDisplay/Process.hpp
  Threedim/ModelDisplay/Process.cpp
  Threedim/ModelDisplay/Layer.hpp

  score_addon_threedim.hpp
  score_addon_threedim.cpp
)

set_property(TARGET score_addon_threedim PROPERTY SCORE_CUSTOM_PCH 1)
setup_score_plugin(score_addon_threedim)

target_include_directories(score_addon_threedim
  PRIVATE
    3rdparty/libssynth/src
    3rdparty/vcglib
    3rdparty/eigen
)

target_link_libraries(score_addon_threedim
  PUBLIC
    score_plugin_engine
    score_plugin_avnd
    score_plugin_gfx
    ${QT_PREFIX}::Xml
    fmt::fmt
)
