/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Swapchain::Swapchain(
    vk::PhysicalDevice physical_device,
    const vk::raii::Device& device,
    vk::SurfaceKHR surface,
    const MemoryAllocator& allocator,
    bool vsync,
    bool triple_buffer
)
    : physical_device(physical_device)
    , device(device)
    , surface(surface)
    , allocator(allocator)
    , swapchain_present_mode(get_swapchain_present_mode(vsync, triple_buffer))
    , m_swapchain(create_swapchain())
    , m_swapchain_images(create_swapchain_images())
    , render_finished_semaphores(create_semaphores(static_cast<uint32_t>(m_swapchain_images.size())))
    , device_present_mask(device.getGroupPresentCapabilitiesKHR().presentMask[0]) {}

vk::Extent2D tge::Swapchain::screen_size() const {
  return m_screen_size;
}

vk::SwapchainKHR tge::Swapchain::swapchain() const {
  return m_swapchain;
}

tge::Image& tge::Swapchain::swapchain_image() {
  return m_swapchain_images[m_image_index];
}

const tge::Image& tge::Swapchain::swapchain_image() const {
  return m_swapchain_images[m_image_index];
}

uint32_t tge::Swapchain::num_of_images() const {
  return static_cast<uint32_t>(m_swapchain_images.size());
}

vk::Semaphore tge::Swapchain::semaphore() const {
  return render_finished_semaphores[m_image_index];
}

uint32_t tge::Swapchain::image_index() const {
  return m_image_index;
}

void tge::Swapchain::resize() {
  auto new_sizes = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
  if (m_screen_size == new_sizes) {
    return;
  }
  m_screen_size = new_sizes;

  m_swapchain_images.clear();

  m_swapchain = device.createSwapchainKHR(
      SwapchainInfo(physical_device, surface, m_screen_size, swapchain_present_mode, m_swapchain).get()
  );

  m_swapchain_images = create_swapchain_images();
}

void tge::Swapchain::acquire_next_image(vk::Semaphore image_available_semaphore) {
  auto [res, new_image_index] = device.acquireNextImage2KHR(
      {.swapchain = swapchain(),
       .timeout = UINT64_MAX,
       .semaphore = image_available_semaphore,
       .deviceMask = device_present_mask}
  );

  m_image_index = new_image_index;
  if (res != vk::Result::eSuccess) {
    throw CoreException("Acquire next image error", static_cast<int32_t>(res));
  }
}

vk::PresentModeKHR tge::Swapchain::get_swapchain_present_mode(const bool vsync, const bool triple_buffer) const {
  const std::vector<vk::PresentModeKHR> modes = physical_device.getSurfacePresentModesKHR(surface);

  bool immediate = false, mailbox = false;

  for (const vk::PresentModeKHR mode : modes) {
    switch (mode) {
    case vk::PresentModeKHR::eImmediate:
      immediate = true;
      break;

    case vk::PresentModeKHR::eMailbox:
      mailbox = true;
      break;
    }
  }

  if (!vsync) {
    return immediate ? vk::PresentModeKHR::eImmediate : vk::PresentModeKHR::eFifo;
  }

  if (triple_buffer) {
    return mailbox ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
  }

  return vk::PresentModeKHR::eFifo;
}

vk::raii::SwapchainKHR tge::Swapchain::create_swapchain() {
  m_screen_size = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;

  // trick to supress useless vulkan warnings
  auto tmp = physical_device.getSurfaceFormatsKHR(surface);

  return device.createSwapchainKHR(SwapchainInfo(physical_device, surface, m_screen_size, swapchain_present_mode).get()
  );
}

std::vector<tge::Image> tge::Swapchain::create_swapchain_images() const {
  const std::vector<vk::Image> imgs = m_swapchain.getImages();
  std::vector<tge::Image> res;
  res.reserve(imgs.size());

  for (const vk::Image& img : imgs) {
    res.emplace_back(allocator, device, img, vk::Format::eB8G8R8A8Unorm);
  }

  return res;
}

std::vector<vk::raii::Semaphore> tge::Swapchain::create_semaphores(uint32_t num) const {
  std::vector<vk::raii::Semaphore> result;
  result.reserve(num);

  for (int32_t i = 0; i < num; i++) {
    result.emplace_back(device.createSemaphore({}));
  }

  return result;
}
