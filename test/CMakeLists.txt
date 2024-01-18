# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/e4717df71a4f45bf9f0ac88c6cd9846a0bc248dd.zip)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(HASHTABLE_TESTS
    hashtable_test.cc
    queue_test.cc
    )

add_executable(hashtable_test ${HASHTABLE_TESTS})

target_link_libraries(
    hashtable_test
    hashtable
    gtest_main
    )

include(GoogleTest)
gtest_discover_tests(hashtable_test)

