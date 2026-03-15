target_sources(${PROJECT_NAME}
  PRIVATE
    "core.cpp"
    "surface.cpp"
  PUBLIC FILE_SET HEADERS FILES
    "core.h"
    "core_exception.h"
    "surface.h"
)

add_subdirectory(create_infos)
add_subdirectory(vma_wrapper)

