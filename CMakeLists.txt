# CMakeList.txt: ChatServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
project(ChatServer)

if(WIN32)

    add_compile_options("/std:c++17")
	add_compile_options("/permissive-")

else()

    add_compile_options("-std=c++17")

endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CLIENT_EXEC_NAME "client")

find_package(MINI_TEST)
find_package(MySql)
find_package(Sqlpp)

if(MINI_TEST_FOUND AND MYSQL_FOUND AND SQLPP_FOUND)

	include_directories(${MINI_TEST_INCLUDE_DIR})
	
	# 将源代码添加到此项目的可执行文件。
	add_executable (${PROJECT_NAME} ChatServer.cpp socket_win_impl.cpp tools/convert.cpp 
	handler/LoginHandler.cpp 
	ab_client.cpp 
	handler/RegHandler.cpp
	handler/ServerStateHandler.cpp
	handler/ModifyHandler.cpp)
	
	message("${MYSQL_INCLUDE_DIR} ${SQLPP_INCLUDE_DIR}")
	message("${MYSQL_LIBRARYS} ${SQLPP_LIBRARY}")

	target_include_directories(${PROJECT_NAME} PRIVATE ${MYSQL_INCLUDE_DIR} ${SQLPP_INCLUDE_DIR})
	
	target_link_libraries(${PROJECT_NAME} ws2_32.lib ${MYSQL_LIBRARYS} ${SQLPP_LIBRARY})
	
	add_executable(${CLIENT_EXEC_NAME} client.cpp socket_win_impl.cpp tools/convert.cpp)
	
	target_link_libraries(${CLIENT_EXEC_NAME} ws2_32.lib)

endif()

# TODO: 如有需要，请添加测试并安装目标。
