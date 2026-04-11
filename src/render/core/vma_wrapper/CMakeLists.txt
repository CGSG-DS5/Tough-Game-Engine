target_sources(${PROJECT_NAME}
    PRIVATE
        "buffer.cpp"
        "image.cpp"
        "vma.cpp"
    PUBLIC FILE_SET HEADERS FILES
        "buffer.h"
        "image.h"
        "vma_allocator.h"
)
