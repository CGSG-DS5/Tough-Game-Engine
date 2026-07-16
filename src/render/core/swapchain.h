/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_swapchain_h
#define __tge_swapchain_h

#include "vma_wrapper/image.h"

namespace tge {
  class Swapchain {
  public:
    Swapchain(
        vk::PhysicalDevice physical_device,
        const Device& device,
        vk::SurfaceKHR surface,
        const MemoryAllocator& allocator,
        bool vsync,
        bool triple_buffer
    );

    vk::Extent2D screen_size() const;
    vk::SwapchainKHR swapchain() const;

    Image& swapchain_image();
    const Image& swapchain_image() const;
    uint32_t num_of_images() const;

    uint32_t image_index() const;

    void resize();

    void acquire_next_image(vk::Semaphore image_available_semaphore);

  private:
    vk::PhysicalDevice physical_device;
    const vk::raii::Device& device;
    vk::SurfaceKHR surface;
    const MemoryAllocator& allocator;

    vk::Extent2D m_screen_size;
    vk::PresentModeKHR swapchain_present_mode;
    vk::raii::SwapchainKHR m_swapchain;
    std::vector<Image> m_swapchain_images;
    uint32_t device_present_mask;

    uint32_t m_image_index{};

    vk::PresentModeKHR get_swapchain_present_mode(bool vsync, bool triple_buffer) const;
    vk::raii::SwapchainKHR create_swapchain();
    std::vector<Image> create_swapchain_images() const;
  };
} // namespace tge

#endif // __tge_swapchain_h
