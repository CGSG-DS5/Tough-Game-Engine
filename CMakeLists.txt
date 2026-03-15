cmake_minimum_required(VERSION 3.16)
project(Tough-Game-Engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if(DEFINED ENV{SDL_PATH})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{SDL_PATH}")
    message(STATUS "Added SDL_PATH to CMAKE_PREFIX_PATH: $ENV{SDL_PATH}")
else()
    message(STATUS "SDL_PATH not found")
endif()

if(DEFINED ENV{VULKAN_SDK})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{VULKAN_SDK}")
    message(STATUS "Added VULKAN_SDK to CMAKE_PREFIX_PATH: $ENV{VULKAN_SDK}")
endif()

find_package(SDL3 REQUIRED CONFIG)

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:VALIDATION>
    $<$<CONFIG:RelWithDebInfo>:VALIDATION>
)

if (WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Spir-v compilation

set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/out/shaders)

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
file(GLOB_RECURSE SHADER_SOURCES 
    RELATIVE ${SHADER_SRC_DIR}
    ${SHADER_SRC_DIR}/*
)

foreach(SHADER_REL_PATH ${SHADER_SOURCES})
    if(SHADER_REL_PATH MATCHES "^include/.*" OR IS_DIRECTORY ${SHADER_SRC_DIR}/${SHADER_REL_PATH})
      continue()
    endif()

    set(SHADER_FILE ${SHADER_SRC_DIR}/${SHADER_REL_PATH})

    get_filename_component(SHADER_NAME_WE ${SHADER_REL_PATH} NAME_WE)
    get_filename_component(SHADER_PATH ${SHADER_REL_PATH} DIRECTORY)
    get_filename_component(SHADER_EXT_DOT ${SHADER_REL_PATH} EXT)
    string(SUBSTRING ${SHADER_EXT_DOT} 1 -1 SHADER_EXT)

    if(SHADER_PATH)
        set(SPV_FILE ${SHADER_OUTPUT_DIR}/${SHADER_PATH}/${SHADER_NAME_WE}.spv)
    else()
        set(SPV_FILE ${SHADER_OUTPUT_DIR}/${SHADER_NAME_WE}.spv)
    endif()

    if(${SHADER_NAME_WE} STREQUAL "ctrl")
        set(SPV_STAGE tesc)
    elseif(${SHADER_NAME_WE} STREQUAL "eval")
        set(SPV_STAGE tese)
    elseif(${SHADER_NAME_WE} STREQUAL "comp")
        set(SPV_STAGE compute)
    elseif(${SHADER_EXT} STREQUAL "glsl")
        set(SPV_STAGE ${SHADER_NAME_WE})
    else()
        set(SPV_STAGE ${SHADER_EXT})
    endif()

    get_filename_component(SPV_DIR ${SPV_FILE} DIRECTORY)
    file(MAKE_DIRECTORY ${SPV_DIR})

    list(APPEND SHADER_SPV_FILES ${SPV_FILE})

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND glslc -fshader-stage=${SPV_STAGE} ${SHADER_FILE} --target-spv=spv1.4 -I ${SHADER_SRC_DIR}/include -o ${SPV_FILE} #> ${SHADER_OUTPUT_DIR}/compile.log 2>&1
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${SHADER_REL_PATH} to SPIR-V"
        VERBATIM
    )
endforeach()

add_custom_target(CompileShaders DEPENDS ${SHADER_SPV_FILES})
add_dependencies(${PROJECT_NAME} CompileShaders)

