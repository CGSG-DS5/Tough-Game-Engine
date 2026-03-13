/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "create_infos/infos.h"
#include "surface.h"
#include "vma_wrapper/vma_allocator.h"

namespace tge {
  class core {
  public:
    core(SDL_Window *window, bool vsync, bool triple_buffer);
    ~core();

    void resize();

    void frame_start();
    void frame_end();

  private:
    const vk::raii::Context context {};

    const vk::raii::Instance instance;
    const vk::raii::DebugUtilsMessengerEXT debug_messenger;
    const vk::raii::PhysicalDevice physical_device;
    const raii_surface surface;
    const vk::raii::Device device;
    const uint32_t device_present_mask;
    const uint32_t queue_family_index;
    const vk::raii::Queue queue;
    const vma_allocator allocator;

    vk::Extent2D screen_size;
    const vk::PresentModeKHR swapchain_present_mode;
    const uint32_t frames_in_fligt_num;
    vk::raii::SwapchainKHR swapchain;
    std::vector<image> swapchain_images;

    const vk::raii::CommandPool command_pool;
    const vk::raii::DescriptorPool descriptor_pool;

    const std::vector<vk::raii::Fence> fences;
    const std::vector<vk::raii::Semaphore> image_available_semaphores;
    const std::vector<vk::raii::Semaphore> render_finished_semaphores;
    const std::vector<vk::raii::CommandBuffer> render_command_buffers;

    uint32_t frame_index {};
    uint32_t image_index {};

    vk::raii::Instance create_instance();
    vk::raii::DebugUtilsMessengerEXT create_debugger();
    vk::raii::PhysicalDevice create_physical_device();
    vk::raii::Device create_device(SDL_Window *window);
    uint32_t get_queue_family_index();
    vk::raii::Queue create_queue();
    vma_allocator create_allocator();

    vk::PresentModeKHR get_swapchain_present_mode(const bool vsync, const bool triple_buffer);
    vk::raii::SwapchainKHR create_swapchain();
    std::vector<image> create_swapchain_images();

    vk::raii::CommandPool create_command_pool();
    vk::raii::DescriptorPool create_descriptor_pool();

    std::vector<vk::raii::Fence> create_fences();
    std::vector<vk::raii::Semaphore> create_semaphores();
    std::vector<vk::raii::CommandBuffer> create_command_buffers();
  };
}

#endif  // __tge_core_h_
