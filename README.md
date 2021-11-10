# Stellar: A 3D Vulkan renderer.

## Minimum Requirements (earlier versions might work, but are untested)

* gcc >= 11.1.0 
* Vulkan >= 1.2
* CMake >= 3.5
* GLFW: x11 >= 3.3.5-1
* GLM >= 0.9.9.8-1
* STB >= 20210401-1

## Building

0.  Install Dependencies
    * Ensure that you have CMake and git installed and accessible from a shell.
1.  Open a shell which provides git and clone the repository with:
    '''git clone https://github.com/dariusgrant/Stellar.git'''
2.  Go to the newly cloned Stellar directory.
3.  Create a new directory named '''build''' and change the current directory to the newly created folder.
4.  Create a build environment with CMake (note that STB include path should be specified in CMAKE_PREFIX_PATH)
    '''cmake -DCMAKE_PREFIX_PATH=/path/to/stb/include ./'''
5.  Either open the generated project with an IDE or launch the build process with '''cmake --build .'''.

## Running

Currently two samples are working: Triangle & Texture.

* To run the triangle sample, run '''./Triangle'''.
* To run the texture sample, run '''./Texture'''.
