cmake_minimum_required(VERSION 3.10)
project(BPTTest)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含源码目录
include_directories(src)

# 添加库文件
add_library(bpt_lib SHARED 
    src/BPT.cpp
)

# 添加可执行程序
add_executable(bpt_main code.cpp)
target_link_libraries(bpt_main bpt_lib)

# 启用测试
# enable_testing()
# add_subdirectory(tests)