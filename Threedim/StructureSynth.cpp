#include "StructureSynth.hpp"

#include <Threedim/TinyObj.hpp>
#include <ssynth/Model/Builder.h>
#include <ssynth/Model/Rendering/ObjRenderer.h>
#include <ssynth/Parser/EisenParser.h>
#include <ssynth/Parser/Preprocessor.h>
#include <ssynth/Parser/Tokenizer.h>

#include <QDebug>
#include <QString>

#include <iostream>

namespace Threedim
{
static auto CreateObj(const QString& input)
try
{
  /*
  QString input = R"_(set maxdepth 2000
{ a 0.9 hue 30 } R1

rule R1 w 10 {
{ x 1  rz 3 ry 5  } R1
{ s 1 1 0.1 sat 0.9 } box
}

rule R1 w 10 {
{ x 1  rz -3 ry 5  } R1
{ s 1 1 0.1 } box
}
)_";
*/
  ssynth::Parser::Preprocessor p;
  auto preprocessed = p.Process(input);

  ssynth::Parser::Tokenizer t{std::move(preprocessed)};
  ssynth::Parser::EisenParser e{t};

  auto ruleset = std::unique_ptr<ssynth::Model::RuleSet>{e.parseRuleset()};
  ruleset->resolveNames();
  ruleset->dumpInfo();

  ssynth::Model::Rendering::ObjRenderer obj{10, 10, true, false};
  ssynth::Model::Builder b(&obj, ruleset.get(), true);
  b.build();

  QByteArray data;
  {
    QTextStream ts(&data);
    obj.writeToStream(ts);
    ts.flush();
  }

  return data.toStdString();
}
catch (...)
{
  return std::string{};
}

StrucSynth::~StrucSynth()
{
  if (compute_thread.joinable())
    compute_thread.join();
}

void StrucSynth::operator()()
{
  if (done.exchange(false))
  {
    {
      std::lock_guard<std::mutex> lck{swap_mutex};
      std::swap(swap, complete);
    }

    outputs.geometry.mesh.buffers.main_buffer.data = complete.data();
    outputs.geometry.mesh.buffers.main_buffer.size = complete.size();
    outputs.geometry.mesh.buffers.main_buffer.dirty = true;

    outputs.geometry.mesh.input.input1.offset = complete.size() / 2;
    outputs.geometry.mesh.vertices = complete.size() / (2 * 3);
    outputs.geometry.mesh.dirty = true;
  }
}

void StrucSynth::recompute()
{
  // FIXME have an LV2-like thread-pool worker API
  if (compute_thread.joinable())
    compute_thread.join();

  compute_thread = std::thread(
      [in = inputs.hehe, &done = done, &swap = swap, &mut = swap_mutex]() mutable
      {
        auto input = CreateObj(QString::fromStdString(in));
        if (input.empty())
          return;

        Threedim::float_vec buf;
        if (auto mesh = Threedim::ObjFromString(input, buf); !mesh.empty())
        {
          std::lock_guard<std::mutex> lck{mut};
          std::swap(buf, swap);
        }
        done.store(true, std::memory_order_seq_cst);
      });
}
}
