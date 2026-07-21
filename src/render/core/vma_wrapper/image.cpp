/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Image::Image(
    const MemoryAllocator& alloc,
    const vk::raii::Device& device,
    vk::Image img,
    vk::Format fmt,
    vk::ImageViewType type,
    vk::ImageAspectFlags aspect,
    uint32_t mip_count,
    uint32_t layer_count
)
    : allocator(alloc)
    , image_handle(img)
    , image_view(create_image_view(device, fmt, type, aspect, mip_count, layer_count))
    , format(fmt) {}

vk::raii::ImageView tge::Image::create_image_view(
    const vk::raii::Device& device,
    vk::Format format,
    vk::ImageViewType type,
    vk::ImageAspectFlags aspect,
    uint32_t mip_count,
    uint32_t layer_count
) {
  return device.createImageView(
      {.image = image_handle,
       .viewType = type,
       .format = format,
       .subresourceRange =
           {.aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mip_count,
            .baseArrayLayer = 0,
            .layerCount = layer_count}}
  );
}

static VkImageType get_image_type(vk::Extent3D extent) {
  if (extent.height == 1) {
    return VK_IMAGE_TYPE_1D;
  }
  if (extent.depth == 1) {
    return VK_IMAGE_TYPE_2D;
  }
  return VK_IMAGE_TYPE_3D;
}

static vk::ImageViewType get_image_view_type(bool is_cube, vk::Extent3D extent) {
  if (is_cube) {
    return vk::ImageViewType::eCubeArray;
  }
  if (extent.height == 1) {
    return vk::ImageViewType::e1D;
  }
  if (extent.depth == 1) {
    return vk::ImageViewType::e2D;
  }
  return vk::ImageViewType::e3D;
}

static vk::ImageAspectFlags get_aspect(vk::Format fmt) {
  switch (fmt) {
  case vk::Format::eD32Sfloat:
    return vk::ImageAspectFlagBits::eDepth;
  case vk::Format::eD32SfloatS8Uint:
    return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
  default:
    return vk::ImageAspectFlagBits::eColor;
  }
}

static uint32_t get_mip_count(uint32_t mip_count, vk::Extent3D extent) {
  if (mip_count != 0) {
    return mip_count;
  }
  return 1u + static_cast<uint32_t>(std::log2(std::max(extent.width, extent.height)));
}

tge::Image::Image(
    const MemoryAllocator& alloc,
    const vk::raii::Device& device,
    vk::Format fmt,
    vk::Extent3D extent,
    uint32_t mip_count,
    vk::SampleCountFlagBits sample_count,
    vk::ImageUsageFlags usage,
    bool is_cube
)
    : allocator(alloc)
    , image_handle(create_image(alloc, device, fmt, extent, mip_count, sample_count, usage, is_cube))
    , image_view(create_image_view(
          device,
          fmt,
          get_image_view_type(is_cube, extent),
          get_aspect(fmt),
          get_mip_count(mip_count, extent),
          is_cube ? 6u : 1u
      ))
    , format(fmt)
    , image_sizes(extent) {}

VkImage tge::Image::create_image(
    const MemoryAllocator& alloc,
    const vk::raii::Device& device,
    vk::Format fmt,
    vk::Extent3D extent,
    uint32_t mip_count,
    vk::SampleCountFlagBits sample_count,
    vk::ImageUsageFlags usage,
    bool is_cube
) {
  assert(mip_count == 1);

  VkImageCreateInfo image_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
      .imageType = get_image_type(extent),
      .format = static_cast<VkFormat>(fmt),
      .extent = extent,
      .mipLevels = get_mip_count(mip_count, extent),
      .arrayLayers = is_cube ? 6u : 1u,
      .samples = static_cast<VkSampleCountFlagBits>(sample_count),
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = static_cast<VkImageUsageFlags>(usage),
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };

  VmaAllocationCreateInfo allocation_create_info{
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  VkImage image;
  if (VkResult res = vmaCreateImage(alloc, &image_create_info, &allocation_create_info, &image, &mem, nullptr);
      res != VK_SUCCESS) {
    throw CoreException("Image creation failure", res);
  }
  return image;
}

tge::Image::Image(Image&& other) noexcept
    : allocator(other.allocator)
    , image_handle(other.image_handle)
    , image_view(std::move(other.image_view))
    , mem(other.mem)
    , image_layout(other.image_layout)
    , format(other.format) {
  other.mem = VK_NULL_HANDLE;
}

tge::Image& tge::Image::operator=(Image&& other) noexcept {
  image_handle = other.image_handle;
  image_view = std::move(other.image_view);
  mem = other.mem;
  image_layout = other.image_layout;

  other.mem = VK_NULL_HANDLE;

  return *this;
}

tge::Image::~Image() {
  if (mem) {
    vmaDestroyImage(allocator, image_handle, mem);
  }
}

vk::ImageView tge::Image::get_image_view() const {
  return image_view;
}

vk::Image tge::Image::get_image() const {
  return image_handle;
}

vk::ImageLayout tge::Image::get_image_layout() const {
  return image_layout;
}

vk::Extent3D tge::Image::get_image_sizes() const {
  return image_sizes;
}

std::pair<vk::PipelineStageFlags2, vk::AccessFlags2> tge::Image::get_stage_acess(const vk::ImageLayout layout) {
  switch (layout) {
  case vk::ImageLayout::eUndefined:
  case vk::ImageLayout::eGeneral:
    return {vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone};
  case vk::ImageLayout::eColorAttachmentOptimal:
    return {vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite};
  case vk::ImageLayout::ePresentSrcKHR:
    return {vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone};
  case vk::ImageLayout::eDepthStencilAttachmentOptimal:
    return {
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite
    };
  case vk::ImageLayout::eTransferDstOptimal:
    return {vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite};
  case vk::ImageLayout::eShaderReadOnlyOptimal:
    return {vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead};
  case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
    return {
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eFragmentShader,
        vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eShaderRead
    };
  }

  return {};
}

void tge::Image::switch_layout(const vk::CommandBuffer cmd_buf, const vk::ImageLayout new_layout) {
  if (image_layout == new_layout) {
    return;
  }

  auto [src_stage, src_acess] = get_stage_acess(image_layout);
  auto [dst_stage, dst_acess] = get_stage_acess(new_layout);

  vk::ImageMemoryBarrier2 image_memory_barrier{
      .srcStageMask = src_stage,
      .srcAccessMask = src_acess,
      .dstStageMask = dst_stage,
      .dstAccessMask = dst_acess,
      .oldLayout = image_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
      .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
      .image = image_handle,
      .subresourceRange =
          {.aspectMask = get_aspect(format), .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
  };

  cmd_buf.pipelineBarrier2(
      vk::DependencyInfo{.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &image_memory_barrier}
  );

  image_layout = new_layout;
}
