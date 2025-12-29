#include "../../../include/utils/db/DbConnectionPool.h"
#include "../../../include/utils/db/DbException.h"
#include <muduo/base/Logging.h>

namespace http
{
    namespace db
    {

        void DbConnectionPool::init(const std::string& host,
            const std::string& user,
            const std::string& password,
            const std::string& database,
            size_t poolSize)
        {
            // 连接池会被多个线程访问，所以操作其成员变量时需要加锁
            std::lock_guard<std::mutex> lock(mutex_);
            // 确保只初始化一次
            if (initialized_)
            {
                return;
            }

            host_ = host;
            user_ = user;
            password_ = password;
            database_ = database;

            // 创建连接
            for (size_t i = 0; i < poolSize; ++i)
            {
                connections_.push(createConnection());
            }

            initialized_ = true;
            LOG_INFO << "Database connection pool initialized with " << poolSize << " connections";
        }

        DbConnectionPool::DbConnectionPool()
        {
            checkThread_ = std::thread(&DbConnectionPool::checkConnections, this);
            checkThread_.detach();
        }

        DbConnectionPool::~DbConnectionPool()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            while (!connections_.empty())
            {
                connections_.pop();
            }
            LOG_INFO << "Database connection pool destroyed";
        }

        /* ***************************************
        getConnection 方法从连接池中获取一个可用的数据库连接。
        如果没有可用连接，则会等待。如果获取连接前未初始化会抛出异常。

        获取到连接之后需要判断连接是否有效，
        如果失效则进行重连注意这里返回的智能指针的第二个参数是一个lambada表达式，
        代表重写std:shared ptr的析构函数(也就是重写智能指针的删除器函数)
        ******************************************* */
        std::shared_ptr<DbConnection> DbConnectionPool::getConnection()
        {
            std::shared_ptr<DbConnection> conn;
            {
                std::unique_lock<std::mutex> lock(mutex_);

                while (connections_.empty())
                {
                    if (!initialized_)
                    {
                        throw DbException("Connection pool not initialized");
                    }
                    LOG_INFO << "Waiting for available connection...";
                    cv_.wait(lock);
                }

                conn = connections_.front();
                connections_.pop();
            } // 释放锁

            try
            {
                // 在锁外检查连接
                if (!conn->ping())
                {
                    LOG_WARN << "Connection lost, attempting to reconnect...";
                    conn->reconnect();
                }

                return std::shared_ptr<DbConnection>(conn.get(),
                    [this, conn](DbConnection*) {
                        std::lock_guard<std::mutex> lock(mutex_);
                        connections_.push(conn);
                        cv_.notify_one();
                    });
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "Failed to get connection: " << e.what();
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    connections_.push(conn);
                    cv_.notify_one();
                }
                throw;
            }
        }

        std::shared_ptr<DbConnection> DbConnectionPool::createConnection()
        {
            return std::make_shared<DbConnection>(host_, user_, password_, database_);
        }

        // 修改检查连接的函数
        void DbConnectionPool::checkConnections()
        {
            while (true)
            {
                try
                {
                    std::vector<std::shared_ptr<DbConnection>> connsToCheck;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        if (connections_.empty())
                        {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            continue;
                        }

                        auto temp = connections_;
                        while (!temp.empty())
                        {
                            connsToCheck.push_back(temp.front());
                            temp.pop();
                        }
                    }

                    // 在锁外检查连接
                    for (auto& conn : connsToCheck)
                    {
                        if (!conn->ping())
                        {
                            try
                            {
                                conn->reconnect();
                            }
                            catch (const std::exception& e)
                            {
                                LOG_ERROR << "Failed to reconnect: " << e.what();
                            }
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(60));
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR << "Error in check thread: " << e.what();
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        }

    } // namespace db
} // namespace http