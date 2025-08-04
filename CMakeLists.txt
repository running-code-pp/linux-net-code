cmake_minimum_required(VERSION 3.10)
project(unixNet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
# Set the output directory for the libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
# Set the output directory for the archives
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
# Set the output directory for the object files
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(datetime_client src/tcp_socket/datetime_client.cpp)
add_executable(datetime_server src/tcp_socket/datetime_server.cpp)
add_executable(echo_client src/tcp_socket/echo_client.cpp)
add_executable(echo_server src/tcp_socket/echo_server.cpp)
add_executable(concurrent_echo_server src/tcp_socket/concurrent_echo_server.cpp)
add_executable(select_server src/select/single_process_select_echo_server.cpp)
add_executable(epoll_server src/epoll/epoll_echoserver.cpp)
add_executable(udp_server src/udp_socket/udp_echo_server.cpp)
add_executable(udp_client src/udp_socket/udp_client.cpp)
add_executable(ipv4_ipv6 src/superSocket/ipv4_ipv6.cpp)
add_executable(daemonProcess src/superSocket/daemonProcess.cpp)
add_executable(pthread_echo_server src/pthread/pthread_test.cpp)
add_executable(pthread_mutex src/pthread/pthread_mutex.cpp)
add_executable(pthread_condition src/pthread/pthread_conditon.cpp)
add_executable(byte_order src/linux_highperformance_code/byteOrder.cpp)
add_executable(cgi_modify src/linux_highperformance_code/superiofunction/cgi.cpp)
add_executable(writev_readv_test src/linux_highperformance_code/superiofunction/writev_readv_test.cpp)
add_executable(sendfile_test src/linux_highperformance_code/superiofunction/sendfile_test.cpp)
add_executable(mmap_test src/linux_highperformance_code/superiofunction/mmap_test.cpp)
add_executable(splice_test src/linux_highperformance_code/superiofunction/splice_test.cpp)
add_executable(tcp_udp src/linux_highperformance_code/tcp_udp.cpp)
add_executable(signal_super src/linux_highperformance_code/signal_super.cpp)
add_executable(handle_zombie_process src/multiprocess/handle_zombie_process.cpp)
add_executable(pipe_correspondence src/multiprocess/pipe_correspondence.cpp)
add_executable(sem_correspondence src/multiprocess/semaphore_correspondence.cpp)
add_executable(std_sem_test src/multiprocess/std_sem_test.cpp)
add_executable(shared_memory_test src/multiprocess/shared_memory_tesst.cpp)
add_executable(msg_queue_test src/multiprocess/msg_queue.cpp)
#add_subdirectory(${PROJECT_SOURCE_DIR}/src/netproxy)