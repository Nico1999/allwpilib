/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/Relay.h"

#include "DigitalInternal.h"
#include "handles/IndexedHandleResource.h"

using namespace hal;

namespace {
struct Relay {
  uint8_t pin;
  bool fwd;
};
}

constexpr uint32_t kRelayPins = 8;
constexpr uint32_t kRelayHeaders = kRelayPins / 2;  // Number of FPGA ID's

static IndexedHandleResource<HalRelayHandle, Relay, kRelayPins,
                             HalHandleEnum::Relay>
    relayHandles;

// Create a mutex to protect changes to the relay values
static priority_recursive_mutex digitalRelayMutex;

extern "C" {
HalRelayHandle initializeRelayPort(HalPortHandle port_handle, uint8_t fwd,
                                   int32_t* status) {
  initializeDigital(status);

  if (*status != 0) return HAL_INVALID_HANDLE;

  int16_t pin = getPortHandlePin(port_handle);
  if (pin == InvalidHandleIndex) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_INVALID_HANDLE;
  }

  if (!fwd) pin += kRelayHeaders;  // add 4 to reverse pins

  auto handle = relayHandles.Allocate(pin, status);

  if (*status != 0)
    return HAL_INVALID_HANDLE;  // failed to allocate. Pass error back.

  auto port = relayHandles.Get(handle);
  if (port == nullptr) {  // would only occur on thread issue.
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_INVALID_HANDLE;
  }

  if (!fwd) {
    pin -= kRelayHeaders;  // subtract number of headers to put pin in range.
    port->fwd = false;     // set to reverse
  } else {
    port->fwd = true;  // set to forward
  }

  port->pin = static_cast<uint8_t>(pin);
  return handle;
}

void freeRelayPort(HalRelayHandle relay_port_handle) {
  // no status, so no need to check for a proper free.
  relayHandles.Free(relay_port_handle);
}

bool checkRelayChannel(uint8_t pin) {
  return pin < kRelayHeaders;  // roboRIO only has 4 headers, and the FPGA has
                               // seperate functions for forward and reverse,
                               // instead of seperate pin IDs
}

/**
 * Set the state of a relay.
 * Set the state of a relay output.
 */
void setRelay(HalRelayHandle relay_port_handle, bool on, int32_t* status) {
  auto port = relayHandles.Get(relay_port_handle);
  if (port == nullptr) {
    *status = PARAMETER_OUT_OF_RANGE;
    return;
  }
  std::lock_guard<priority_recursive_mutex> sync(digitalRelayMutex);
  uint8_t relays = 0;
  if (port->fwd) {
    relays = relaySystem->readValue_Forward(status);
  } else {
    relays = relaySystem->readValue_Reverse(status);
  }

  if (*status != 0) return;  // bad status read

  if (on) {
    relays |= 1 << port->pin;
  } else {
    relays &= ~(1 << port->pin);
  }

  if (port->fwd) {
    relaySystem->writeValue_Forward(relays, status);
  } else {
    relaySystem->writeValue_Reverse(relays, status);
  }
}

/**
 * Get the current state of the relay channel
 */
bool getRelay(HalRelayHandle relay_port_handle, int32_t* status) {
  auto port = relayHandles.Get(relay_port_handle);
  if (port == nullptr) {
    *status = PARAMETER_OUT_OF_RANGE;
    return false;
  }

  uint8_t relays = 0;
  if (port->fwd) {
    relays = relaySystem->readValue_Forward(status);
  } else {
    relays = relaySystem->readValue_Reverse(status);
  }

  return (relays & (1 << port->pin)) != 0;
}
}