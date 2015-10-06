cmake_minimum_required(VERSION 3.0)
project(openvz_daemon)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(LibMicroHTTPD REQUIRED)
include_directories(${MHD_INCLUDE_DIRS})


find_package(GnuTLS REQUIRED)
include_directories(${GNUTLS_INCLUDE_DIRS})

set(SOURCE_FILES main.c)
add_executable(openvz_daemon ${SOURCE_FILES})
target_link_libraries(openvz_daemon ${MHD_LIBRARIES})
target_link_libraries(openvz_daemon ${GNUTLS_LIBRARIES})
