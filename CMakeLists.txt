cmake_minimum_required(VERSION 3.10)
project(http_server)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/HttpServer/include
    /usr/include/mysql-cppconn-8
    /usr/include/mysql
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# 查找必要的库
find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn mysqlcppconn8
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

# 添加 HttpServer 源文件
file(GLOB_RECURSE HTTP_SERVER_SRC
    "${PROJECT_SOURCE_DIR}/HttpServer/src/*.cpp"
)

# 主程序文件（假设 HttpServer 有 main.cpp，需根据实际情况调整）
set(MAIN_SRC "${PROJECT_SOURCE_DIR}/HttpServer/examples/test_client.cc")

# 添加可执行文件
add_executable(http_server
    ${MAIN_SRC}
    ${HTTP_SERVER_SRC}
)

# 链接库
target_link_libraries(http_server
    pthread
    muduo_net
    muduo_base
    mysqlcppconn
    mysqlclient
    ssl
    crypto
    ${CURL_LIBRARIES}
)

# 如果库不在默认路径，添加链接目录
link_directories(/usr/local/lib)

# 打印调试信息
message(STATUS "Include directories:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "  ${dir}")
endforeach()

message(STATUS "Main source file: ${MAIN_SRC}")
message(STATUS "HTTP Server sources: ${HTTP_SERVER_SRC}")
