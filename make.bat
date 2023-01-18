@echo off

cd build
del RayTracer-x64.exe
del output.bmp

copy ..\src\opencl\gpu_kernel.cl ..\build\gpu_kernel.cl

cls

echo Building...

g++ -std=c++17 -O2 ^
../src/cpp/main.cpp ^
../src/cpp/bitmap_io.cpp ^
../src/cpp/cl_vec3.cpp ^
../src/cpp/colour.cpp ^
../src/cpp/vec3.cpp ^
-I"C:\OpenCL\include" ^
-L"C:\OpenCL\lib" ^
-lOpenCL ^
-o RayTracer-x64.exe

echo Done
echo.

if exist RayTracer-x64.exe (
	RayTracer-x64.exe
	if exist output.bmp (
		output.bmp
	) else (
		echo.
		echo Program failed to output an image
	)
)

cd ../