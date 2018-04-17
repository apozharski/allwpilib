/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "Counter.h"
#include "PIDSource.h"
#include "SensorBase.h"

namespace frc {

class DigitalInput;
class DigitalOutput;

/**
 * Ultrasonic rangefinder class.
 *
 * The Ultrasonic rangefinder measures absolute distance based on the round-trip
 * time of a ping generated by the controller. These sensors use two
 * transducers, a speaker and a microphone both tuned to the ultrasonic range. A
 * common ultrasonic sensor, the Daventech SRF04 requires a short pulse to be
 * generated on a digital channel. This causes the chirp to be emitted. A second
 * line becomes high as the ping is transmitted and goes low when the echo is
 * received. The time that the line is high determines the round trip distance
 * (time of flight).
 */
class Ultrasonic : public SensorBase, public PIDSource {
public:
  enum DistanceUnit { kInches = 0, kMilliMeters = 1 };

  Ultrasonic(DigitalOutput *pingChannel, DigitalInput *echoChannel,
             DistanceUnit units = kInches);
  Ultrasonic(DigitalOutput &pingChannel, DigitalInput &echoChannel,
             DistanceUnit units = kInches);
  Ultrasonic(std::shared_ptr<DigitalOutput> pingChannel,
             std::shared_ptr<DigitalInput> echoChannel,
             DistanceUnit units = kInches);
  Ultrasonic(int pingChannel, int echoChannel, DistanceUnit units = kInches);
  ~Ultrasonic() override;

  void Ping();
  bool IsRangeValid() const;
  static void SetAutomaticMode(bool enabling);
  double GetRangeInches() const;
  double GetRangeMM() const;
  bool IsEnabled() const { return m_enabled; }
  void SetEnabled(bool enable) { m_enabled = enable; }

  double PIDGet() override;
  void SetPIDSourceType(PIDSourceType pidSource) override;
  void SetDistanceUnits(DistanceUnit units);
  DistanceUnit GetDistanceUnits() const;

  void InitSendable(SendableBuilder &builder) override;

private:
  void Initialize();

  static void UltrasonicChecker();

  // Time (sec) for the ping trigger pulse.
  static constexpr double kPingTime = 10 * 1e-6;

  // Priority that the ultrasonic round robin task runs.
  static constexpr int kPriority = 64;

  // Max time (ms) between readings.
  static constexpr double kMaxUltrasonicTime = 0.1;
  static constexpr double kSpeedOfSoundInchesPerSec = 1130.0 * 12.0;

  // Thread doing the round-robin automatic sensing
  static std::thread m_thread;

  // Ultrasonic sensors
  static std::vector<Ultrasonic *> m_sensors;

  // Automatic round-robin mode
  static std::atomic<bool> m_automaticEnabled;

  std::shared_ptr<DigitalOutput> m_pingChannel;
  std::shared_ptr<DigitalInput> m_echoChannel;
  bool m_enabled = false;
  Counter m_counter;
  DistanceUnit m_units;
};

} // namespace frc
