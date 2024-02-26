cmake_minimum_required (VERSION 3.10)

project (OpenCL-Ray-Tracer)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DUNICODE")

set (SOURCE_FILES
	src/cpp/main.cpp
	src/cpp/bitmap_io.cpp
	src/cpp/colour.cpp
	src/cpp/utility.cpp
	src/cpp/window_utilities.cpp
	src/cpp/opencl_objects/cl_bounding_box.cpp
	src/cpp/opencl_objects/cl_camera.cpp
	src/cpp/opencl_objects/cl_material.cpp
	src/cpp/opencl_objects/cl_sphere.cpp
	src/cpp/opencl_objects/cl_triangle.cpp
	src/cpp/opencl_objects/cl_vec3.cpp
	src/cpp/lib/hdrloader.cpp
)

include_directories ("C:/OpenCL/include")
link_directories    ("C:/OpenCL/lib")

add_executable (RayTracer-x64 ${SOURCE_FILES})

target_link_libraries (RayTracer-x64
	OpenCL
	gdi32
)

# Assets to copy
file (GLOB ASSETS
	src/assets/img/*
	src/assets/models/*
	src/assets/scenes/*
	src/opencl/gpu_kernel.cl
)

# Remove output.bmp
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output_removed
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/output.bmp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output_removed
)

# Copy assets
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets_copied
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy
    ${ASSETS}
    ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/assets_copied
    DEPENDS ${ASSETS}
)

# Prepare files before build
add_custom_target(PrepareFiles
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output_removed ${CMAKE_CURRENT_BINARY_DIR}/assets_copied
)
add_dependencies(RayTracer-x64 PrepareFiles)

# Remove marker files after build
add_custom_command(
    TARGET RayTracer-x64
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BINARY_DIR}/output_removed
    ${CMAKE_CURRENT_BINARY_DIR}/assets_copied
)