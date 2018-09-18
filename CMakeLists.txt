cmake_minimum_required(VERSION 3.0.0)
project(libuv-tutorial VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)
include_directories(${LIBUV_INCLUDE_DIRS})

add_executable(hello-world hello-world/main.c)
target_link_libraries(hello-world ${LIBUV_LIBRARIES})

add_executable(tcp-echo-server tcp-echo/server.c)
target_link_libraries(tcp-echo-server ${LIBUV_LIBRARIES})

add_executable(tcp-echo-client tcp-echo/client.c)
target_link_libraries(tcp-echo-client ${LIBUV_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
