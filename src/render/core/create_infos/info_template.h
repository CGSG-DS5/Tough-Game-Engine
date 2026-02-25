/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_info_template_info_
#define __tge_info_template_info_

#include "def.h"

namespace tge {
  template<typename T>
    class info_template {
    public:
      const T & get() const {
        return info;
      }

      T & get() {
        return info;
      }

    protected:
      template<typename... Vars>
        info_template(Vars&&... args) : info(std::forward<Vars>(args)...) {
        }

    private:
      T info;
    };

  std::vector<const char *> get_supported(
    const std::span<const char * const> &required,
    const std::span<const char * const> &supported
  );

  std::vector<const char *> get_unsupported(
    const std::span<const char * const> &required,
    const std::span<const char * const> &supported
  );
}

#endif  // __tge_info_template_info_
