/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_instance_extensions_
#define __tge_instance_extensions_

#include "def.h"

namespace tge {
  class InstanceExtensions : public InfoTemplate<std::vector<const char *>> {
  public:
    InstanceExtensions(const vk::raii::Context &ctx)
        : InfoTemplate(get_exts(ctx)) {}

  private:
    static const std::vector<const char *> additional_exts;

    static std::vector<const char *> get_exts(const vk::raii::Context &ctx);
  };
} // namespace tge

#endif // __tge_instance_extensions_
