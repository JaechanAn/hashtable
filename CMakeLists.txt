cmake_minimum_required(VERSION 3.21)

project(hashtable VERSION 1.0.0)

configure_file(HashTableConfig.h.in HashTableConfig.h)

include(CTest)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options for libraries
option(USE_HASHTABLE "Use the Hash Table library" ON)
option(USE_GOOGLE_TEST "Use GoogleTest for testing" ON)

# Enable or disable to change concurrency policy
#add_compile_definitions(COARSE_GRAINED_LOCKING)
add_compile_definitions(FINE_GRAINED_LOCKING)

# HashTable project library
if(USE_HASHTABLE)
    add_subdirectory(hashtable)
    list(APPEND EXTRA_LIBS hashtable)
endif()

# GoogleTest
if(USE_GOOGLE_TEST)
    add_subdirectory(test)
endif()

# Server program
set(SERVER_PROJECT_NAME server)
add_executable(${SERVER_PROJECT_NAME} server.cc)

target_link_libraries(${SERVER_PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(${SERVER_PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}"
    )

# Client program
set(CLIENT_PROJECT_NAME client)
add_executable(${CLIENT_PROJECT_NAME} client.cc)

target_link_libraries(${CLIENT_PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(${CLIENT_PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}"
    )

# Benchmark program
set(BENCHMARK_PROJECT_NAME benchmark)
add_executable(${BENCHMARK_PROJECT_NAME} benchmark.cc)

target_link_libraries(${BENCHMARK_PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(${BENCHMARK_PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}"
    )


