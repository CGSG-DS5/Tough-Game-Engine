/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::core::core(const std::span<const char * const> &required_extensions) :
    instance(create_instance(required_extensions)),
    debug_messenger(create_debugger()),
    physical_device(create_physical_device()),
    device(create_device())
{
}

vk::raii::Instance tge::core::create_instance(const std::span<const char * const> &required_extensions) {
  return context.createInstance(
    vk::InstanceCreateInfo(
      vk::InstanceCreateFlags(),
      &application_info().get(),
      layers(context).get(),
      instance_extensions(required_extensions, context).get()
    )
    .setPNext(&debug_messenger_info().get()
      .setPNext(&validation_features().get()))
  );
}

vk::raii::DebugUtilsMessengerEXT tge::core::create_debugger() {
  return instance.createDebugUtilsMessengerEXT(
    debug_messenger_info().get()
  );
}

static int64_t get_physical_device_score(const vk::raii::PhysicalDevice &device) {
  int64_t score = 0;

  vk::PhysicalDeviceProperties props = device.getProperties();

  if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 100000;
  }

  score += props.limits.maxImageDimension2D;

  return score;
}

vk::raii::PhysicalDevice tge::core::create_physical_device() {
  return std::ranges::max(instance.enumeratePhysicalDevices(), std::less(), get_physical_device_score);
}

vk::raii::Device tge::core::create_device() {
  return nullptr;
}
