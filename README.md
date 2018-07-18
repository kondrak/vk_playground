# Vulkan playground
My personal SDL2 stub project with 6DOF WASD camera for Vulkan experiments. Targetted at VS2015 development. Basic math functionality and simple debug overlay (FPS counter) included.

This is a download-and-get-running-asap type of a project for your quick development pleasure.

Building on Linux and MacOS
-----
Assuming that the Vulkan SDK is already downloaded and properly set up on your target platform:
- set the VULKAN_SDK environment variable pointing to the location of downloaded Vulkan SDK
- download and install SDL2 (`libsdl2-dev` 2.0.7 or higher for Linux, `SDL2.framework` 2.0.8 or higher for MacOS)
- run the Makefile (Linux) or XCode project (MacOS) to build the application