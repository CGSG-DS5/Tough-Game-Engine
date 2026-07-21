/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::VulkanContext::VulkanContext()
    : m_instance(create_instance())
    , m_debug_messenger(create_debugger())
    , m_physical_device(create_physical_device()) {}

const vk::raii::Instance& tge::VulkanContext::instance() const {
  return m_instance;
}

const vk::raii::PhysicalDevice& tge::VulkanContext::physical_device() const {
  return m_physical_device;
}

vk::raii::Instance tge::VulkanContext::create_instance() const {
  return context.createInstance(
      vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT, vk::ValidationFeaturesEXT>{
          {
              .pApplicationInfo = &ApplicationInfo().get(),
              .enabledLayerCount = static_cast<uint32_t>(Layers(context).get().size()),
              .ppEnabledLayerNames = Layers(context).get().data(),
              .enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions(context).get().size()),
              .ppEnabledExtensionNames = InstanceExtensions(context).get().data(),
          },
#ifdef VALIDATION
          DebugMessengerInfo().get(),
          ValidationFeatures().get()
#else
          {},
          {}
#endif // VALIDATION
      }
          .get()
  );
  return context.createInstance(vk::InstanceCreateInfo{
      .pApplicationInfo = &ApplicationInfo().get(),
      .enabledLayerCount = static_cast<uint32_t>(Layers(context).get().size()),
      .ppEnabledLayerNames = Layers(context).get().data(),
      .enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions(context).get().size()),
      .ppEnabledExtensionNames = InstanceExtensions(context).get().data(),
  }
#ifdef VALIDATION
                                    .setPNext(&DebugMessengerInfo().get().setPNext(&ValidationFeatures().get()))
#endif // VALIDATION
  );
}

vk::raii::DebugUtilsMessengerEXT tge::VulkanContext::create_debugger() const {
#ifdef VALIDATION
  return m_instance.createDebugUtilsMessengerEXT(DebugMessengerInfo().get());
#else // VALIDATION
  return nullptr;
#endif // VALIDATION
}

static int64_t get_physical_device_score(const vk::raii::PhysicalDevice& device) {
  int64_t score = 0;

  vk::PhysicalDeviceProperties props = device.getProperties();

  if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 100000;
  }

  score += props.limits.maxImageDimension2D;

  return score;
}

vk::raii::PhysicalDevice tge::VulkanContext::create_physical_device() const {
  return std::ranges::max(m_instance.enumeratePhysicalDevices(), std::less(), get_physical_device_score);
}
