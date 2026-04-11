/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_layers_
#define __tge_layers_

#include "def.h"

namespace tge {
  class Layers : public InfoTemplate<std::vector<const char *>> {
  public:
    Layers(const vk::raii::Context &ctx)
        : InfoTemplate(get_layers(ctx)) {}

  private:
    std::vector<const char *> get_layers(const vk::raii::Context &ctx);
  };
} // namespace tge

#endif // __tge_layers_
