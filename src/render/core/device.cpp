/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::Device::Device(vk::raii::PhysicalDevice physical_device, std::span<Surface> surfaces)
    : device(create_device(physical_device, surfaces)) {}

tge::Device::operator const vk::raii::Device&() const {
  return device;
}

const vk::raii::Device& tge::Device::operator*() const {
  return device;
}

const vk::raii::Device* tge::Device::operator->() const {
  return &device;
}

vk::raii::Device
tge::Device::create_device(vk::raii::PhysicalDevice physical_device, std::span<Surface> surfaces) const {
  std::vector<vk::DeviceQueueCreateInfo> queue_info;
  for (uint32_t i = 0; i < surfaces.size(); i++) {
    queue_info.push_back(QueueInfo(physical_device, surfaces[i]).get());
  }

  auto device_exts = DeviceExtensions(physical_device).get();

  vk::PhysicalDeviceFeatures device_features{.fullDrawIndexUint32 = true, .geometryShader = true};
  vk::StructureChain<
      vk::DeviceCreateInfo,
      vk::PhysicalDeviceSynchronization2Features,
      vk::PhysicalDeviceDynamicRenderingFeatures>
      device_create_info{
          {
              .queueCreateInfoCount = static_cast<uint32_t>(queue_info.size()),
              .pQueueCreateInfos = queue_info.data(),
              .enabledExtensionCount = static_cast<uint32_t>(device_exts.size()),
              .ppEnabledExtensionNames = device_exts.data(),
              .pEnabledFeatures = &device_features,
          },
          {.synchronization2 = true},
          {.dynamicRendering = true}
      };

  return vk::raii::Device(physical_device, device_create_info.get<vk::DeviceCreateInfo>());
}
