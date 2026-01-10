cmake_minimum_required(VERSION 3.1)
project(WhisperServer)

# Opsi Build (Default OFF agar aman di Mac)
option(ENABLE_CUDA "Enable CUDA compilation" OFF)

find_package(oatpp REQUIRED)

set(SOURCES
    src/App.cpp
    src/controller/MyController.hpp
    src/service/AudioService.cpp
    src/AppConfig.hpp
)

if(ENABLE_CUDA)
    message(STATUS ">> BUILD MODE: GPU/CUDA (Production)")
    enable_language(CUDA)
    find_package(CUDA REQUIRED)
    list(APPEND SOURCES src/worker/GpuWorker.cu)
else()
    message(STATUS ">> BUILD MODE: CPU Mock (Development)")
    list(APPEND SOURCES src/worker/CpuMock.cpp)
endif()

add_executable(my-server ${SOURCES})
target_link_libraries(my-server oatpp::oatpp)

target_include_directories(my-server PUBLIC src)