cmake_minimum_required(VERSION 3.0)
project(mprpc)


# 配置编译选项
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置最终可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜素路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜素路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含mprpc框架代码
add_subdirectory(src)

# example包含mprpc框架使用代码
add_subdirectory(example)