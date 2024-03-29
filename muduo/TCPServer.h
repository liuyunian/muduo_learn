#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>
#include <memory>
#include <map>
#include <atomic>

#include <tools/base/noncopyable.h>
#include <tools/socket/ConnSocket.h>
#include <tools/socket/InetAddress.h>

#include "muduo/Callbacks.h"

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

/**
 * TCP服务器，支持单线程和线程池模式
 * 这是一个接口类，所以不暴露过多的细节
 */
class TCPServer : noncopyable {
public:
  TCPServer(const std::string &name, const InetAddress &listenAddr, EventLoop *loop);

  ~TCPServer(); // // force out-line dtor, for scoped_ptr members.

  // get_xxx is_xxx set_xxx
  const std::string& get_name() const {
    return m_name;
  }

  const std::string& get_ip_and_port() const {
    return m_ipPort;
  }

  EventLoop* get_loop() const {
    return m_loop;
  }

  std::shared_ptr<EventLoopThreadPool> get_pool() const {
    return m_threadPool;
  }

  void set_number_of_threads(int numThreads);

  void set_connection_callback(const ConnectionCallback &cb){
    m_connectionCallback = cb;
  }

  void set_message_callback(const MessageCallback &cb){
    m_messageCallback = cb;
  }

  void set_writeCompleteCallback(const WriteCompleteCallback &cb){
    m_writeCompleteCallback = cb;
  }

  /**
   * 如何TCPServer没有在监听，就调用start
   * 线程安全
   * 多次调用是无害的
  */
  void start();

private:
  // 不是线程安全的，但是出于事件循环中
  void new_connection(ConnSocket connSocket, const InetAddress &peerAddr);

  void remove_connection(const TCPConnectionPtr &conn);

private:
  const std::string m_name;
  const std::string m_ipPort;
  std::atomic<bool> m_started;

  EventLoop *m_loop;
  std::unique_ptr<Acceptor> m_acceptor;
  std::shared_ptr<EventLoopThreadPool> m_threadPool;  // EventLoop线程池

  ConnectionCallback m_connectionCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;

  int m_nextConnId;
  std::map<std::string, TCPConnectionPtr> m_connections;
};

#endif // TCPSERVER_H_