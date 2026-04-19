#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>   // atoi
#include <unistd.h>  // getpid, getopt
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include"../include/ChatServer.h"

const std::string RABBITMQ_HOST = "localhost";
const std::string QUEUE_NAME = "sql_queue";
const int THREAD_NUM = 2;

void executeMysql(const std::string sql) {
    http::MysqlUtil mysqlUtil_;
    mysqlUtil_.executeUpdate(sql);
}


int main(int argc, char* argv[]) {
	LOG_INFO << "pid = " << getpid();
	std::string serverName = "ChatServer";
	int port = 8080;
    // 
    int opt;
    const char* str = "p:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
    muduo::Logger::setLogLevel(muduo::Logger::WARN);
    ChatServer server(port, serverName);
    server.setThreadNum(4);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    server.initChatMessage();    


    RabbitMQThreadPool pool(RABBITMQ_HOST, QUEUE_NAME, THREAD_NUM, executeMysql);
    pool.start();

    server.start();
}

/*
第一步：启动入口（main.cpp）
重点：

看如何初始化 EventLoop（Muduo 的核心）。

看如何读取 SSL 配置并初始化 SslContext。

看如何实例化 ChatServer 并启动监听。

目的： 搞清楚整个服务器是怎么“活”起来的。

第二步：分发中枢（ChatServer.cpp）
重点：

寻找类似 setHttpCallback 或路由映射表的地方。

看看它是如何根据 URL（如 /login 或 /chat）来决定调用哪个 Handler 的。

目的： 理解“回调函数”在这里是如何作为路由分发器发挥作用的。

第三步：核心业务逻辑（handlers/）
建议先看 ChatLoginHandler，再看 ChatSendHandler。 重点：

登录流：如何处理 POST 数据，如何进行简单的身份验证。

AI 交互流：如何从 Request 中提取问题，如何调用 AIHelper 发起第三方请求，以及如何处理异步回调。

第四步：异步工具类（AIUtil/）
重点：

MQManager: 观察它是如何管理线程池的。在高并发下，如果 AI 响应慢，它是如何保证服务器不卡死的？

AIHelper: 学习如何用 C++ 封装第三方 HTTP API 调用（处理 JSON、Header 等）。

*/
