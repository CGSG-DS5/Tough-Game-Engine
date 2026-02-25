/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_layers_
#define __tge_layers_

#include "def.h"

namespace tge {
  class layers : public info_template<std::vector<const char *>> {
  public:
    layers(const vk::raii::Context &ctx) :
      info_template(get_layers(ctx))
    {}

  private:
    std::vector<const char *> get_layers(const vk::raii::Context &ctx);
  };
}

#endif  // __tge_layers_
