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
      const vk::Extent2D extent,
      const vk::PresentModeKHR present_mode,
      vk::SwapchainKHR old_swapchain = {}
    ) :
      info_template(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        present_mode == vk::PresentModeKHR::eMailbox ? 3 : 2,
        vk::Format::eB8G8R8A8Unorm,
        vk::ColorSpaceKHR::eSrgbNonlinear,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        present_mode,
        true,
        old_swapchain
      )
    {}
  };
}

#endif  // __tge_swapchain_info_
