# 定义了SRC_LIST变量，包含当前目录下所有的源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})
# 指定可执行文件链接时需要的库文件
target_link_libraries(ChatClient pthread)