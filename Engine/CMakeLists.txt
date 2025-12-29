cmake_minimum_required(VERSION 3.25)

project(Engine LANGUAGES CXX)

# Add source files
file(
    GLOB_RECURSE
    ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to a static library
add_library(Engine STATIC ${ENGINE_SOURCES})

# Configure fmt
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_FUZZ OFF CACHE BOOL "" FORCE)
set(FMT_CUDA_TEST OFF CACHE BOOL "" FORCE)
set(FMT_MODULE OFF CACHE BOOL "" FORCE)

# Add fmt
add_subdirectory("Vendor/fmt" EXCLUDE_FROM_ALL)

# Remove specific warnings for fmt
if(TARGET fmt)
    target_compile_options(
        fmt PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-result>
    )
endif()

# Configure glfw
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Add glfw
add_subdirectory("Vendor/glfw" EXCLUDE_FROM_ALL)

# Remove specific warnings for glfw
if(TARGET glfw)
    target_compile_options(
        glfw PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-Wno-pedantic>
        $<$<COMPILE_LANGUAGE:C>:-Wno-format-nonliteral>
        $<$<COMPILE_LANGUAGE:C>:-Wno-missing-field-initializers>
        $<$<COMPILE_LANGUAGE:C>:-Wno-sign-compare>
        $<$<COMPILE_LANGUAGE:C>:-Wno-format-nonliteral>
        $<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types-discards-qualifiers>
    )
endif()

# Check for Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK version: ${Vulkan_VERSION}")

# Link against everything
target_link_libraries(Engine PRIVATE fmt::fmt glfw PUBLIC Vulkan::Vulkan)

# Add linker flags
target_link_libraries(Engine PRIVATE ${ENGINE_LINKER_FLAGS})
