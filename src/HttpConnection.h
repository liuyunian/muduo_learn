#ifndef HTTPCONNECTION_H_
#define HTTPCONNECTION_H_

#include <string>
#include <functional>
#include <unordered_map>

#include <sys/epoll.h>

#include "reactor/Channel.h"

enum Http_method{
    GET,
    POST,
    OTHER_METHOD
};

enum Http_version{
    HTTPV1_0,
    HTTPV1_1,
    OTHER_VERSION
};

enum Http_stateCode{
    HTTP_OK = 200,
    HTTP_NO_FOUND = 404
};

class HttpConnection{
public:
    typedef std::function<void()> CloseCallback;

    HttpConnection(Channel* channel, const CloseCallback& cb);
    ~HttpConnection();

    void handle_read();

    void process_request();

private:
    void clear();

    void parse_request();

    void parse_header();

    int get_content(std::string & content, std::string sepa);

    ssize_t recv_msg();

    ssize_t send_msg();

    void fill_response_header(Http_stateCode stateCode, struct stat * p_sbuf);

    struct HttpProtocol{
        Http_method method;
        std::string url;
        Http_version version;
        std::unordered_map<std::string, std::string> header;
    };

private:
    Channel* m_connChannel;
    CloseCallback m_closeCallback;

    std::string m_recvBuf;
    std::string m_sendBuf;

    HttpProtocol m_http;
    int m_pos;              // 记录解析http协议的位置
};

#endif // HTTPCONNECTION_H_