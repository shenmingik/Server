# 最小支持版本
cmake_minimum_required(VERSION 3.0)
# 项目名称
project(main)

# 编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 头文件路径
#include_directories()

# 库文件路径
#link_directories()

# 设置源文件列表
set(SRC_LIST ./muduo_server.cpp)

# 设置可执行文件最终存储路径
set(EXECUTABLE_OUTPUT_PATH ../bin)

# 把当前所有文件放进SRC_LIST变量中
aux_source_directory(. SRC_LIST)

# 创建执行文件所需文件
add_executable(server ${SRC_LIST})

# 所需库文件，需要连接muduo_net muduo_base pthread三个库
target_link_libraries(server muduo_net muduo_base pthread)
