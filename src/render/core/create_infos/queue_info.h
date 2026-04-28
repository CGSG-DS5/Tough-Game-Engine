/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_queue_info_
#define __tge_queue_info_

#include "info_template.h"

namespace tge {
  class QueueInfo : public InfoTemplate<vk::DeviceQueueCreateInfo> {
  public:
    QueueInfo(vk::PhysicalDevice device, vk::SurfaceKHR surface)
        : InfoTemplate(vk::DeviceQueueCreateFlags(), get_family_index(device, surface), priority) {}

  private:
    static const std::vector<float> priority;
    uint32_t get_family_index(vk::PhysicalDevice device, vk::SurfaceKHR surface);
  };
} // namespace tge

#endif // __tge_queue_info_
