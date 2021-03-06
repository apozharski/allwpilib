/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "Filters/Filter.h"

#include "Base.h"

using namespace frc;

Filter::Filter(PIDSource& source)
    : m_source(std::shared_ptr<PIDSource>(&source, NullDeleter<PIDSource>())) {}

Filter::Filter(std::shared_ptr<PIDSource> source)
    : m_source(std::move(source)) {}

void Filter::SetPIDSourceType(PIDSourceType pidSource) {
  m_pidSourceType = pidSource;
}

PIDSourceType Filter::GetPIDSourceType() const {
  return m_pidSourceType;
}

double Filter::PIDGetSource() { return m_source->PIDGet(m_pidSourceType); }
