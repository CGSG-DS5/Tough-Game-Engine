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

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
)

if (WIN32 AND TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3-shared>"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
