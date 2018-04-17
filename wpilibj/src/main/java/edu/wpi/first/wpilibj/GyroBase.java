/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj;

import edu.wpi.first.wpilibj.interfaces.Gyro;
import edu.wpi.first.wpilibj.smartdashboard.SendableBuilder;

/**
 * GyroBase is the common base class for Gyro implementations such as AnalogGyro.
 */
public abstract class GyroBase extends SensorBase implements Gyro, PIDSource, Sendable {


  /**
   * Get the output of the gyro for use with PIDControllers. May be the angle or rate depending on
   * the set PIDSourceType
   *
   * @return the output according to the gyro
   */
  @Override
  public double pidGet(PIDSourceType pidSource) {
    switch (pidSource) {
      case kRate:
        return getRate();
      case kDisplacement:
        return getAngle();
      default:
        return 0.0;
    }
  }

  @Override
  public void initSendable(SendableBuilder builder) {
    builder.setSmartDashboardType("Gyro");
    builder.addDoubleProperty("Value", this::getAngle, null);
  }
}
