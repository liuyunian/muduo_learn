#include <iostream>

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <tools_cxx/log.h>

#include "Server.h"

static void set_nonblocking(int sockfd){
    int flags = fcntl(sockfd, F_GETFL, 0);                                  // 获取sockfd当前的标志
    if(flags < 0){
        LOG_SYSFATAL("调用fcntl(sockfd, F_GETFL, 0)获取套接字标志失败");
    }

    flags |= O_NONBLOCK;

    if(fcntl(sockfd, F_SETFL, flags)){                                      // 设置sockfd标志
        LOG_SYSFATAL("将套接字设置为非阻塞方式失败");
    }
}

static int create_listenSockfd(int port){
    int listenSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSockfd < 0){
        LOG_SYSFATAL("创建监听套接字失败");
    }

    set_nonblocking(listenSockfd);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
	if(setsockopt(listenSockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        LOG_SYSFATAL("调用setsockopt()设置监听套接字失败");
    }

    if(bind(listenSockfd, (struct sockaddr * )&serv_addr, sizeof(struct sockaddr)) < 0){
        LOG_SYSFATAL("监听套接字绑定地址失败");
    }

    return listenSockfd;
}

Server::Server(int port) : 
    m_port(port),
    m_listenSockfd(create_listenSockfd(m_port)),
    m_loop(new EventLoop),
    m_listenChannel(new Channel(m_loop.get(), m_listenSockfd))
{
    m_listenChannel->set_readCallback(std::bind(&Server::handle_connect, this));
    m_listenChannel->enable_reading();
}

Server::~Server(){}

void Server::run(){
    if(listen(m_listenSockfd, BACKLOG) < 0){
        LOG_SYSFATAL("调用listen()函数失败");
    }

    LOG_INFO("服务器启动");
    m_loop->loop();
}

void Server::handle_connect(){
    int connfd = accept(m_listenSockfd, (struct sockaddr * )NULL, NULL);
    if(connfd < 0){
        LOG_SYSERR("accept()执行失败");
    }

    set_nonblocking(connfd);

    std::unique_ptr<Channel> connectionChannel(new Channel(m_loop.get(), connfd));
    HttpConnection * hc = new HttpConnection(connectionChannel);
    m_connectionStore.insert({connfd, hc});
}