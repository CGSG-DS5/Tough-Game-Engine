/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_device_extensions_
#define __tge_device_extensions_

#include "def.h"

namespace tge {
  class device_extensions : public info_template<std::vector<const char *>> {
  public:
    device_extensions(vk::PhysicalDevice phys_device) :
        info_template(get_exts(phys_device)) {
    }

  private:
    static const std::vector<const char *> additional_exts;

    static std::vector<const char *> get_exts(vk::PhysicalDevice phys_device);
  };
}

#endif  // __tge_device_extensions_
