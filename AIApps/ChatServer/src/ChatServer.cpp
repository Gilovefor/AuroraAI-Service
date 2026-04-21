#include "../include/handlers/ChatLoginHandler.h"
#include "../include/handlers/ChatRegisterHandler.h"
#include "../include/handlers/ChatLogoutHandler.h"
#include"../include/handlers/ChatHandler.h"
#include"../include/handlers/ChatEntryHandler.h"
#include"../include/handlers/ChatSendHandler.h"
#include"../include/handlers/AIMenuHandler.h"
#include"../include/handlers/AIUploadSendHandler.h"
#include"../include/handlers/AIUploadHandler.h"
#include"../include/handlers/ChatHistoryHandler.h"

#include"../include/handlers/ChatCreateAndSendHandler.h"
#include"../include/handlers/ChatSessionsHandler.h"
#include"../include/handlers/ChatSpeechHandler.h"

#include "../include/ChatServer.h"
#include "../../../HttpServer/include/http/HttpRequest.h"
#include "../../../HttpServer/include/http/HttpResponse.h"
#include "../../../HttpServer/include/http/HttpServer.h"

#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>
#include <filesystem>

using namespace http;

namespace
{
std::string getContentTypeByExtension(const std::filesystem::path& filePath)
{
    const std::string ext = filePath.extension().string();
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".webp") return "image/webp";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".woff") return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    return "application/octet-stream";
}

void writeStaticFileResponse(const http::HttpRequest& req,
                             http::HttpResponse* resp,
                             const std::filesystem::path& filePath,
                             const std::string& contentType,
                             const std::string& cacheControl)
{
    FileUtil fileOperater(filePath.string());
    if (!fileOperater.isValid())
    {
        const std::string notFound = "Not Found";
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k404NotFound, "Not Found");
        resp->setCloseConnection(false);
        resp->setContentType("text/plain; charset=utf-8");
        resp->setContentLength(notFound.size());
        resp->setBody(notFound);
        return;
    }

    std::vector<char> buffer(fileOperater.size());
    fileOperater.readFile(buffer);
    std::string body(buffer.data(), buffer.size());

    resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
    resp->setCloseConnection(false);
    resp->setContentType(contentType);
    resp->addHeader("Cache-Control", cacheControl);
    resp->setContentLength(body.size());
    resp->setBody(body);
}

std::string getEnvOrDefault(const char* key, const std::string& defaultValue)
{
    const char* value = std::getenv(key);
    return (value == nullptr || *value == '\0') ? defaultValue : std::string(value);
}

void ensureDatabaseSchema(const std::string& host,
                          const std::string& user,
                          const std::string& password,
                          const std::string& database)
{
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    std::unique_ptr<sql::Connection> conn(driver->connect(host, user, password));
    std::unique_ptr<sql::Statement> stmt(conn->createStatement());

    stmt->execute("CREATE DATABASE IF NOT EXISTS `" + database + "` "
                  "DEFAULT CHARACTER SET utf8mb4 "
                  "DEFAULT COLLATE utf8mb4_general_ci");
    conn->setSchema(database);

    stmt->execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
        "  username VARCHAR(64) NOT NULL,"
        "  password VARCHAR(255) NOT NULL,"
        "  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "  PRIMARY KEY (id),"
        "  UNIQUE KEY uk_users_username (username)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    stmt->execute(
        "CREATE TABLE IF NOT EXISTS chat_message ("
        "  msg_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
        "  id BIGINT UNSIGNED NOT NULL COMMENT 'user_id，与现有代码字段名兼容',"
        "  username VARCHAR(64) NOT NULL,"
        "  session_id VARCHAR(64) NOT NULL,"
        "  is_user TINYINT(1) NOT NULL,"
        "  content LONGTEXT NOT NULL,"
        "  ts BIGINT NOT NULL,"
        "  PRIMARY KEY (msg_id),"
        "  KEY idx_chat_message_user_session (id, session_id),"
        "  KEY idx_chat_message_session_ts_id (session_id, ts, id),"
        "  KEY idx_chat_message_ts_id (ts, id)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
}
} // namespace

ChatServer::ChatServer(int port,
    const std::string& name,
    muduo::net::TcpServer::Option option)
    : httpServer_(port, name, option)
{
    initialize();
}

void ChatServer::initialize() {
    std::cout << "ChatServer initialize start  ! " << std::endl;

    const std::string dbHost = getEnvOrDefault("CHAT_DB_HOST", "tcp://127.0.0.1:3306");
    const std::string dbUser = getEnvOrDefault("CHAT_DB_USER", "root");
    const std::string dbPassword = getEnvOrDefault("CHAT_DB_PASSWORD", "123456");
    const std::string dbName = getEnvOrDefault("CHAT_DB_NAME", "ChatHttpServer");

    ensureDatabaseSchema(dbHost, dbUser, dbPassword, dbName);
    http::MysqlUtil::init(dbHost, dbUser, dbPassword, dbName, 5);

    initializeSession();
    initializeMiddleware();
    initializeRouter();
}

void ChatServer::initChatMessage() {

    std::cout << "initChatMessage start ! " << std::endl;
    readDataFromMySQL();
    std::cout << "initChatMessage success ! " << std::endl;
}

void ChatServer::readDataFromMySQL() {


    std::string sql = "SELECT id, username,session_id, is_user, content, ts FROM chat_message ORDER BY ts ASC, id ASC";

    sql::ResultSet* res;
    try {
        res = mysqlUtil_.executeQuery(sql);
    }
    catch (const std::exception& e) {
        std::cerr << "MySQL query failed: " << e.what() << std::endl;
        return;
    }

    while (res->next()) {
        long long user_id = 0;
        std::string session_id ;  
        std::string username, content;
        long long ts = 0;
        int is_user = 1;

        try {
            user_id    = res->getInt64("id");       
            session_id = res->getString("session_id");  
            username   = res->getString("username");
            content    = res->getString("content");
            ts         = res->getInt64("ts");
            is_user    = res->getInt("is_user");
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to read row: " << e.what() << std::endl;
            continue; 
        }

        auto& userSessions = chatInformation[user_id];

        std::shared_ptr<AIHelper> helper;
        auto itSession = userSessions.find(session_id);
        if (itSession == userSessions.end()) {
            helper = std::make_shared<AIHelper>();
            userSessions[session_id] = helper;
			sessionsIdsMap[user_id].push_back(session_id);
        } else {
            helper = itSession->second;
        }

        helper->restoreMessage(content, ts);
    }

    std::cout << "readDataFromMySQL finished" << std::endl;
}



void ChatServer::setThreadNum(int numThreads) {
    httpServer_.setThreadNum(numThreads);
}


void ChatServer::start() {
    httpServer_.start();
}


void ChatServer::initializeRouter() {

    httpServer_.Get("/", std::make_shared<ChatEntryHandler>(this));
    httpServer_.Get("/entry", std::make_shared<ChatEntryHandler>(this));

    auto registerImageRoute = [this](const std::string& routePath, const std::string& fileName) {
        httpServer_.Get(routePath, [fileName](const http::HttpRequest& req, http::HttpResponse* resp) {
#ifdef CHATSERVER_RESOURCE_DIR
            std::filesystem::path imagePath = std::filesystem::path(CHATSERVER_RESOURCE_DIR).parent_path().parent_path() / "images" / fileName;
#else
            std::filesystem::path imagePath = std::filesystem::path("../AIApps/images") / fileName;
#endif
            FileUtil fileOperater(imagePath.string());
            if (!fileOperater.isValid())
            {
                const std::string notFound = "Image not found";
                resp->setStatusLine(req.getVersion(), http::HttpResponse::k404NotFound, "Not Found");
                resp->setCloseConnection(false);
                resp->setContentType("text/plain; charset=utf-8");
                resp->setContentLength(notFound.size());
                resp->setBody(notFound);
                return;
            }

            std::vector<char> buffer(fileOperater.size());
            fileOperater.readFile(buffer);
            std::string imageBody(buffer.data(), buffer.size());

            resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
            resp->setCloseConnection(false);
            resp->setContentType("image/jpeg");
            resp->setContentLength(imageBody.size());
            resp->setBody(imageBody);
        });
    };

    registerImageRoute("/assets/entry_pic.jpg", "entry_pic.jpg");
    registerImageRoute("/assets/404NotFound_pic.jpg", "404NotFound_pic.jpg");
    registerImageRoute("/assets/upload_pic.jpg", "upload_pic.jpg");

    httpServer_.addRoute(http::HttpRequest::kGet, "/assets/:file", [](const http::HttpRequest& req, http::HttpResponse* resp) {
        const std::string fileName = req.getPathParameters("param1");
#ifdef CHATSERVER_WEB_DIST_DIR
        const std::filesystem::path filePath = std::filesystem::path(CHATSERVER_WEB_DIST_DIR) / "assets" / fileName;
#else
        const std::filesystem::path filePath = std::filesystem::path("../AIApps/ChatServer/web/dist/assets") / fileName;
#endif
        writeStaticFileResponse(req, resp, filePath, getContentTypeByExtension(filePath), "public, max-age=31536000, immutable");
});

    httpServer_.Post("/login", std::make_shared<ChatLoginHandler>(this));
    httpServer_.Post("/register", std::make_shared<ChatRegisterHandler>(this));
    httpServer_.Post("/user/logout", std::make_shared<ChatLogoutHandler>(this));
    httpServer_.Get("/chat", std::make_shared<ChatHandler>(this));
    httpServer_.Post("/chat/send", std::make_shared<ChatSendHandler>(this));
    httpServer_.Get("/menu", std::make_shared<AIMenuHandler>(this));
    httpServer_.Get("/upload", std::make_shared<AIUploadHandler>(this));
    httpServer_.Post("/upload/send", std::make_shared<AIUploadSendHandler>(this));
    httpServer_.Post("/chat/history", std::make_shared<ChatHistoryHandler>(this));
    httpServer_.Post("/chat/send-new-session", std::make_shared<ChatCreateAndSendHandler>(this));
    httpServer_.Get("/chat/sessions", std::make_shared<ChatSessionsHandler>(this));
    httpServer_.Post("/chat/tts", std::make_shared<ChatSpeechHandler>(this));
}

void ChatServer::initializeSession() {

    auto sessionStorage = std::make_unique<http::session::MemorySessionStorage>();

    auto sessionManager = std::make_unique<http::session::SessionManager>(std::move(sessionStorage));

    setSessionManager(std::move(sessionManager));
}

void ChatServer::initializeMiddleware() {

    auto corsMiddleware = std::make_shared<http::middleware::CorsMiddleware>();

    httpServer_.addMiddleware(corsMiddleware);
}


void ChatServer::packageResp(const std::string& version,
    http::HttpResponse::HttpStatusCode statusCode,
    const std::string& statusMsg,
    bool close,
    const std::string& contentType,
    int contentLen,
    const std::string& body,
    http::HttpResponse* resp)
{
    if (resp == nullptr)
    {
        LOG_ERROR << "Response pointer is null";
        return;
    }

    try
    {
        resp->setVersion(version);
        resp->setStatusCode(statusCode);
        resp->setStatusMessage(statusMsg);
        resp->setCloseConnection(close);
        resp->setContentType(contentType);
        resp->setContentLength(contentLen);
        resp->setBody(body);

        LOG_INFO << "Response packaged successfully";
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "Error in packageResp: " << e.what();

        resp->setStatusCode(http::HttpResponse::k500InternalServerError);
        resp->setStatusMessage("Internal Server Error");
        resp->setCloseConnection(true);
    }
}
