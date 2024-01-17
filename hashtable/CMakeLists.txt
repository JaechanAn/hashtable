# Sources
set(HASHTABLE_SOURCE_DIR src)
set(HASHTABLE_SOURCES
    ${HASHTABLE_SOURCE_DIR}/hashtable.cc
    ${HASHTABLE_SOURCE_DIR}/shm.cc
    )

# Headers
set(HASHTABLE_HEADER_DIR include)
set(HASHTABLE_HEADERS
    ${HASHTABLE_HEADER_DIR}/hashtable.h
    ${HASHTABLE_HEADER_DIR}/shm.h
    )

add_library(hashtable STATIC ${HASHTABLE_HEADERS} ${HASHTABLE_SOURCES})

target_include_directories(hashtable
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${HASHTABLE_HEADER_DIR}"
    )

