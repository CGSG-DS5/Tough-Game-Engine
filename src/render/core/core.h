/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "def.h"

#include "create_infos/infos.h"

#include "core_exception.h"

#include "vma_wrapper/vma_allocator.h"

namespace tge {
  class core {
  public:
    core(SDL_Window *window);

    ~core();

  private:
    vk::raii::Context context {};

    vk::raii::Instance instance;
    vk::raii::DebugUtilsMessengerEXT debug_messenger;
    vk::raii::PhysicalDevice physical_device;
    vk::SurfaceKHR surface;
    vk::raii::Device device;
    vk::raii::Queue queue;
    vma_allocator allocator;

    vk::raii::Instance create_instance();
    vk::raii::DebugUtilsMessengerEXT create_debugger();
    vk::raii::PhysicalDevice create_physical_device();
    void create_suface(SDL_Window *window);
    vk::raii::Device create_device(SDL_Window *window);
    vk::raii::Queue create_queue();
    vma_allocator create_allocator();
  };
}

#endif  // __tge_core_h_
