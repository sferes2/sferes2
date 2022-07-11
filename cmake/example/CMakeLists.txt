cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(sferes2_example)
find_package(sferes2 REQUIRED)

add_executable(sferes2_example example.cpp)
target_compile_features(sferes2_example PRIVATE cxx_std_14)

target_link_libraries(sferes2_example
  sferes2
)
