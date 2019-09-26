#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <memory>

#include <sys/epoll.h>

#include <tools_cxx/Timestamp.h>

#include "HttpConnection.h"
#include "reactor/EventLoop.h"
#include "reactor/Channel.h"

#define MAX_CONN_NUM 1024 // 支持的最大连接数

#define BACKLOG 1024 // 用于指定listen第二个参数

class Server{
public:
    Server(int port);
    ~Server();

    void run();

private:
    void handle_connect();

private:
    int m_port;
    int m_listenSockfd;

    std::unique_ptr<EventLoop> m_loop;
    std::unique_ptr<Channel> m_listenChannel;

    std::map<int, HttpConnection *> m_connectionStore; // http连接对象数组
};

#endif // SERVER_H_