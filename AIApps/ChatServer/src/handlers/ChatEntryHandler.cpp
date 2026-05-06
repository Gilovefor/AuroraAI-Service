#include "../include/handlers/ChatEntryHandler.h"



void ChatEntryHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    std::string reqFile;
#ifdef CHATSERVER_RESOURCE_DIR
    reqFile = std::string(CHATSERVER_RESOURCE_DIR) + "/entry.html";
#else
    reqFile = "../AIApps/ChatServer/resource/entry.html";
#endif

    FileUtil fileOperater(reqFile);
    if (!fileOperater.isValid())
    {
        LOG_WARN << reqFile << " not exist";
        fileOperater.resetDefaultFile(); // 404 NOT FOUND
    }

    std::vector<char> buffer(fileOperater.size());
    fileOperater.readFile(buffer);
    std::string bufStr = std::string(buffer.data(), buffer.size());

    resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
    resp->setCloseConnection(false);
    resp->setContentType("text/html");
    resp->setContentLength(bufStr.size());
    resp->setBody(bufStr);
}
