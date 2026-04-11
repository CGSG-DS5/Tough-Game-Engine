/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_swapchain_info_
#define __tge_swapchain_info_

#include "info_template.h"

namespace tge {
  class SwapchainInfo : public InfoTemplate<vk::SwapchainCreateInfoKHR> {
  public:
    SwapchainInfo(
        vk::PhysicalDevice device,
        vk::SurfaceKHR surface,
        const vk::Extent2D extent,
        const vk::PresentModeKHR present_mode,
        vk::SwapchainKHR old_swapchain = {}
    )
        : InfoTemplate(
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
          ) {}
  };
} // namespace tge

#endif // __tge_swapchain_info_
