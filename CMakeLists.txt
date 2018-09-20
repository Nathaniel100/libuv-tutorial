cmake_minimum_required(VERSION 3.0.0)
project(libuv-tutorial VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)
include_directories(${LIBUV_INCLUDE_DIRS})

add_executable(hello-world hello-world/main.c)
target_link_libraries(hello-world ${LIBUV_LIBRARIES})

add_executable(idle-basic idle-basic/main.c)
target_link_libraries(idle-basic ${LIBUV_LIBRARIES})

add_executable(tcp-echo-server tcp-echo/server.c)
target_link_libraries(tcp-echo-server ${LIBUV_LIBRARIES})

add_executable(tcp-echo-client tcp-echo/client.c)
target_link_libraries(tcp-echo-client ${LIBUV_LIBRARIES})

add_executable(udp-echo-server udp-echo/server.c)
target_link_libraries(udp-echo-server ${LIBUV_LIBRARIES})

add_executable(udp-echo-client udp-echo/client.c)
target_link_libraries(udp-echo-client ${LIBUV_LIBRARIES})

add_executable(daytime-server daytime/server.c)
target_link_libraries(daytime-server ${LIBUV_LIBRARIES})

add_executable(daytime-client daytime/client.c)
target_link_libraries(daytime-client ${LIBUV_LIBRARIES})

add_executable(uvcat uvcat/main.c)
target_link_libraries(uvcat ${LIBUV_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
