/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_swapchain_info_
#define __tge_swapchain_info_

#include "info_template.h"

namespace tge {
  class swapchain_info : public info_template<vk::SwapchainCreateInfoKHR> {
  public:
    swapchain_info(
      vk::PhysicalDevice device,
      vk::SurfaceKHR surface,
      const uint32_t swapchain_images_num,
      const uint32_t W, const uint32_t H,
      const bool vsync, const bool triple_buffer,
      vk::SwapchainKHR old_swapchain = {}
    ) :
      info_template(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        swapchain_images_num,
        vk::Format::eB8G8R8A8Unorm,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        vk::Extent2D(W, H),
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        get_present_mode(device, surface, vsync, triple_buffer),
        true,
        old_swapchain
      )
    {}

    vk::PresentModeKHR get_present_mode(
      vk::PhysicalDevice device, vk::SurfaceKHR surface,
      const bool vsync, const bool triple_buffer
    );
  };
}

#endif  // __tge_swapchain_info_
