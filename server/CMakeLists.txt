# Sources
set(SERVER_SOURCE_DIR src)
set(SERVER_SOURCES
    ${SERVER_SOURCE_DIR}/bpt.cc
    ${SERVER_SOURCE_DIR}/file.cc
    ${SERVER_SOURCE_DIR}/iputil.cc
    ${SERVER_SOURCE_DIR}/hashtable.cc
    )

# Headers
set(SERVER_HEADER_DIR include)
set(SERVER_HEADERS
    ${SERVER_HEADER_DIR}/bpt.h
    ${SERVER_HEADER_DIR}/file.h
    ${SERVER_HEADER_DIR}/iputil.h
    ${SERVER_HEADER_DIR}/hashtable.h
    )

add_library(server STATIC ${SERVER_HEADERS} ${SERVER_SOURCES})

target_include_directories(server
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${SERVER_HEADER_DIR}"
    )

