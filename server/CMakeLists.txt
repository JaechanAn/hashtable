# Sources
set(SERVER_SOURCE_DIR src)
set(SERVER_SOURCES
    ${SERVER_SOURCE_DIR}/hashtable.cc
    ${SERVER_SOURCE_DIR}/shm.cc
    )

# Headers
set(SERVER_HEADER_DIR include)
set(SERVER_HEADERS
    ${SERVER_HEADER_DIR}/hashtable.h
    ${SERVER_HEADER_DIR}/shm.h
    )

add_library(server STATIC ${SERVER_HEADERS} ${SERVER_SOURCES})

target_include_directories(server
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${SERVER_HEADER_DIR}"
    )

