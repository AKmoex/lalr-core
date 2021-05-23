# cmake版本
cmake_minimum_required(VERSION 3.19)
# 项目名称
project(lalr-core)
# 编译版本
set(CMAKE_CXX_STANDARD 14)
# cmake tcp连接
link_libraries(ws2_32)

# 引入cpp-http库
add_library(cpp-httplib INTERFACE)
target_include_directories(cpp-httplib INTERFACE .)


add_executable(LALR main.cpp LALR.cpp LALR.h)
target_link_libraries(LALR cpp-httplib)


