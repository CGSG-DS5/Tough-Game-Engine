/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_application_info_
#define __tge_application_info_

#include "info_template.h"

#ifdef VK_API_VERSION_1_4
#define CURRENT_VULKAN_VERSION VK_API_VERSION_1_4
#else
#define CURRENT_VULKAN_VERSION VK_API_VERSION_1_3
#endif /* VK_API_VERSION_1_4 */

namespace tge {
  class ApplicationInfo : public InfoTemplate<vk::ApplicationInfo> {
  public:
    ApplicationInfo()
        : InfoTemplate(
              "Tough Game Engine Application",
              vk::makeApiVersion(0, 0, 1, 0),
              "Tough Game Engine",
              vk::makeApiVersion(0, 0, 1, 0),
              CURRENT_VULKAN_VERSION
          ) {}
  };
} // namespace tge

#undef CURRENT_VULKAN_VERSION

#endif // __tge_application_info_
