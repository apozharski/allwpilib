/*----------------------------------------------------------------------------*/
/* Copyright (c) 2011-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "Commands/Scheduler.h"

#include <algorithm>
#include <set>

#include "Buttons/ButtonScheduler.h"
#include "Commands/Subsystem.h"
#include "HLUsageReporting.h"
#include "SmartDashboard/SendableBuilder.h"
#include "WPIErrors.h"

using namespace frc;

Scheduler::Scheduler() {
  HLUsageReporting::ReportScheduler();
  SetName("Scheduler");
}

/**
 * Returns the Scheduler, creating it if one does not exist.
 *
 * @return the Scheduler
 */
Scheduler* Scheduler::GetInstance() {
  static Scheduler instance;
  return &instance;
}

void Scheduler::SetEnabled(bool enabled) { m_enabled = enabled; }

/**
 * Add a command to be scheduled later.
 *
 * In any pass through the scheduler, all commands are added to the additions
 * list, then at the end of the pass, they are all scheduled.
 *
 * @param command The command to be scheduled
 */
void Scheduler::AddCommand(Command* command) {
  std::lock_guard<wpi::mutex> lock(m_additionsMutex);
  if (std::find(m_additions.begin(), m_additions.end(), command) !=
      m_additions.end())
    return;
  m_additions.push_back(command);
}

void Scheduler::AddButton(ButtonScheduler* button) {
  std::lock_guard<wpi::mutex> lock(m_buttonsMutex);
  m_buttons.push_back(button);
}

void Scheduler::ProcessCommandAddition(Command* command) {
  if (command == nullptr) return;

  // Check to make sure no adding during adding
  if (m_adding) {
    wpi_setWPIErrorWithContext(IncompatibleState,
                               "Can not start command from cancel method");
    return;
  }

  // Only add if not already in
  auto found = m_commands.find(command);
  if (found == m_commands.end()) {
    // Check that the requirements can be had
    Command::SubsystemSet requirements = command->GetRequirements();
    for (Command::SubsystemSet::iterator iter = requirements.begin();
         iter != requirements.end(); iter++) {
      Subsystem* lock = *iter;
      if (lock->GetCurrentCommand() != nullptr &&
          !lock->GetCurrentCommand()->IsInterruptible())
        return;
    }

    // Give it the requirements
    m_adding = true;
    for (Command::SubsystemSet::iterator iter = requirements.begin();
         iter != requirements.end(); iter++) {
      Subsystem* lock = *iter;
      if (lock->GetCurrentCommand() != nullptr) {
        lock->GetCurrentCommand()->Cancel();
        Remove(lock->GetCurrentCommand());
      }
      lock->SetCurrentCommand(command);
    }
    m_adding = false;

    m_commands.insert(command);

    command->StartRunning();
    m_runningCommandsChanged = true;
  }
}

/**
 * Runs a single iteration of the loop.
 *
 * This method should be called often in order to have a functioning
 * Command system. The loop has five stages:
 *
 * <ol>
 *   <li>Poll the Buttons</li>
 *   <li>Execute/Remove the Commands</li>
 *   <li>Send values to SmartDashboard</li>
 *   <li>Add Commands</li>
 *   <li>Add Defaults</li>
 * </ol>
 */
void Scheduler::Run() {
  // Get button input (going backwards preserves button priority)
  {
    if (!m_enabled) return;

    std::lock_guard<wpi::mutex> lock(m_buttonsMutex);
    for (auto rButtonIter = m_buttons.rbegin(); rButtonIter != m_buttons.rend();
         rButtonIter++) {
      (*rButtonIter)->Execute();
    }
  }

  // Call every subsystem's periodic method
  for (auto subsystemIter = m_subsystems.begin();
       subsystemIter != m_subsystems.end(); subsystemIter++) {
    Subsystem* subsystem = *subsystemIter;
    subsystem->Periodic();
  }

  m_runningCommandsChanged = false;

  // Loop through the commands
  for (auto commandIter = m_commands.begin();
       commandIter != m_commands.end();) {
    Command* command = *commandIter;
    // Increment before potentially removing to keep the iterator valid
    ++commandIter;
    if (!command->Run()) {
      Remove(command);
      m_runningCommandsChanged = true;
    }
  }

  // Add the new things
  {
    std::lock_guard<wpi::mutex> lock(m_additionsMutex);
    for (auto additionsIter = m_additions.begin();
         additionsIter != m_additions.end(); additionsIter++) {
      ProcessCommandAddition(*additionsIter);
    }
    m_additions.clear();
  }

  // Add in the defaults
  for (auto subsystemIter = m_subsystems.begin();
       subsystemIter != m_subsystems.end(); subsystemIter++) {
    Subsystem* lock = *subsystemIter;
    if (lock->GetCurrentCommand() == nullptr) {
      ProcessCommandAddition(lock->GetDefaultCommand());
    }
    lock->ConfirmCommand();
  }
}

/**
 * Registers a Subsystem to this Scheduler, so that the Scheduler might know if
 * a default Command needs to be run.
 *
 * All Subsystems should call this.
 *
 * @param system the system
 */
void Scheduler::RegisterSubsystem(Subsystem* subsystem) {
  if (subsystem == nullptr) {
    wpi_setWPIErrorWithContext(NullParameter, "subsystem");
    return;
  }
  m_subsystems.insert(subsystem);
}

/**
 * Removes the Command from the Scheduler.
 *
 * @param command the command to remove
 */
void Scheduler::Remove(Command* command) {
  if (command == nullptr) {
    wpi_setWPIErrorWithContext(NullParameter, "command");
    return;
  }

  if (!m_commands.erase(command)) return;

  Command::SubsystemSet requirements = command->GetRequirements();
  for (auto iter = requirements.begin(); iter != requirements.end(); iter++) {
    Subsystem* lock = *iter;
    lock->SetCurrentCommand(nullptr);
  }

  command->Removed();
}

void Scheduler::RemoveAll() {
  while (m_commands.size() > 0) {
    Remove(*m_commands.begin());
  }
}

/**
 * Completely resets the scheduler. Undefined behavior if running.
 */
void Scheduler::ResetAll() {
  RemoveAll();
  m_subsystems.clear();
  m_buttons.clear();
  m_additions.clear();
  m_commands.clear();
  m_namesEntry = nt::NetworkTableEntry();
  m_idsEntry = nt::NetworkTableEntry();
  m_cancelEntry = nt::NetworkTableEntry();
}

void Scheduler::InitSendable(SendableBuilder& builder) {
  builder.SetSmartDashboardType("Scheduler");
  m_namesEntry = builder.GetEntry("Names");
  m_idsEntry = builder.GetEntry("Ids");
  m_cancelEntry = builder.GetEntry("Cancel");
  builder.SetUpdateTable([=]() {
    // Get the list of possible commands to cancel
    auto new_toCancel = m_cancelEntry.GetValue();
    if (new_toCancel)
      toCancel = new_toCancel->GetDoubleArray();
    else
      toCancel.resize(0);

    // Cancel commands whose cancel buttons were pressed on the SmartDashboard
    if (!toCancel.empty()) {
      for (auto commandIter = m_commands.begin();
           commandIter != m_commands.end(); ++commandIter) {
        for (size_t i = 0; i < toCancel.size(); i++) {
          Command* c = *commandIter;
          if (c->GetID() == toCancel[i]) {
            c->Cancel();
          }
        }
      }
      toCancel.resize(0);
      m_cancelEntry.SetDoubleArray(toCancel);
    }

    // Set the running commands
    if (m_runningCommandsChanged) {
      commands.resize(0);
      ids.resize(0);
      for (auto commandIter = m_commands.begin();
           commandIter != m_commands.end(); ++commandIter) {
        Command* c = *commandIter;
        commands.push_back(c->GetName());
        ids.push_back(c->GetID());
      }
      m_namesEntry.SetStringArray(commands);
      m_idsEntry.SetDoubleArray(ids);
    }
  });
}
