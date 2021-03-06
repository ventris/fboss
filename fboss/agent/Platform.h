/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/Conv.h>
#include <folly/MacAddress.h>
#include <memory>
#include "fboss/agent/types.h"
#include "fboss/agent/PlatformPort.h"
#include "fboss/agent/if/gen-cpp2/ctrl_types.h"

// TODO(aeckert): change to forward declaration once all platforms implement
// createTestHandle
#include "fboss/agent/test/HwTestHandle.h"

namespace facebook { namespace fboss {

class HwSwitch;
class SwSwitch;
class ThriftHandler;
struct ProductInfo;

/*
 * Platform represents a specific switch/router platform.
 *
 * Note that Platform is somewhat similar to the HwSwitch class--both contain
 * hardware-specific details.  However, HwSwitch contains data only about the
 * switching logic, while Platform contains all platform specific information,
 * beyond just the switching logic.  The Platform class itself isn't involved
 * in switching, it simply knows what type of HwSwitch to instantiate for this
 * platform.
 *
 * In general, multiple platforms may share the same HwSwitch implementation.
 * For instance, the BcmSwitch class is a HwSwitch implementation for systems
 * based on a single Broadcom ASIC.  Any platform with a single Broadcom ASIC
 * may use BcmSwitch, but they will likely each have their own Platform
 * implementation.
 */
class Platform {
 public:
  Platform() {}
  virtual ~Platform() {}

  /*
   * Get the HwSwitch for this platform.
   *
   * The HwSwitch object returned should be owned by the Platform, and must
   * remain valid for the lifetime of the Platform object.
   */
  virtual HwSwitch* getHwSwitch() const = 0;

  /*
   * onHwInitialized() will be called once the HwSwitch object has been
   * initialized.  Platform-specific initialization that requires access to the
   * HwSwitch can be performed here.
   */
  virtual void onHwInitialized(SwSwitch* sw) = 0;

  /*
   * onInitialConfigApplied() will be called after the initial
   * configuration has been applied.  Platform-specific initialization
   * that needs to happen after this can be performed here.
   */
  virtual void onInitialConfigApplied(SwSwitch* sw) = 0;

  /*
   * Create the ThriftHandler.
   *
   * This will be invoked by fbossMain() during the initialization process.
   */
  virtual std::unique_ptr<ThriftHandler> createHandler(SwSwitch* sw) = 0;


  /*
   * Get the local MAC address for the switch.
   *
   * This method must be thread safe.  It may be called simultaneously from
   * various different threads.
   */
  virtual folly::MacAddress getLocalMac() const = 0;

  /*
   * Get the path to a directory where persistent state can be stored.
   *
   * Files written to this directory should be preserved across system reboots.
   */
  virtual std::string getPersistentStateDir() const = 0;

  /*
   * Get the product information
   */
  virtual void getProductInfo(ProductInfo& info) = 0;

  /*
   * Get the path to a directory where volatile state can be stored.
   *
   * Files written to this directory should be preserved across controller
   * restarts, but must be removed across system restarts.
   *
   * For instance, these files could be stored in a ramdisk.  Alternatively,
   * these could be stored in persistent storage with an init script that
   * empties the directory on reboot.
   */
  virtual std::string getVolatileStateDir() const = 0;

  /*
   * Get the directory where we will dump info when there is a crash.
   *
   * The directory is in persistent storage.
   */
  std::string getCrashInfoDir() const {
    return getPersistentStateDir() + "/crash";
  }

  /*
   * Get the directory where warm boot state is stored.
   */
  std::string getWarmBootDir() const {
    return getVolatileStateDir() + "/warm_boot";
  }
  /*
   * Get filename for where we dump hw state on crash
   */
  std::string getCrashHwStateFile() const;
  /*
   * Get filename for where we dump switch state on crash
   */
  std::string getCrashSwitchStateFile() const;
  /*
   * For a specific logical port, return the transceiver and channel
   * it represents if available
   */
  virtual TransceiverIdxThrift getPortMapping(PortID port) const = 0;

  virtual PlatformPort* getPlatformPort(PortID port) const = 0;

  /*
   * Provide a test handle to induce certain low level events. This is
   * used to unit test the platform code.
   *
   * TODO(aeckert): make pure virtual once all platforms implement this.
   */
  virtual std::unique_ptr<HwTestHandle> createTestHandle(
      std::unique_ptr<SwSwitch> /*sw*/) {
    return nullptr;
  };

 private:
  // Forbidden copy constructor and assignment operator
  Platform(Platform const &) = delete;
  Platform& operator=(Platform const &) = delete;
};

}} // facebook::fboss
