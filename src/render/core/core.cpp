/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::core::core(const std::span<const char * const> &required_extensions) :
    instance(create_instance(required_extensions)),
    debug_messenger(instance.createDebugUtilsMessengerEXT(debug_messenger_info().get())),
    physical_device(create_physical_device())
{
}

vk::raii::Instance tge::core::create_instance(const std::span<const char * const> &required_extensions) {
  return context.createInstance(
    vk::InstanceCreateInfo (
      vk::InstanceCreateFlags(),
      &application_info().get(),
      layers().get(),
      instance_extensions(required_extensions, context).get()
    )
    .setPNext(&debug_messenger_info().get()
      .setPNext(&validation_features().get()))
  );
}

vk::raii::DebugUtilsMessengerEXT tge::core::create_debugger() {
  return instance.createDebugUtilsMessengerEXT(
    debug_messenger_info().get()
  );
}

vk::raii::PhysicalDevice tge::core::create_physical_device() {
  return nullptr;
}