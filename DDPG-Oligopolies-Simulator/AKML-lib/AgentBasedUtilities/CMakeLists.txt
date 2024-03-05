include_directories(${PROJECT_SOURCE_DIR}/AKML-lib/)
add_library(AKML_AgentBasedUtilities CSV_Saver.hpp Save.hpp BaseAgent.cpp BaseAgent.hpp)
set_property(TARGET AKML_AgentBasedUtilities PROPERTY CXX_STANDARD 20)
