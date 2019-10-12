#include <errno.h>  // errno
#include <fcntl.h>  // open 

#include <tools/log/log.h>
#include <tools/socket/Socket.h>
#include <tools/socket/SocketsOps.h>
#include <tools/socket/InetAddress.h>
#include <tools/socket/ServerSocket.h>

#include "Server.h"

Server::Server(int port) : 
    m_port(port),
    m_idlefd(open("/dev/null", O_RDONLY | O_CLOEXEC)),
    m_serverSocket(sockets::create_nonblocking_socket(AF_INET)),
    m_listenChannel(&m_loop, m_serverSocket.get_sockfd())
{
    InetAddress addr(m_port);
    m_serverSocket.set_reuseAddr(true);
    m_serverSocket.bind(addr);

    m_listenChannel.set_readCallback(std::bind(&Server::handle_connect, this));
    m_listenChannel.enable_reading();
}

void Server::run(){
    m_serverSocket.listen();
    LOG_INFO("服务器启动");
    m_loop.loop();
}

void Server::handle_connect(){
    Socket* connSocket = m_serverSocket.accept_nonblocking(nullptr);
    if(connSocket == nullptr){
        if(errno == EMFILE){                                                // 进程描述符已达到上限
            sockets::close(m_idlefd);                                       // 关闭预留描述符，进程有了一个空闲描述符
            connSocket = m_serverSocket.accept_nonblocking(nullptr);        // 此时可以正确的接受连接
            delete connSocket;                                              // 服务器端优雅的关闭连接
            
            m_idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);             // 再次预留文件描述符
            return;
        }

        LOG_SYSERR("accept()函数执行出错");
        return;
    }

    Channel* connChannel = new Channel(&m_loop, connSocket->get_sockfd());
    HttpConnection * hc = new HttpConnection(connChannel, std::bind(&Server::handle_close, this, connSocket));
    m_connStore.insert({connSocket, hc});
}

void Server::handle_close(Socket* socket){
    auto iter = m_connStore.find(socket);
    assert(iter != m_connStore.end());

    delete iter->second;
    delete socket;
}