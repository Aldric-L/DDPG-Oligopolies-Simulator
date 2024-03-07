cmake_minimum_required(VERSION 3.27)
project(DDPG-Oligopolies-Simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SOURCES Simulator/LearningAgent.cpp
Simulator/LearningAgent.hpp
Simulator/SimulationManager.cpp
Simulator/SimulationManager.hpp
Simulator/main.cpp)
add_executable(DDPG-Oligopolies-Simulator ${SOURCES})

target_include_directories(DDPG-Oligopolies-Simulator PRIVATE Simulator/)

include_directories (Simulator/)
add_subdirectory(Simulator/AKML-lib/)
target_link_libraries(DDPG-Oligopolies-Simulator PRIVATE AKML)
target_include_directories(DDPG-Oligopolies-Simulator PRIVATE Simulator/AKML-lib/)
set_property(TARGET AKML PROPERTY CXX_STANDARD 20)
