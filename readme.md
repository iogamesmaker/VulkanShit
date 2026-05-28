# Vulkan Shit
Fucking about with Diligant engine to make a Vulkan renderer, end goal is some kind of microvoxel engine, if I get far, maybe a raymarcher / voxel traversal thing hybrid. Don't know. Very early stages :)

How to compile it?
 - Download SDL3 development libraries to your system, for Debian linux it's as easy as running `sudo apt install libsdl3-dev`.
 - Clone this repo, go to the root, and clone the DiligentCore repository into this project. `git clone https://github.com/DiligentGraphics/DiligentCore.git --recursive`
 - Run `cmake -S . -B build` to configure cmake
 - Try `cmake --build build -j$(nproc)`, it should compile 🙏
 
