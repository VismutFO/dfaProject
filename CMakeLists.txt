cmake_minimum_required(VERSION 3.0)
project(dfaProject) 

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_STANDARD 17)

link_directories(dfaProject ${PROJECT_SOURCE_DIR}/)
add_executable(dfaProject main.cpp dfa2re.cpp re2dfa.cpp)

set_target_properties(dfaProject PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
       target_link_libraries(dfaProject RegexCheckerCore_Darwin)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
       target_link_libraries(dfaProject RegexCheckerCore_Linux)
elseif(CMAKE_HOST_WIN32)
       target_link_libraries(dfaProject RegexCheckerCore_Windows)
endif()

install(TARGETS dfaProject DESTINATION .)
