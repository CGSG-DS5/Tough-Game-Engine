target_sources(${PROJECT_NAME}
    PRIVATE
        "render.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "render.h"
)

add_subdirectory(core)
