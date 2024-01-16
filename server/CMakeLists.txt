# Sources
set(SERVER_SOURCE_DIR src)
set(SERVER_SOURCES
    ${SERVER_SOURCE_DIR}/hashtable.cc
    )

# Headers
set(SERVER_HEADER_DIR include)
set(SERVER_HEADERS
    ${SERVER_HEADER_DIR}/hashtable.h
    )

add_library(server STATIC ${SERVER_HEADERS} ${SERVER_SOURCES})

target_include_directories(server
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${SERVER_HEADER_DIR}"
    )

