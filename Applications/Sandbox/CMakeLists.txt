cmake_minimum_required(VERSION 3.25)

project(HelloTriangle LANGUAGES CXX)

# Get all GLSL files
file(
    GLOB_RECURSE
    GLSL_SHADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.glsl"
)

# Compile them all to SPIR-V
set(SPIRV_SHADERS "")
foreach(GLSL ${GLSL_SHADERS})
    get_filename_component(FILENAME ${GLSL} NAME_WE)
    set(SPV "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${FILENAME}.spv")
    list(APPEND SPIRV_SHADERS ${SPV})

    add_custom_command(
        OUTPUT ${SPV}
        COMMAND glslc ${GLSL} -o ${SPV}
        DEPENDS ${GLSL}
        COMMENT "Compile Shader: ${FILENAME}.glsl"
        VERBATIM
    )
endforeach()

# Gather all SPIR-V files in one target
add_custom_target(Shaders DEPENDS ${SPIRV_SHADERS})

# Get all source files
file(
    GLOB_RECURSE
    APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to an executable
add_executable(Sandbox ${APP_SOURCES})

# Link application against the engine
target_link_libraries(Sandbox PRIVATE Engine)

# Make application depend on the shaders
add_dependencies(Sandbox Shaders)
