cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include("${SCORE_SOURCE_DIR}/cmake/ScoreExternalAddon.cmake")
endif()

project(score_addon_structuresynth LANGUAGES CXX)
find_package(${QT_VERSION} REQUIRED COMPONENTS Xml)


add_library(
  score_addon_structuresynth

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

  StructureSynth/AnEffectModel.hpp
  StructureSynth/AnEffectModel.cpp
  StructureSynth/AnEffectUi.hpp
  StructureSynth/AnEffect.hpp

  score_addon_structuresynth.hpp
  score_addon_structuresynth.cpp
)

setup_score_plugin(score_addon_structuresynth)

target_include_directories(score_addon_structuresynth PRIVATE 3rdparty/libssynth/src)
target_link_libraries(score_addon_structuresynth PUBLIC score_plugin_engine score_plugin_avnd  ${QT_PREFIX}::Xml)
