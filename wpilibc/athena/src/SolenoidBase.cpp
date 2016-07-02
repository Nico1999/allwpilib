/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008-2016. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "SolenoidBase.h"

#include "HAL/HAL.h"

/**
 * Constructor
 *
 * @param moduleNumber The CAN PCM ID.
 */
SolenoidBase::SolenoidBase(uint8_t moduleNumber)
    : m_moduleNumber(moduleNumber) {}

/**
 * Read all 8 solenoids as a single byte
 *
 * @return The current value of all 8 solenoids on the module.
 */
uint8_t SolenoidBase::GetAll(int module) const {
  uint8_t value = 0;
  int32_t status = 0;
  value = getAllSolenoids(static_cast<uint8_t>(module), &status);
  wpi_setErrorWithContext(status, getHALErrorMessage(status));
  return value;
}

/**
 * Reads complete solenoid blacklist for all 8 solenoids as a single byte.
 *
 * If a solenoid is shorted, it is added to the blacklist and
 * disabled until power cycle, or until faults are cleared.
 * @see ClearAllPCMStickyFaults()
 *
 * @return The solenoid blacklist of all 8 solenoids on the module.
 */
uint8_t SolenoidBase::GetPCMSolenoidBlackList(int module) const {
  int32_t status = 0;
  return getPCMSolenoidBlackList(static_cast<uint8_t>(module), &status);
}

/**
 * @return true if PCM sticky fault is set : The common highside solenoid
 *         voltage rail is too low, most likely a solenoid channel is shorted.
 */
bool SolenoidBase::GetPCMSolenoidVoltageStickyFault(int module) const {
  int32_t status = 0;
  return getPCMSolenoidVoltageStickyFault(static_cast<uint8_t>(module),
                                          &status);
}

/**
 * @return true if PCM is in fault state : The common highside solenoid voltage
 *         rail is too low, most likely a solenoid channel is shorted.
 */
bool SolenoidBase::GetPCMSolenoidVoltageFault(int module) const {
  int32_t status = 0;
  return getPCMSolenoidVoltageFault(static_cast<uint8_t>(module), &status);
}

/**
 * Clear ALL sticky faults inside PCM that Compressor is wired to.
 *
 * If a sticky fault is set, then it will be persistently cleared.  Compressor
 * drive maybe momentarily disable while flags are being cleared. Care should
 * be taken to not call this too frequently, otherwise normal compressor
 * functionality may be prevented.
 *
 * If no sticky faults are set then this call will have no effect.
 */
void SolenoidBase::ClearAllPCMStickyFaults(int module) {
  int32_t status = 0;
  return clearAllPCMStickyFaults_sol(static_cast<uint8_t>(module), &status);
}