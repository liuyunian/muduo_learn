#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <functional>
#include <memory>

#include <tools/base/Timestamp.h>

// Timer
typedef std::function<void()> TimerCallback;

// TCPConnection
class Buffer;
class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
typedef std::function<void(const TCPConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TCPConnectionPtr&)> CloseCallback;
typedef std::function<void(const TCPConnectionPtr&, Buffer*, Timestamp)> MessageCallback;
typedef std::function<void(const TCPConnectionPtr&)> WriteCompleteCallback; // 低水位标回调函数
typedef std::function<void(const TCPConnectionPtr&, size_t)> HighWaterMarkCallback;

#endif // CALLBACK_H_