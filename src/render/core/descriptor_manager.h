/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_descriptor_manager_h_
#define __tge_descriptor_manager_h_

#include "core_exception.h"

namespace tge {
  enum struct DescriptorLayoutType : uint32_t {
    RENDER = 0,
    MATERIAL = 1,
    FINAL = 2,
  };

  class DescriptorManager {
  public:
    DescriptorManager(const vk::raii::Device& device, uint32_t frames_in_flight);

    const std::vector<vk::DescriptorSetLayout>& layouts() const;
    const std::vector<vk::DescriptorSet>& descriptor_sets(DescriptorLayoutType type) const;

  private:
    const vk::raii::Device& device;
    uint32_t frames_in_flight;

    std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_raii;
    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

    vk::raii::DescriptorPool descriptor_pool;

    std::map<DescriptorLayoutType, std::vector<vk::raii::DescriptorSet>> descriptor_sets_raii;
    std::map<DescriptorLayoutType, std::vector<vk::DescriptorSet>> m_descriptor_sets;

    static const std::map<DescriptorLayoutType, std::vector<vk::DescriptorSetLayoutBinding>>& get_layout_bindings();
    std::vector<vk::raii::DescriptorSetLayout> create_descriptor_set_layout_raii() const;
    std::vector<vk::DescriptorSetLayout> create_descriptor_set_layout() const;

    vk::raii::DescriptorPool create_descriptor_pool() const;

    std::map<DescriptorLayoutType, std::vector<vk::raii::DescriptorSet>> create_all_descriptor_sets_raii() const;
    std::vector<vk::raii::DescriptorSet> create_descriptor_sets(DescriptorLayoutType type) const;
    std::map<DescriptorLayoutType, std::vector<vk::DescriptorSet>> create_all_descriptor_sets() const;
  };
} // namespace tge

#endif // __tge_descriptor_manager_h
