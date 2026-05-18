# AuroraAI-Service

基于 C++17 自建 HTTP 服务器框架的 AI 服务平台，集成多模型对话、图像识别、语音合成等功能。后端基于 muduo 网络库实现高性能异步通信，前端采用 Vue 3 构建单页应用，提供完整的 AI 交互体验。

---

## 环境配置

> 系统要求：Ubuntu 24

| 依赖 | 版本 |
|------|------|
| MySQL | 8.0.45 |
| OpenSSL | 3.0.13 |
| libcurl | 8.5.0 |
| OpenCV | 4.6.0 |
| ONNX Runtime | 1.20.1 |
| RabbitMQ | 3.12.1 |
| Node.js | v24.14.1 |
| CMake | 3.10+ |

RabbitMQ 相关依赖：
- SimpleAmqpClient
- librabbitmq

---

## 项目架构

```
AuroraAI-Service/
├── HttpServer/                  # 自建 HTTP 服务器框架（可复用库）
│   ├── include/
│   │   ├── http/                # HTTP 核心类型（请求解析、响应构建、服务器）
│   │   ├── middleware/          # 中间件链（CORS 跨域支持）
│   │   ├── router/              # 路由系统（静态路由 + 正则动态路由）
│   │   ├── session/             # 会话管理（Cookie + 内存存储）
│   │   ├── ssl/                 # SSL/TLS 支持（OpenSSL）
│   │   └── utils/               # 工具类（数据库连接池、文件、JSON）
│   └── src/                     # 实现文件
│
├── AIApps/
│   ├── ChatServer/              # 主应用：AI 聊天服务
│   │   ├── include/
│   │   │   ├── AIUtil/          # AI 工具层（策略工厂、模型调用、MCP、RAG）
│   │   │   └── handlers/        # 请求处理器（登录、聊天、上传等）
│   │   ├── src/                 # 实现文件
│   │   ├── resource/
│   │   │   ├── config.json      # MCP 工具配置
│   │   │   └── dist/            # 前端构建产物（后端静态托管）
│   │   └── web/                 # Vue 3 前端源码
│   │       └── src/views/       # 页面组件（登录、菜单、聊天、图片上传）
│   └── images/                  # 静态资源图片
│
└── CMakeLists.txt               # 构建配置
```

---

## 核心功能

### 自建 HTTP 服务器框架

基于 muduo 网络库实现的高性能 HTTP 服务器，采用 Reactor 事件驱动 + 多线程模型。

- **路由系统**：支持静态路由与正则动态路由，路径参数自动提取
- **中间件架构**：可插拔的中间件链，支持 CORS 跨域、日志、鉴权等扩展
- **会话管理**：基于 Cookie 的 Session 机制，内存存储，自动过期清理
- **HTTPS 支持**：集成 OpenSSL，支持 SSL 证书配置，保障通信安全
- **数据库连接池**：MySQL 连接复用，支持高并发场景下的数据库访问

### AI 多模型对话

采用策略模式（Strategy Pattern）实现多模型切换，通过工厂 + 静态自注册机制实现模型的即插即用。

| 模式 | 模型 | 说明 |
|------|------|------|
| 通义千问 | qwen-plus | 阿里云 DashScope，主对话模型 |
| 豆包 | doubao-seed-1-6-thinking | 字节跳动火山引擎，思维链推理 |
| RAG 知识库 | DashScope Knowledge Base | 基于阿里云知识库的检索增强生成 |
| MCP 工具调用 | qwen-plus + Tools | 支持天气查询、时间获取等工具扩展 |

### 图像识别

基于 ONNX Runtime 本地推理，使用 MobileNetV2 模型对上传图片进行分类识别，前端以 Base64 编码传输，后端通过 OpenCV 预处理后执行推理。

### 语音合成

集成百度语音 API，支持将 AI 回复文本转换为语音，提供多模态交互体验。

### 异步消息队列

通过 RabbitMQ 实现数据库写入的异步化处理。聊天消息先发布到消息队列，由消费者线程池异步写入 MySQL，降低 AI 对话响应延迟。

### MCP 工具调用

实现 Model Context Protocol 工具调用机制，采用两阶段 AI 调用模式：第一阶段由模型决定是否需要调用工具，第二阶段将工具执行结果注入上下文后生成最终回复。内置天气查询、时间获取等工具，可通过 `config.json` 扩展。

---

## 技术栈

**后端**
- C++17 / muduo / OpenSSL / libcurl / OpenCV / ONNX Runtime
- MySQL 8.0 + 连接池 / RabbitMQ 消息队列
- nlohmann/json / CMake + Ninja

**前端**
- Vue 3.5 (Composition API) / Vue Router 4.5 / Vite 5.4
- 自定义暗色主题 + 毛玻璃设计风格

**外部服务**
- 阿里云 DashScope（通义千问 / RAG 知识库）
- 字节跳动火山引擎（豆包大模型）
- 百度语音合成 API

---

## API 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/login` | 用户登录 |
| POST | `/register` | 用户注册 |
| POST | `/user/logout` | 用户登出 |
| POST | `/chat/send` | 发送消息（已有会话） |
| POST | `/chat/send-new-session` | 新建会话并发送消息 |
| GET | `/chat/sessions` | 获取会话列表 |
| POST | `/chat/history` | 获取历史消息 |
| POST | `/chat/tts` | 文本转语音 |
| POST | `/upload/send` | 上传图片进行识别 |

---

## 构建与运行

### 后端

```bash
# 创建构建目录
mkdir build && cd build

# 配置（使用 Ninja 构建器）
cmake -G Ninja ..

# 编译
ninja
```

### 前端

```bash
cd AIApps/ChatServer/web

# 安装依赖
npm install

# 构建（产物输出到 resource/dist/）
npm run build
```

### 启动服务

```bash
# 默认 8080 端口，4 个工作线程
./http_server

# 指定端口
./http_server -p 9090
```

### 环境变量

| 变量 | 说明 |
|------|------|
| `CHAT_DB_HOST` | MySQL 主机地址 |
| `CHAT_DB_USER` | MySQL 用户名 |
| `CHAT_DB_PASSWORD` | MySQL 密码 |
| `CHAT_DB_NAME` | 数据库名 |
| `DASHSCOPE_API_KEY` | 阿里云 DashScope API Key |
| `DOUBAO_API_KEY` | 火山引擎豆包 API Key |
| `Knowledge_Base_ID` | DashScope RAG 知识库 ID |
| `BAIDU_CLIENT_ID` | 百度语音 Client ID |
| `BAIDU_CLIENT_SECRET` | 百度语音 Client Secret |

---

## License

MIT
