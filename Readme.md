![screenshot of the game](screenshots/banner.png)

# AABBlocks

This project is intended to become a minimalistic voxel game, similar to minecraft in it's creative mode, built in vulkan and c++.

The code is it not intended to be scalable but rather to be good enough for the simple game I'm planning and to rapidly develop new features, for that reason the code does not use polymorfism, classes or custom destructors and constructors, and instead offers simple structs with manual data lifecycle management. Vulkan code is also very poorly abstracted, and some advanced features features like automatic dependency management are not intended to be developed.

The project however is intended to (and mainly developed for) have some graphics programming techniques that are used in modern games both for perfomance and visuals, and once a minimal functionality have been developed that will be the main goal of the project.

# [Video demonstration](https://www.youtube.com/watch?v=jWAyVscy1ZE)

# Features

Right now, some of the features of the game are:

* World persistence with sqlite databases.
* GPU-side frustum culling for chunks with compute shaders.
* Simple UI with ImGUI.
* Basic FPS controls (camera, moving, placing/removing blocks).
* Multitreading in world generation and physics simulation.

The features I'm planning to add are:

* Better world generation with structures like trees and caves.
* Deferred rendering and light volumes.
* Water with FBM waves and SSR reflections.
* LOD system for chunks.
* PBR materials.

# Build

Despite having been built on CMake and having all it's dependencies explicity included, I have not yet tested in other device than my Ubuntu machine. Given that I'm not giving any information on how to built it right now.