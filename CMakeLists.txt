cmake_minimum_required(VERSION 3.21)

project(hashtable_server VERSION 1.0.0)

configure_file(ServerConfig.h.in ServerConfig.h)

include(CTest)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options for libraries
option(USE_SERVER "Use the Server library" ON)
option(USE_GOOGLE_TEST "Use GoogleTest for testing" ON)

# Enable or disable to change concurrency policy
add_compile_definitions(COARSE_GRAINED_LOCKING)

# Server project library
if(USE_SERVER)
    add_subdirectory(server)
    list(APPEND EXTRA_LIBS server)
endif()

# GoogleTest
if(USE_GOOGLE_TEST)
    add_subdirectory(test)
endif()

# Server program
add_executable(${CMAKE_PROJECT_NAME} main.cc)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}"
    )

# Benchmark program
set(BENCHMARK_PROJECT_NAME benchmark)
add_executable(${BENCHMARK_PROJECT_NAME} benchmark.cc)

target_link_libraries(${BENCHMARK_PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(${BENCHMARK_PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}"
    )


