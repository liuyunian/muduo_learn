# webserver
使用C++11实现Web服务器  
代码学习自：https://github.com/viktorika/Webserver

## 环境 & 依赖
* 操作系统：Linux
* 编译器：g++ 7.4
* boost库
* tools_cxx -- [自行封装的C++工具库](https://github.com/liuyunian/tools-cxx)

## 编译 & 安装
```
make
make install
```

## 使用
```
webserver [Options...]
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

### version 3
* 支持了favicon.icon
* 使用tools_cxx中封装的Socket重构了代码
* 修复了连接断开之后，部分资源没有释放的问题