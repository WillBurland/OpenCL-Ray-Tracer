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

file (GLOB ASSETS
	src/assets/img/*
	src/assets/models/*
	src/opencl/gpu_kernel.cl
)

file (COPY
	${ASSETS}
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)