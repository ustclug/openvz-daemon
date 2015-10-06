cmake_minimum_required(VERSION 3.0)
project(openvz_daemon)

find_package(PkgConfig REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(LibMicroHTTPD REQUIRED)
include_directories(${MHD_INCLUDE_DIRS})


find_package(GnuTLS REQUIRED)
include_directories(${GNUTLS_INCLUDE_DIRS})

pkg_search_module(JSON-C REQUIRED json-c)
include_directories(${JSON-C_INCLUDE_DIRS})

set(SOURCE_FILES main.c url_handle.c command_handle.c)
add_executable(openvz_daemon ${SOURCE_FILES})
target_link_libraries(openvz_daemon ${MHD_LIBRARIES})
target_link_libraries(openvz_daemon ${GNUTLS_LIBRARIES})
target_link_libraries(openvz_daemon ${JSON-C_LIBRARIES})
