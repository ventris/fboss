/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/state/PortQueue.h"
#include "fboss/agent/state/NodeBase-defs.h"
#include <folly/Conv.h>

namespace {
template<typename Param>
bool isPortQueueOptionalAttributeSame(
    const folly::Optional<Param>& swValue,
    bool isConfSet,
    const Param& confValue) {
  if (!swValue.hasValue() && !isConfSet) {
    return true;
  }
  if (swValue.hasValue() && isConfSet && swValue.value() == confValue) {
    return true;
  }
  return false;
}
} // unnamed namespace

namespace facebook { namespace fboss {

state::PortQueueFields PortQueueFields::toThrift() const {
  state::PortQueueFields queue;
  queue.id = id;
  queue.weight = weight;
  if (reservedBytes) {
    queue.reserved = reservedBytes;
  }
  if (scalingFactor) {
    queue.scalingFactor =
        cfg::_MMUScalingFactor_VALUES_TO_NAMES.at(*scalingFactor);
  }
  queue.scheduling = cfg::_QueueScheduling_VALUES_TO_NAMES.at(scheduling);
  queue.streamType = cfg::_StreamType_VALUES_TO_NAMES.at(streamType);
  if (name) {
    queue.name = name;
  }
  if (packetsPerSec) {
    queue.packetsPerSec = packetsPerSec;
  }
  if (sharedBytes) {
    queue.sharedBytes = sharedBytes;
  }
  if (!aqms.empty()) {
    std::vector<cfg::ActiveQueueManagement> aqmList;
    for (const auto& aqm: aqms) {
      aqmList.push_back(aqm.second);
    }
    queue.aqms = aqmList;
  }
  return queue;
}

// static, public
PortQueueFields PortQueueFields::fromThrift(
    state::PortQueueFields const& queueThrift) {
  PortQueueFields queue(static_cast<uint8_t>(queueThrift.id));

  auto const itrStreamType = cfg::_StreamType_NAMES_TO_VALUES.find(
      queueThrift.streamType.c_str());
  CHECK(itrStreamType != cfg::_StreamType_NAMES_TO_VALUES.end());
  queue.streamType = itrStreamType->second;

  auto const itrSched = cfg::_QueueScheduling_NAMES_TO_VALUES.find(
      queueThrift.scheduling.c_str());
  CHECK(itrSched != cfg::_QueueScheduling_NAMES_TO_VALUES.end());
  queue.scheduling = itrSched->second;

  queue.weight = queueThrift.weight;
  if (queueThrift.reserved) {
    queue.reservedBytes = queueThrift.reserved;
  }
  if (queueThrift.scalingFactor) {
    auto itrScalingFactor = cfg::_MMUScalingFactor_NAMES_TO_VALUES.find(
        queueThrift.scalingFactor->c_str());
    CHECK(itrScalingFactor != cfg::_MMUScalingFactor_NAMES_TO_VALUES.end());
    queue.scalingFactor = itrScalingFactor->second;
  }
  if (queueThrift.name) {
    queue.name = queueThrift.name;
  }
  if (queueThrift.packetsPerSec) {
    queue.packetsPerSec = queueThrift.packetsPerSec;
  }
  if (queueThrift.sharedBytes) {
    queue.sharedBytes = queueThrift.sharedBytes;
  }
  if (queueThrift.aqms) {
    for (const auto& aqm: queueThrift.aqms.value()) {
      queue.aqms.emplace(aqm.behavior, aqm);
    }
  }

  return queue;
}

PortQueue::PortQueue(uint8_t id) : ThriftyBaseT(id) {
}

bool comparePortQueueAQMs(
    const PortQueue::AQMMap& aqmMap,
    const std::vector<cfg::ActiveQueueManagement>& aqms) {
  if (aqmMap.size() != aqms.size()) {
    return false;
  }
  for (const auto& aqm: aqms) {
    auto aqmMapItr = aqmMap.find(aqm.behavior);
    if (aqmMapItr == aqmMap.end() || aqmMapItr->second != aqm) {
      return false;
    }
  }
  return true;
}

bool checkSwConfPortQueueMatch(
    const std::shared_ptr<PortQueue>& swQueue,
    const cfg::PortQueue* cfgQueue) {
  return swQueue->getID() == cfgQueue->id &&
         swQueue->getStreamType() == cfgQueue->streamType &&
         swQueue->getScheduling() == cfgQueue->scheduling &&
         (cfgQueue->scheduling == cfg::QueueScheduling::STRICT_PRIORITY ||
          swQueue->getWeight() == cfgQueue->weight) &&
         isPortQueueOptionalAttributeSame(swQueue->getReservedBytes(),
           cfgQueue->__isset.reservedBytes, cfgQueue->reservedBytes) &&
         isPortQueueOptionalAttributeSame(swQueue->getScalingFactor(),
           cfgQueue->__isset.scalingFactor, cfgQueue->scalingFactor) &&
         isPortQueueOptionalAttributeSame(swQueue->getPacketsPerSec(),
           cfgQueue->__isset.packetsPerSec, cfgQueue->packetsPerSec) &&
         isPortQueueOptionalAttributeSame(swQueue->getSharedBytes(),
           cfgQueue->__isset.sharedBytes, cfgQueue->sharedBytes) &&
         comparePortQueueAQMs(swQueue->getAqms(), cfgQueue->aqms);
}

template class NodeBaseT<PortQueue, PortQueueFields>;

}} // facebook::fboss
