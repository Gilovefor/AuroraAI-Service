#pragma once
#include <string>

namespace ssl
{

    // SSL/TLS 协议版本
    enum class SSLVersion
    {
        TLS_1_0,
        TLS_1_1,
        TLS_1_2,
        TLS_1_3
    };

    // SSL 错误类型
    enum class SSLError
    {
        NONE,
        WANT_READ,
        WANT_WRITE,
        SYSCALL,
        SSL,
        UNKNOWN
    };

    // SSL 状态
    enum class SSLState
    {
        HANDSHAKE,
        ESTABLISHED,
        SHUTDOWN,
        ERROR
    };

} // namespace ssl