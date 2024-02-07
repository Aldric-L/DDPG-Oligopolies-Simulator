cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/AKML-lib/)

add_library(AKML
AKML.hpp
AKML_consts.hpp
UtilityLoops.hpp
GeneticAlgorithm.cpp
GeneticAlgorithm.hpp
Matrices.hpp
NeuralLayer.hpp
NeuralNetwork.hpp
NeuralFunctions.hpp
NeuralFunctions.cpp
Matrices/MatrixOperations.hpp
Matrices/StaticMatrix.hpp
Matrices/DynamicMatrix.hpp
Matrices/MatrixInterface.hpp
Matrices/Matrix.hpp)
add_subdirectory(AgentBasedUtilities/)


