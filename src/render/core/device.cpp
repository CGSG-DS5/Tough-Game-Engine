/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::Device::Device(vk::raii::PhysicalDevice physical_device, vk::SurfaceKHR surface)
    : device(create_device(physical_device, surface)) {}

tge::Device::operator const vk::raii::Device&() const {
  return device;
}

const vk::raii::Device& tge::Device::operator*() const {
  return device;
}

const vk::raii::Device* tge::Device::operator->() const {
  return &device;
}

vk::raii::Device tge::Device::create_device(vk::raii::PhysicalDevice physical_device, vk::SurfaceKHR surface) const {
  auto queue_info = QueueInfo(physical_device, surface).get();
  auto device_exts = DeviceExtensions(physical_device).get();

  vk::PhysicalDeviceFeatures device_features{.fullDrawIndexUint32 = true, .geometryShader = true};
  vk::StructureChain<
      vk::DeviceCreateInfo,
      vk::PhysicalDeviceSynchronization2Features,
      vk::PhysicalDeviceDynamicRenderingFeatures>
      device_create_info{
          {
              .queueCreateInfoCount = 1,
              .pQueueCreateInfos = &queue_info,
              .enabledExtensionCount = static_cast<uint32_t>(device_exts.size()),
              .ppEnabledExtensionNames = device_exts.data(),
              .pEnabledFeatures = &device_features,
          },
          {.synchronization2 = true},
          {.dynamicRendering = true}
      };

  return vk::raii::Device(physical_device, device_create_info.get<vk::DeviceCreateInfo>());
}

std::vector<vk::raii::Fence> tge::Device::create_fences(uint32_t num) const {
  std::vector<vk::raii::Fence> result;
  result.reserve(num);

  for (int32_t i = 0; i < num; i++) {
    result.emplace_back(device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled}));
  }

  return result;
}

std::vector<vk::raii::Semaphore> tge::Device::create_semaphores(uint32_t num) const {
  std::vector<vk::raii::Semaphore> result;
  result.reserve(num);

  for (int32_t i = 0; i < num; i++) {
    result.emplace_back(device.createSemaphore(vk::SemaphoreCreateInfo()));
  }

  return result;
}
