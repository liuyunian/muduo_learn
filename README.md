# webserver
使用C++11实现Web服务器  
代码学习自：https://github.com/viktorika/Webserver

## 依赖
* 操作系统：Linux
* 编译器：g++ 7.4

## 编译
```
make
```

## 使用
```sh
./webserver [Options...]
    Options:
        -h          帮助
        -p <port>   指定监听端口
```

## 版本信息
### version 1
* 采用epoll ET模式
* 单线程处理多个客户端的请求
* 解决了基础的HTTP协议解析

### version 2
* 参考了muduo库加入了Reactor模式
* 默认采用epoll LT模式 -- 没有有效的数据表明ET模式比LT模式更高效
* 同时也支持poll -- 通过定义环境变量USE_POLL采用切换poll