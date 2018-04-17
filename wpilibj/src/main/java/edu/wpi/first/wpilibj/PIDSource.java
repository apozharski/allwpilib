/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj;

/**
 * This interface allows for PIDController to automatically read from this object.
 */
public interface PIDSource {

  /**
   * Get the result to use in PIDController.
   *
   * @return the result to use in PIDController
   */
  double pidGet(PIDSourceType pidSource);
}
