/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_instance_extensions_
#define __tge_instance_extensions_

#include "def.h"

namespace tge {
  class instance_extensions : public info_template<std::vector<const char *>> {
  public:
    instance_extensions(std::span<const char * const> required_exts, const vk::raii::Context &ctx) :
        info_template(get_exts(required_exts, ctx)) {
    }

  private:
    static const std::vector<const char *> additional_exts;

    static std::vector<const char *> get_exts(std::span<const char * const> required_exts, const vk::raii::Context &ctx);
  };
}

#endif  // __tge_instance_extensions_
