target_sources(${PROJECT_NAME}
    PRIVATE
        "infos.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "application_info.h"
        "debug_messenger_info.h"
        "device_extensions.h"
        "infos.h"
        "info_template.h"
        "instance_extensions.h"
        "layers.h"
        "queue_info.h"
        "swapchain_info.h"
        "validation_features.h"
)

