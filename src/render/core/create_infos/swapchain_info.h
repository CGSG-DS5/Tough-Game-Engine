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
              {.surface = surface,
               .minImageCount = present_mode == vk::PresentModeKHR::eMailbox ? 3u : 2u,
               .imageFormat = vk::Format::eB8G8R8A8Unorm,
               .imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
               .imageExtent = extent,
               .imageArrayLayers = 1,
               .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
               .imageSharingMode = vk::SharingMode::eExclusive,
               .preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
               .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
               .presentMode = present_mode,
               .clipped = true,
               .oldSwapchain = old_swapchain}
          ) {}
  };
} // namespace tge

#endif // __tge_swapchain_info_
