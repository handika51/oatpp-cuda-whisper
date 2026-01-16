cmake_minimum_required(VERSION 3.1)
project(WhisperServer)

# Opsi Build (Default OFF agar aman di Mac)
option(ENABLE_CUDA "Enable CUDA compilation" OFF)
option(ENABLE_COVERAGE "Enable code coverage generation" OFF)

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
    set(WORKER_SRC src/worker/GpuWorker.cu)
else()
    message(STATUS ">> BUILD MODE: CPU Mock (Development)")
    list(APPEND SOURCES src/worker/CpuMock.cpp)
    set(WORKER_SRC src/worker/CpuMock.cpp)
endif()

add_executable(my-server ${SOURCES})
target_link_libraries(my-server oatpp::oatpp)

target_include_directories(my-server PUBLIC src)

# Add security hardening flags for Release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS ">> Adding security hardening flags for Release build")
    target_compile_options(my-server PRIVATE
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=2
    )
    target_link_options(my-server PRIVATE
        -Wl,-z,relro
        -Wl,-z,now
    )
endif()

##########################################################
# Testing
##########################################################
enable_testing()

add_executable(my-tests
    test/tests.cpp
    test/AudioServiceTest.cpp
    test/errorhandler/GlobalErrorHandlerTest.cpp
    src/service/AudioService.cpp
    ${WORKER_SRC} # Use same worker as main build
)

target_link_libraries(my-tests oatpp::oatpp oatpp::oatpp-test)
target_include_directories(my-tests PUBLIC src test)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS ">> Code coverage enabled")
        target_compile_options(my-tests PRIVATE --coverage -g -O0)
        target_link_options(my-tests PRIVATE --coverage)
        
        # Add coverage flags to the objects being tested as well if needed (or ensure they are built with coverage)
        # Since source files are compiled directly into my-tests, the flags above should cover them.

        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage_report
                COMMAND ${LCOV_PATH} --capture --directory . --output-file coverage.info --ignore-errors mismatch
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/test/*' '*/oatpp/*' --output-file coverage_filtered.info --ignore-errors unused,mismatch
                COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage_report
                COMMAND echo "Coverage report generated in coverage_report/index.html"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report..."
            )
        else()
            message(WARNING "lcov or genhtml not found. Coverage report target will not be available.")
        endif()
    else()
        message(WARNING "Code coverage requested but compiler is not GNU/Clang")
    endif()
endif()

add_test(NAME MyTests COMMAND my-tests)