/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::ImageManager::ImageManager(
    const MemoryAllocator& allocator,
    const vk::raii::Device& device,
    vk::PhysicalDevice physical_device
)
    : allocator(allocator)
    , device(device)
    , samplers(create_samplers(physical_device)) {}

std::map<tge::ImageSamplerType, vk::raii::Sampler> tge::ImageManager::create_samplers(vk::PhysicalDevice physical_device
) const {
  std::map<ImageSamplerType, vk::raii::Sampler> result;
  vk::SamplerCreateInfo sampler_info{
      .magFilter = vk::Filter::eNearest,
      .minFilter = vk::Filter::eNearest,
      .mipmapMode = vk::SamplerMipmapMode::eNearest,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = false,
      .maxAnisotropy = physical_device.getProperties().limits.maxSamplerAnisotropy,
      .compareEnable = false,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.f,
      .maxLod = 0.f,
      .borderColor = vk::BorderColor::eFloatOpaqueBlack,
      .unnormalizedCoordinates = vk::False,
  };
  result.insert({ImageSamplerType::REPEAT, device.createSampler(sampler_info)});

  sampler_info.magFilter = sampler_info.minFilter = vk::Filter::eLinear;
  sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
  sampler_info.anisotropyEnable = true;
  sampler_info.maxLod = vk::LodClampNone;
  result.insert({ImageSamplerType::REPEAT_MIPMAP, device.createSampler(sampler_info)});

  sampler_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
  sampler_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
  sampler_info.addressModeW = vk::SamplerAddressMode::eClampToEdge;
  result.insert({ImageSamplerType::CLAMP_MIPMAP, device.createSampler(sampler_info)});

  sampler_info.magFilter = sampler_info.minFilter = vk::Filter::eNearest;
  sampler_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
  sampler_info.anisotropyEnable = false;
  sampler_info.maxLod = 0.f;
  result.insert({ImageSamplerType::CLAMP, device.createSampler(sampler_info)});

  return result;
}

vk::Sampler tge::ImageManager::sampler(ImageSamplerType type) const {
  return samplers.at(type);
}
