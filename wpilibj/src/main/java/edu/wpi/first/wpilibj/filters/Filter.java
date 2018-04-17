/*----------------------------------------------------------------------------*/
/* Copyright (c) 2015-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj.filters;

import edu.wpi.first.wpilibj.PIDSource;
import edu.wpi.first.wpilibj.PIDSourceType;

/**
 * Superclass for filters.
 */
public abstract class Filter implements PIDSource {
  private PIDSource m_source;

  public Filter(PIDSource source) {
    m_source = source;
  }


  @Override
  public abstract double pidGet(PIDSourceType pidSource);

  /**
   * Returns the current filter estimate without also inserting new data as pidGet() would do.
   *
   * @return The current filter estimate
   */
  public abstract double get();

  /**
   * Reset the filter state.
   */
  public abstract void reset();

  /**
   * Calls PIDGet() of source.
   *
   * @return Current value of source
   */
  protected double pidGetSource(PIDSourceType pidSource) {
    return m_source.pidGet(pidSource);
  }
}
