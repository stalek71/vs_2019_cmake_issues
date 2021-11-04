# CMakeList.txt : CMake project for Mesh, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

# Include sub-projects.
add_subdirectory ("Cache")
add_subdirectory ("Calculation")
add_subdirectory ("Storage")
