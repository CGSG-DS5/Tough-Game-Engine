/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_validation_features_
#define __tge_validation_features_

#include "def.h"

namespace tge {
  class validation_features : public info_template<vk::ValidationFeaturesEXT> {
  public:
    validation_features() :
      info_template(enabled_features)
    {}

  private:
    static const std::vector<vk::ValidationFeatureEnableEXT> enabled_features;
  };
}

#endif  // __tge_validation_features_
