# System packages required by an installed AlloLib.
# Bundled submodule targets are provided by AlloLibBundledTargets.cmake.
# A future package-manager layout would also find_dependency() glfw3, etc. here.
include(CMakeFindDependencyMacro)

find_dependency(OpenGL)
find_dependency(Threads)
