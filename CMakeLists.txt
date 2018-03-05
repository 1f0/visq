cmake_minimum_required(VERSION 2.8)
project(visq)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

find_package(Eigen3 REQUIRED)
if (EIGEN3_FOUND)
	message("-- Eigen @ ${EIGEN3_INCLUDE_DIR}")
	message("-- Eigen version: ${EIGEN3_VERSION}")
	include_directories(${EIGEN3_INCLUDE_DIR})
endif (EIGEN3_FOUND)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

include_directories(antweakbar/include)

# set bin && lib
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(ARCHIVE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(src)
