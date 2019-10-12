#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <memory>

#include <sys/epoll.h>

#include <tools/socket/ServerSocket.h>
#include <tools/socket/Socket.h>

#include "HttpConnection.h"
#include "reactor/EventLoop.h"
#include "reactor/Channel.h"

#define MAX_CONN_NUM 1024 // 支持的最大连接数

class Server{
public:
    Server(int port);
    ~Server() = default;

    void run();

private:
    void handle_connect();

    void handle_close(Socket* socket);

private:
    int m_port;
    int m_idlefd;                                       // 预留文件描述符
    ServerSocket m_serverSocket;

    EventLoop m_loop;
    Channel m_listenChannel;

    std::map<Socket*, HttpConnection*> m_connStore;  // http连接对象数组
};

#endif // SERVER_H_