target_precompile_headers(${PROJECT_NAME} PRIVATE [["tge.h"]])

target_sources(${PROJECT_NAME}
  PRIVATE
    "main.cpp"
  PUBLIC FILE_SET HEADERS FILES
    "def.h"
)

add_subdirectory(win)
add_subdirectory(render)

