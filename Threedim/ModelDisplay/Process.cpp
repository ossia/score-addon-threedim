#include "Process.hpp"

#include <Gfx/Graph/Node.hpp>
#include <Gfx/TexturePort.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/Dataflow/WidgetInlets.hpp>
#include <wobjectimpl.h>

#include <QFileInfo>
#include <QImageReader>

W_OBJECT_IMPL(Gfx::ModelDisplay::Model)
namespace Gfx::ModelDisplay
{

Model::Model(
    const TimeVal& duration,
    const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id, "gfxProcess", parent}
{
  metadata().setInstanceName(*this);

  m_inlets.push_back(new TextureInlet{Id<Process::Port>(0), this});
  m_inlets.push_back(new GeometryInlet{Id<Process::Port>(1), this});

  m_inlets.push_back(new Process::XYZSpinboxes{
      ossia::vec3f{-100., -100., -100.},
      ossia::vec3f{100., 100., 100.},
      ossia::vec3f{},
      "Position",
      Id<Process::Port>(2),
      this});
  m_inlets.push_back(new Process::XYZSpinboxes{
      ossia::vec3f{0., 0., 0.},
      ossia::vec3f{359.999999999, 359.999999999, 359.999999999},
      ossia::vec3f{},
      "Rotation",
      Id<Process::Port>(3),
      this});
  m_inlets.push_back(new Process::XYZSpinboxes{
      ossia::vec3f{0.001, 0.001, 0.001},
      ossia::vec3f{100., 100., 100.},
      ossia::vec3f{1., 1., 1.},
      "Scale",
      Id<Process::Port>(4),
      this});

  m_inlets.push_back(new Process::FloatSlider{
      0.01, 1000., 50., "Distance", Id<Process::Port>(5), this});

  m_outlets.push_back(new TextureOutlet{Id<Process::Port>(0), this});
}

Model::~Model() { }

QString Model::prettyName() const noexcept
{
  return tr("Text");
}

}
template <>
void DataStreamReader::read(const Gfx::ModelDisplay::Model& proc)
{
  readPorts(*this, proc.m_inlets, proc.m_outlets);

  insertDelimiter();
}

template <>
void DataStreamWriter::write(Gfx::ModelDisplay::Model& proc)
{
  writePorts(
      *this,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);

  checkDelimiter();
}

template <>
void JSONReader::read(const Gfx::ModelDisplay::Model& proc)
{
  readPorts(*this, proc.m_inlets, proc.m_outlets);
}

template <>
void JSONWriter::write(Gfx::ModelDisplay::Model& proc)
{
  writePorts(
      *this,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);
}
