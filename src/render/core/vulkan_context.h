/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "core_exception.h"

namespace tge {
  class VulkanContext {
  public:
    VulkanContext();

    const vk::raii::Instance& instance() const;
    const vk::raii::PhysicalDevice& physical_device() const;

  private:
    vk::raii::Context context{};
    vk::raii::Instance m_instance;
    vk::raii::DebugUtilsMessengerEXT m_debug_messenger;
    vk::raii::PhysicalDevice m_physical_device;

    vk::raii::Instance create_instance();
    vk::raii::DebugUtilsMessengerEXT create_debugger();
    vk::raii::PhysicalDevice create_physical_device();
  };
} // namespace tge
