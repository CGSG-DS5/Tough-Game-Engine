/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_command_manager_h_
#define __tge_command_manager_h_

#include "core_exception.h"

namespace tge {
  /* Forward declarations */
  class Device;
  class Swapchain;

  class CommandManager {
  public:
    CommandManager(
        vk::PhysicalDevice physical_device,
        const Device& device,
        vk::SurfaceKHR surface,
        uint32_t frames_in_flight
    );

    uint32_t frame_index() const;

    void wait_idle() const;
    void wait_finishing() const;

    const vk::CommandBuffer& operator*() const;
    const vk::CommandBuffer* operator->() const;

    void submit(vk::Semaphore image_available_semaphore, vk::Semaphore render_finished_semaphore) const;
    void present(vk::Semaphore render_finished_semaphore, const Swapchain& swapchain) const;

  private:
    const Device& device;
    uint32_t frames_in_flight;

    uint32_t queue_family_index;
    vk::raii::Queue queue;

    vk::raii::CommandPool command_pool;

    std::vector<vk::raii::Fence> fences;
    std::vector<vk::raii::CommandBuffer> command_buffers;

    uint32_t m_frame_index{};

    uint32_t get_queue_family_index(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
    vk::raii::Queue create_queue();

    vk::raii::CommandPool create_command_pool();

    std::vector<vk::raii::CommandBuffer> create_command_buffers(uint32_t num) const;
  };
} // namespace tge

#endif // __tge_command_manager_h_
