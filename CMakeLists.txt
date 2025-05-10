# CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(WebServer VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g ")

# 指定源文件
file(GLOB SOURCES
    "main.cpp"
    "server/*.cpp"
    "server/*.hpp"
#    "server/buffer/buffer.cpp"
    "server/buffer/modern_buffer.hpp"
    "server/loger/log.cpp"
    #"server/loger/log.hpp"
    "server/http/*.cpp"
)


# 可执行文件生成路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/bin)

# 生成可执行文件
add_executable(server ${SOURCES})

# 链接线程和 MySQL 客户端库
target_link_libraries(server pthread)
