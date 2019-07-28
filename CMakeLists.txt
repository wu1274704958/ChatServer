# CMakeList.txt: ChatServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
project(ChatServer)

set(CLIENT_EXEC_NAME "client")


# 将源代码添加到此项目的可执行文件。
add_executable (${PROJECT_NAME} ChatServer.cpp ChatServerImpl.cpp)

target_link_libraries(${PROJECT_NAME} ws2_32.lib)

add_executable(${CLIENT_EXEC_NAME} client.cpp ChatServerImpl.cpp)

target_link_libraries(${CLIENT_EXEC_NAME} ws2_32.lib)


# TODO: 如有需要，请添加测试并安装目标。
