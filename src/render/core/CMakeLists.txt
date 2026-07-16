target_sources(${PROJECT_NAME}
    PRIVATE
        "core.cpp"
        "surface.cpp"
        "render_pass.cpp"
        "vulkan_context.cpp"
        "device.cpp"
        "swapchain.cpp"
        "descriptor_manager.cpp"
        "command_manager.cpp"
        
    PUBLIC FILE_SET HEADERS FILES
        "core.h"
        "core_exception.h"
        "surface.h"
        "render_pass.h"
        "vulkan_context.h"
        "device.h"
        "swapchain.h"
        "descriptor_manager.h"
        "command_manager.h"
        )

add_subdirectory(create_infos)
add_subdirectory(vma_wrapper)
add_subdirectory(pipelines)

