# Rendu Documentation

![Example of included demos](docs/img/header.png)


Rendu is a rendering engine designed for experimentation. The computer graphics academic and industrial litterature is full of interesting techniques and approaches than can be cumbersome to implement without some basic building blocks. This project aims to provide those building blocks, along with examples of interesting methods or papers. It also contains more general demo applications, such as a small snake game or a gamepad configurator. Rendu requires OpenGL 4 and builds on macOS (main test machine), Windows (regular testing) and Linux. After cloning the Rendu repository (all resources are included), see the [Building](#building) section to get the engine running!


# Projects

## Physically based rendering

Deferred rendering and PBR demo. *Description to come.*

## Atmospheric scattering

Atmospheric scattering demo. *Description to come.*

## Snake game

A small game. *Description to come.*

## Image Filtering

Image processing filters. *Description to come.*

## Tools

Name  | Function
------------- | -------------
Image viewer  |  ![](docs/img/imageviewer.png) Basic image viewer and editor for LDR and HDR images, supporting rotations, channels toggling, color picking.
BRDF Estimator  | ![](docs/img/brdfpreproc.png) Compute data for image-based lighting from an environment map: pre-convolved irradiance, BRDF look-up table, ambient lighting spherical harmonics decomposition. 
Controller mapper |  ![](docs/img/controllermap.png) Interface to create and edit controller button/stick mappings.
Shader validator |  ![](docs/img/shadervalidator.png) Perform per-shader compilation against the GPU driver and reports errors in an IDE-compatible fashion.
Atmospheric scattering preprocess | ![](docs/img/atmopreproc.png)  Compute the atmosphere coefficients look-up table for the Atmospheric Scattering project.

# Features

On a more detailed level, here are the main features you will find in Rendu.

- Window and graphics context setup.
- GPU objects creation and management (shaders, textures, buffers).
- Resources handling and packing.
- Shader validation at compilation time.
- Input management with controllers support.
- 3D rendering, including per-fragment shading, normal maps, parallax mapping. 
- Lights: omni/spots/directional lights, variance shadow mapping.
- Environment lighting, using cubemaps, preconvolved irradiance maps and spherical harmonics.
- Linear lighting pipeline, with HDR, bloom, tonemapping and gamma correction.
- Screen space techniques: antialiasing, ambient occlusion.
- Image processing techniques, such as fast gaussian blur, Poisson inpainting, flood filling.
- 2D interface rendering (buttons, checkboxes) with support for font distance fields
- A raycaster CPU implementation using a bounding volume hierarchy.

## Planned

I would like to add some additional features to Rendu in the near future, mainly to get a better grasp of some techniques and allow for more experimentations.

- Screen-space reflections and shadows (raymarching against the depth buffer).
- Local parallax-corrected light probes for reflections.
- Rendering of a terrain and water using a procedural approach (Perlin/Worley/Fractal noise, maybe tesselation).
- Temporal Antialiasing with reprojection and clamping.
- Particle effects (updated on the GPU).
- Volumetric effects, such as godrays and lit fog. 
- Support interesting controllers (MIDI controllers, PS4 light bar and touchpad,...)

On a more down-to-earth level, some engineering tasks could also help improve the engine.

- Reorganize scenes and objects, which are strongly coupled to the deferred rendering demo.
- Avoid binary dependencies by integrating GLFW3 and NativeFileDialog as subprojects.
- Real-time cube maps could be rendered in multiple calls after culling objects, instead of layered rendering.
- Abstract interactions with OpenGL and/or move to Vulkan.

# Building

This project use `premake` ([premake.github.io](https://premake.github.io)) for generating the workspace and projects files.
Run

	premake5.exe [vs2017 | xcode | make | ...]
	
To generate the desired workspace in the `build` directory.

The documentation (access it at `docs/index.html`) relies on Doxygen being installed. Generate it with 

	premake5 docs

You can clean the build directory with

	premake5 clean

Two non-system dependencies are required by this framework: `glfw3` and `nfd`, binaries for both are provided for macOS and Windows. All other dependencies are compiled along with the projects.
