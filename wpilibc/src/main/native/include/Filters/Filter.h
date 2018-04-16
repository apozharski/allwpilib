/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <memory>

#include "PIDSource.h"

namespace frc {

/**
 * Interface for filters
 */
class Filter : public PIDSource {
 public:
  explicit Filter(PIDSource& source);
  explicit Filter(std::shared_ptr<PIDSource> source);
  virtual ~Filter() = default;

  // PIDSource interface
  void SetPIDSourceType(PIDSourceType pidSource);
  PIDSourceType GetPIDSourceType() const;
  double PIDGet(PIDSourceType pidSource) override = 0;

  /**
   * Returns the current filter estimate without also inserting new data as
   * PIDGet() would do.
   *
   * @return The current filter estimate
   */
  virtual double Get() const = 0;

  /**
   * Reset the filter state
   */
  virtual void Reset() = 0;

 protected:
  /**
   * Calls PIDGet() of source
   *
   * @return Current value of source
   */
  double PIDGetSource();

 private:
  std::shared_ptr<PIDSource> m_source;
  PIDSourceType m_pidSourceType;
};

}  // namespace frc
