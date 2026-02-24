/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "def.h"

#include "create_infos/infos.h"

#include "core_exception.h"

namespace tge {
  class core {
  public:
    core(const std::span<const char * const> &required_extensions);

  private:
    vk::raii::Context context {};

    vk::raii::Instance instance;
    vk::raii::DebugUtilsMessengerEXT debug_messenger;
    vk::raii::PhysicalDevice physical_device;

    vk::raii::Instance create_instance(const std::span<const char * const> &required_extensions);
    vk::raii::DebugUtilsMessengerEXT create_debugger();
    vk::raii::PhysicalDevice create_physical_device();
  };
}

#endif  // __tge_core_h_
