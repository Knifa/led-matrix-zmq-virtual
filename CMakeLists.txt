cmake_minimum_required(VERSION 3.1)
project(led-matrix-zmq-virtual
  LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#if (NOT CMAKE_BUILD_TYPE)
# set(CMAKE_BUILD_TYPE Release)
#endif()

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(led-matrix-zmq-virtual
  src/main.cpp
)

set_target_properties(led-matrix-zmq-virtual
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(led-matrix-zmq-virtual
  ${SDL2_LIBRARIES}
  zmq
)

target_compile_options(led-matrix-zmq-virtual PRIVATE
  -Wall
)

target_compile_features(led-matrix-zmq-virtual PUBLIC cxx_std_11)
