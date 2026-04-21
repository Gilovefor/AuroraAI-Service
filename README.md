# AuroraAI-Service
**在Http服务器的基础上搭建了AI服务，可以做一些基础的AI服务**
*** 
操作系统为Ubuntu 24，主要环境如下：
* muduo
* MySQL   (mysql Ver 8.0.45-0ubuntu0.24.04.1)
* OpenSSL（OpenSSL 3.0.13 30 Jan 2024）
* libcurl（curl 8.5.0）
* OpenCV （OpenCV 4.6.0）
* ONNX Runtime （1.20.1）
* RabbitMQ （rabbitmq-server 3.12.1-1ubuntu1.2 ）
	* SimpleAmqpClient
	* librabbitmq
* Node.js （v24.14.1.）

## Http Server
**基于muduo网络库实现高性能异步通，采用事件驱动和多线程模型，支持静态路由，动态路由，
  以及Handler映射机制，实现业务逻辑与网络层解耦。路由与业务处理分层设计采用Router+Handler架构
  将各种应用层业务拆分为独立的Handler，提升代码可维护性和可扩展性**

* 完善支持Https协议，集成SSL证书管理与加密通信，提升服务的数据安全与可靠性
* 可插拔中间件机制，支持日志记录、请求验证、性能监控等功能的灵活扩展
* 集成数据库连接池，支持多线程高并发访问，统一管理出库连接资源，提升数据库访问效率和系统性能

## AI Service
**基于ONNX Runtime实现AI推理服务，支持多种AI模型的加载与推理，
提供统一的API接口，方便业务层调用。通过优化模型加载和推理流程，提升AI服务的响应速度和吞吐量**

* 异步任务与消息解耦，封装消息队列接口，支持RabbitMQ等主流消息中间件，实现AI任务的异步处理和结果回调
* 多模态AI能力集成，支持文本、图像、语音等多种输入输出类型的AI模型，支持图片Base64编码传输，语音合成与识别功能
* 工程化与部署支持，使用Docker容器化部署，提供配置文件管理和环境变量支持，简化AI服务的部署和运维流程
