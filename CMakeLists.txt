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

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

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
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/shaders)

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

find_program(SLANGC_COMPILER slangc REQUIRED)
if(NOT SLANGC_COMPILER)
    message(FATAL_ERROR "Slangc not found!")
endif()

function(compile_slang_shader 
    SOURCE_FILE
    ENTRY_POINT
    SHADER_STAGE
    OUTPUT_SPV
)
    set(STAGE_MAP_vertex vertex)
    set(STAGE_MAP_fragment fragment)
    set(STAGE_MAP_geometry geometry)
    set(STAGE_MAP_tesc hull)
    set(STAGE_MAP_tese domain)
    set(STAGE_MAP_compute compute)
    set(STAGE_MAP_mesh mesh)
    set(STAGE_MAP_task task)
    
    if(NOT DEFINED STAGE_MAP_${SHADER_STAGE})
        message(FATAL_ERROR "Unknown shader stage: ${SHADER_STAGE}")
    endif()
    
    set(SLANG_STAGE ${STAGE_MAP_${SHADER_STAGE}})
    
    add_custom_command(
        OUTPUT ${OUTPUT_SPV}
        COMMAND ${SLANGC_COMPILER}
            ${SOURCE_FILE}
            -entry ${ENTRY_POINT}
            -stage ${SLANG_STAGE}
            -target spirv
            -I ${SHADER_SRC_DIR}/include
            -o ${OUTPUT_SPV}
            -O2
        DEPENDS ${SOURCE_FILE}
        COMMENT "Compiling ${SOURCE_FILE} -> ${OUTPUT_SPV} [${ENTRY_POINT} / ${SLANG_STAGE}]"
        VERBATIM
    )
endfunction()

file(GLOB_RECURSE SLANG_SOURCES 
    RELATIVE ${SHADER_SRC_DIR}
    CONFIGURE_DEPENDS
    ${SHADER_SRC_DIR}/*.slang
)

function(find_all_entry_points SLANG_FILE OUT_VAR)
    file(READ ${SLANG_FILE} FILE_CONTENT)
    
    set(KNOWN_ENTRIES
        "vertMain"
        "fragMain"
        "geomMain"
        "ctrlMain"
        "evalMain"
        "computeMain"
        "main"
    )
    
    foreach(FUNC_NAME ${KNOWN_ENTRIES})
        string(FIND "${FILE_CONTENT}" "${FUNC_NAME}(" FOUND_POS)
        
        if(NOT FOUND_POS EQUAL -1)
            if(FUNC_NAME MATCHES "^(vertexMain|vertMain|main$)")
                set(STAGE "vertex")
                set(SHORT_STAGE "vert")
            elseif(FUNC_NAME MATCHES "^(fragmentMain|fragMain)")
                set(STAGE "fragment")
                set(SHORT_STAGE "frag")
            elseif(FUNC_NAME MATCHES "^(geomMain|geometryMain)")
                set(STAGE "geometry")
                set(SHORT_STAGE "geom")
            elseif(FUNC_NAME MATCHES "^(ctrlMain|hullMain)")
                set(STAGE "hull")
                set(SHORT_STAGE "ctrl")
            elseif(FUNC_NAME MATCHES "^(evalMain|domainMain)")
                set(STAGE "domain")
                set(SHORT_STAGE "eval")
            elseif(FUNC_NAME MATCHES "^(computeMain|compMain)")
                set(STAGE "compute")
                set(SHORT_STAGE "comp")
            else()
                continue()
            endif()
            
            list(APPEND ENTRIES "${STAGE}:${FUNC_NAME}:${SHORT_STAGE}")
        endif()
    endforeach()    
    set(${OUT_VAR} ${ENTRIES} PARENT_SCOPE)
endfunction()

set(SHADER_SPV_FILES "")

foreach(SLANG_REL_PATH ${SLANG_SOURCES})
    if(SLANG_REL_PATH MATCHES "^include/")
        continue()
    endif()
    
    set(SLANG_FILE ${SHADER_SRC_DIR}/${SLANG_REL_PATH})
    get_filename_component(SLANG_PATH ${SLANG_REL_PATH} DIRECTORY)
    get_filename_component(SLANG_NAME_WE ${SLANG_REL_PATH} NAME_WE)
    
    message(STATUS "Scanning: ${SLANG_REL_PATH}")
    
    find_all_entry_points(${SLANG_FILE} ENTRIES)
    
    if(ENTRIES)
        foreach(ENTRY ${ENTRIES})
            string(REPLACE ":" ";" ENTRY_PARTS ${ENTRY})
            list(GET ENTRY_PARTS 0 STAGE)
            list(GET ENTRY_PARTS 1 FUNC_NAME)
            list(GET ENTRY_PARTS 2 SHORT_STAGE)

            set(OUTPUT_SPV ${SHADER_OUTPUT_DIR}/${SLANG_PATH}/${SLANG_NAME_WE}/${SHORT_STAGE}.spv)
            
            get_filename_component(SPV_DIR ${OUTPUT_SPV} DIRECTORY)
            file(MAKE_DIRECTORY ${SPV_DIR})
            
            list(APPEND SHADER_SPV_FILES ${OUTPUT_SPV})
            
            compile_slang_shader(
                ${SLANG_FILE} 
                ${FUNC_NAME} 
                ${STAGE} 
                ${OUTPUT_SPV}
            )
        endforeach()
    else()
        message(WARNING "  No entry points found!")
    endif()
endforeach()

message(STATUS "Total shader files to compile: ${SHADER_SPV_FILES}")

add_custom_target(CompileShaders DEPENDS ${SHADER_SPV_FILES})
add_dependencies(${PROJECT_NAME} CompileShaders)