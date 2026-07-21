/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::DescriptorManager::DescriptorManager(const vk::raii::Device& device, uint32_t frames_in_flight)
    : device(device)
    , frames_in_flight(frames_in_flight)
    , descriptor_set_layouts_raii(create_descriptor_set_layout_raii())
    , descriptor_set_layouts(create_descriptor_set_layout())
    , descriptor_pool(create_descriptor_pool())
    , descriptor_sets_raii(create_all_descriptor_sets_raii())
    , m_descriptor_sets(create_all_descriptor_sets()) {}

std::span<const vk::DescriptorSetLayout> tge::DescriptorManager::layouts() const {
  return descriptor_set_layouts;
}

std::span<const vk::DescriptorSet> tge::DescriptorManager::descriptor_sets(DescriptorLayoutType type) const {
  return m_descriptor_sets.at(type);
}

const std::map<tge::DescriptorLayoutType, std::vector<vk::DescriptorSetLayoutBinding>>&
tge::DescriptorManager::get_layout_bindings() {
  static const std::map<DescriptorLayoutType, std::vector<vk::DescriptorSetLayoutBinding>> bindings{
      {DescriptorLayoutType::RENDER,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eUniformBuffer,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eAll},
        {.binding = 1,
         .descriptorType = vk::DescriptorType::eStorageBuffer,
         .descriptorCount = 3,
         .stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex}}},
      {DescriptorLayoutType::MATERIAL,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eCombinedImageSampler,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment}}},
      {DescriptorLayoutType::FINAL,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eCombinedImageSampler,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment}}}
  };

  return bindings;
}

std::vector<vk::raii::DescriptorSetLayout> tge::DescriptorManager::create_descriptor_set_layout_raii() const {
  const auto& layout_bindings = get_layout_bindings();

  std::vector<vk::raii::DescriptorSetLayout> res;
  for (const auto& [type, bindings] : layout_bindings) {
    vk::DescriptorSetLayoutCreateInfo info{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    res.emplace_back(device, info);
  }

  return res;
}

std::vector<vk::DescriptorSetLayout> tge::DescriptorManager::create_descriptor_set_layout() const {
  std::vector<vk::DescriptorSetLayout> result;
  result.reserve(descriptor_set_layouts_raii.size());
  for (const auto& layout : descriptor_set_layouts_raii) {
    result.push_back(*layout);
  }
  return result;
}

vk::raii::DescriptorPool tge::DescriptorManager::create_descriptor_pool() const {
  const auto& layout_bindings = get_layout_bindings();

  uint32_t max_sets = 0;
  std::vector<vk::DescriptorPoolSize> sizes;

  for (const auto& [type, bindings] : layout_bindings) {
    uint32_t num_sets = num_of_descriptor_sets(type);
    max_sets += num_sets;
    for (const auto& binding : bindings) {
      sizes.push_back({.type = binding.descriptorType, .descriptorCount = binding.descriptorCount * num_sets});
    }
  }

  return device.createDescriptorPool(
      {.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
       .maxSets = max_sets,
       .poolSizeCount = static_cast<uint32_t>(sizes.size()),
       .pPoolSizes = sizes.data()}
  );
}

std::map<tge::DescriptorLayoutType, std::vector<vk::raii::DescriptorSet>>
tge::DescriptorManager::create_all_descriptor_sets_raii() const {
  std::map<DescriptorLayoutType, std::vector<vk::raii::DescriptorSet>> res;
  for (uint32_t i = 0; i < descriptor_set_layouts.size(); i++) {
    auto type = static_cast<DescriptorLayoutType>(i);
    res.insert({type, create_descriptor_sets(type)});
  }

  return res;
}

uint32_t tge::DescriptorManager::num_of_descriptor_sets(DescriptorLayoutType type) const {
  switch (type) {
  case DescriptorLayoutType::FINAL:
  case DescriptorLayoutType::RENDER:
    return frames_in_flight;
  default:
    return 1;
  }
}

std::vector<vk::raii::DescriptorSet> tge::DescriptorManager::create_descriptor_sets(DescriptorLayoutType type) const {
  std::vector<vk::DescriptorSetLayout> layouts(
      num_of_descriptor_sets(type),
      descriptor_set_layouts[static_cast<uint32_t>(type)]
  );

  return device.allocateDescriptorSets(
      {.descriptorPool = descriptor_pool,
       .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
       .pSetLayouts = layouts.data()}
  );
}

std::map<tge::DescriptorLayoutType, std::vector<vk::DescriptorSet>> tge::DescriptorManager::create_all_descriptor_sets(
) const {
  std::map<DescriptorLayoutType, std::vector<vk::DescriptorSet>> result;
  for (const auto& [k, v] : descriptor_sets_raii) {
    result[k] = {};
    for (const auto& descriptor_set : v) {
      result[k].push_back(*descriptor_set);
    }
  }
  return result;
}
