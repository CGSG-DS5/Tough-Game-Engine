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
    layers() :
      info_template(std::initializer_list {"VK_LAYER_KHRONOS_validation"})
    {}
  };
}

#endif  // __tge_layers_