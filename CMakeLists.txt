cmake_minimum_required(VERSION 3.10)
cmake_minimum_required(VERSION 3.10)
project(http_server)

# ================= 基础配置 =================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================= Vue/Node 构建配置 =================
option(CHATSERVER_BUILD_WEB "Build ChatServer Vue frontend with npm" ON)
set(CHATSERVER_WEB_DIR "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/web")
set(CHATSERVER_WEB_DIST_DIR "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/resource/dist")

# ================= 第三方库 =================
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenCV REQUIRED)

# ================= ONNX Runtime =================
set(ONNXRUNTIME_DIR /home/gilove/onnxruntime-linux-x64-1.20.1)

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS ${ONNXRUNTIME_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime not found")
endif()

# ================= muduo =================
find_path(MUDUO_INCLUDE_DIR
    NAMES muduo/net/TcpServer.h
    PATHS /usr/local/include
)

find_library(MUDUO_NET_LIBRARY
    NAMES muduo_net
    PATHS /usr/local/lib
)

find_library(MUDUO_BASE_LIBRARY
    NAMES muduo_base
    PATHS /usr/local/lib
)

if(NOT MUDUO_INCLUDE_DIR OR NOT MUDUO_NET_LIBRARY OR NOT MUDUO_BASE_LIBRARY)
    message(FATAL_ERROR "muduo not found")
endif()

# ================= MySQL =================
find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn mysqlcppconn8
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

# ================= RabbitMQ / SimpleAmqpClient =================
find_path(SIMPLEAMQPCLIENT_INCLUDE_DIR
    NAMES SimpleAmqpClient/SimpleAmqpClient.h
    PATHS /usr/include /usr/local/include
)

find_library(SIMPLEAMQPCLIENT_LIBRARY
    NAMES SimpleAmqpClient
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

find_library(RABBITMQ_LIBRARY
    NAMES rabbitmq
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

if(NOT SIMPLEAMQPCLIENT_INCLUDE_DIR OR NOT SIMPLEAMQPCLIENT_LIBRARY OR NOT RABBITMQ_LIBRARY)
    message(FATAL_ERROR "SimpleAmqpClient or rabbitmq not found")
endif()

# ================= 头文件 =================
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/HttpServer/include
    ${PROJECT_SOURCE_DIR}/AIApps/ChatServer/include

    ${MUDUO_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}

    ${ONNXRUNTIME_DIR}/include

    /usr/include/mysql-cppconn-8
    /usr/include/mysql

    ${SIMPLEAMQPCLIENT_INCLUDE_DIR}
)

# ================= 源文件 =================
file(GLOB_RECURSE HTTP_SERVER_SRC
    "${PROJECT_SOURCE_DIR}/HttpServer/src/*.cpp"
)

file(GLOB_RECURSE GOMOKU_SERVER_SRC
    "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/*.cpp"
)

set(MAIN_SRC "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/main.cpp")

# ================= 可执行文件 =================
add_executable(http_server
    ${MAIN_SRC}
    ${HTTP_SERVER_SRC}
    ${GOMOKU_SERVER_SRC}
)

# ================= 链接库 =================
target_link_libraries(http_server
    pthread

    ${MUDUO_NET_LIBRARY}
    ${MUDUO_BASE_LIBRARY}

    mysqlcppconn
    mysqlclient

    ssl
    crypto

    ${CURL_LIBRARIES}
    ${OpenCV_LIBS}

    ${ONNXRUNTIME_LIBRARY}

    ${SIMPLEAMQPCLIENT_LIBRARY}
    ${RABBITMQ_LIBRARY}
)

# ================= 运行时路径 =================
set_target_properties(http_server PROPERTIES
    BUILD_RPATH "/usr/local/lib;${ONNXRUNTIME_DIR}/lib"
    INSTALL_RPATH "/usr/local/lib;${ONNXRUNTIME_DIR}/lib"
)

# ================= 调试信息 =================
message(STATUS "Muduo include: ${MUDUO_INCLUDE_DIR}")
message(STATUS "Muduo net: ${MUDUO_NET_LIBRARY}")
message(STATUS "Muduo base: ${MUDUO_BASE_LIBRARY}")
message(STATUS "ONNX Runtime: ${ONNXRUNTIME_LIBRARY}")
message(STATUS "OpenCV: ${OpenCV_LIBS}")

target_compile_definitions(http_server
    PRIVATE CHATSERVER_RESOURCE_DIR="${PROJECT_SOURCE_DIR}/AIApps/ChatServer/resource"
)

# ================= 前端构建（Node.js + Vue） =================
if(CHATSERVER_BUILD_WEB)
    if(NOT EXISTS "${CHATSERVER_WEB_DIR}/package.json")
        message(FATAL_ERROR "Vue frontend not found: ${CHATSERVER_WEB_DIR}/package.json")
    endif()

    find_program(NPM_EXECUTABLE npm REQUIRED)
    message(STATUS "Vue frontend build enabled: ${CHATSERVER_WEB_DIR}")

    add_custom_command(
        OUTPUT "${CHATSERVER_WEB_DIST_DIR}/index.html"
        COMMAND ${CMAKE_COMMAND} -E chdir "${CHATSERVER_WEB_DIR}" ${NPM_EXECUTABLE} install
        COMMAND ${CMAKE_COMMAND} -E chdir "${CHATSERVER_WEB_DIR}" ${NPM_EXECUTABLE} run build
        DEPENDS "${CHATSERVER_WEB_DIR}/package.json"
        COMMENT "Building Vue frontend assets..."
        VERBATIM
    )

    add_custom_target(chatserver_web_build DEPENDS "${CHATSERVER_WEB_DIST_DIR}/index.html")
    add_dependencies(http_server chatserver_web_build)
endif()