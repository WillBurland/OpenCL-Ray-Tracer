<img src="https://img.shields.io/github/repo-size/Will1162/OpenCL-Ray-Tracer"/> <img src="https://img.shields.io/tokei/lines/github/Will1162/OpenCL-Ray-Tracer"/> <img src="https://img.shields.io/github/downloads/Will1162/OpenCL-Ray-Tracer/total"/> <img src="https://img.shields.io/github/last-commit/Will1162/OpenCL-Ray-Tracer"/>

# OpenCL Ray Tracer
	
## About the project

Build on top of [Ray-Tracer-Legacy](https://github.com/Will1162/Ray-Tracer-Legacy), which ran on the CPU, as a single thread. This was slow, so I decided to convert the entire codebase into an OpenCL project, so that it would run in parallel. The conversion process took some time, as I needed to learn the quirks of programming with OpenCL and GPUs in general, but this repository continues from where the legacy project was last committed to and has been improved upon greatly since then.

The initial inspiration and general implementation techniques came from Peter Shirley's [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html). As development platforms varied, the book using C++ a CPU workflow, and myself using OpenCL and a GPU workflow, I had to find ways to adapt Peter's code to work with my own codebase, especially as OpenCL does not yet support C++ features such as classes, as it is based on C99 and has many random quirks that can make it confusing to work with.

As the program runs, its output it displayed to a Win32 window, where blocks of pixels are filled in as they are finished. These blocks are customisable in size, but should only really be used in higher resolution renders. This is because at smaller resolutions, deploying more blocks causes more overhead than time saved, while in larger renders, there may be hardware limitations when allocating large buffers and GPU threads, which is where blocks are useful, to break up the workload.

## Sample image from a recent commit

![output](https://github.com/Will1162/OpenCL-Ray-Tracer/assets/39223201/a37c52e9-b17d-4278-ad58-98562e1c5657)

## Comparison to Legacy-Ray-Tracer

Note: The following comparison is likely out of date and performance will have changed as the project has been updated since this comparison was made, with both new features and performance improvements.
An equal test between the two now would not be a good comparison with how much the project has changed since then and was only possible initially as the two codebases were very similar.

### Output variables

| Attribute         | Value      |
|-------------------|------------|
| Output resolution | 1280 x 720 |
| Samples per pixel | 250        |
| Max bounce depth  | 50         |

### Scene environment

| Sphere # | X, Y, Z           | Radius | R, G, B       | Material   | Fuzz |
|----------|-------------------|--------|---------------|------------|------|
| 1        | 0.0, -100.5, -1.0 | 100.0  | 0.0, 0.8. 0.7 | Lambertian | N/A  |
| 2        | 0.0, 0.5, -1.0    | 0.5    | 1.0, 0.5, 0.3 | Lambertian | N/A  |
| 3        | -0.9, 0.0, -1.0   | 0.5    | 0.8, 0.5, 0.5 | Metal      | 0.1  |
| 4        | 0.9, 0.0, -1.0    | 0.5    | 0.8, 0.6, 0.2 | Metal      | 0.5  |
| 5        | 0.0, -0.3, -1.0   | 0.2    | 0.8, 0.8, 0.8 | Metal      | 0.0  |

### Results

| Device       | Time   |
|--------------|--------|
| CPU (Legacy) | ~122s  |
| GPU (OpenCL) | ~0.38s |

This equates to an approximate speed up of 320x for my hardware between the two variations of the program, although realistically, the test is not a fair comparison as the CPU version was single threaded, as by the time I was thinking about moving to a multi-threaded CPU version, I had already started the conversion to OpenCL, so I decided to continue with that instead.

### Output image

![output](https://user-images.githubusercontent.com/39223201/212554754-de0f2e15-93e3-49d4-ac89-50cbbbfc367e.png)

## To-do

- Texture support
- Scene descriptor JSON file
- Specular reflection
- Code cleanup and commenting
- Switch to CMake or similar build system, instead of a .bat file

## Notes

- Output resolution width must be a multiple of 128 (unsure as of *exactly* why)

## References & Inspiration

- [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html) by Peter Shirley
- [Coding Adventure: Ray Tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk) by Sebastian Lague
