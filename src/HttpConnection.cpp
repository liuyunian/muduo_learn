#include <iostream>

#include <sys/stat.h>   // stat
#include <errno.h>      // errno
#include <stdlib.h>     // getenv
#include <unistd.h>     // close
#include <string.h>     // memset
#include <sys/socket.h> // recv send
#include <fcntl.h>      // open
#include <sys/mman.h>

#include <tools_cxx/log.h>
#include <boost/filesystem.hpp>
 
#include "HttpConnection.h"

#define MAX_BUF_SZ 2048
#define DEFAULT_KEEP_ALIVE_TIME 10

static std::unordered_map<std::string,std::string> mime = {
    {".html",   "text/html"},
    {".avi",    "video/x-msvideo"},
    {".bmp",    "image/bmp"},
    {".c",      "text/plain"},
    {".doc",    "application/msword"},
    {".gif",    "image/gif"},
    {".gz",     "application/x-gzip"},
    {".htm",    "text/html"},
    {".ico",    "image/x-icon"},
    {".jpg",    "image/jpeg"},
    {".png",    "image/png"},
    {".txt",    "text/plain"},
    {".mp3",    "audio/mp3"},
    {"default", "text/html"}
};

HttpConnection::HttpConnection(std::unique_ptr<Channel> & channel) : 
    m_connectionChannel(std::move(channel)),
    m_sockfd(m_connectionChannel->get_fd()),
    m_pos(0)
{
    m_connectionChannel->set_readCallback(std::bind(&HttpConnection::handle_read, this));
    m_connectionChannel->enable_reading();
}

void HttpConnection::handle_read(){
    clear();

    ssize_t len = recv_msg();
    if(len < 0){
        LOG_SYSERR("接收数据出错");
        return;
    }
    else if(len == 0){
        // TODO: 连接断开
        LOG_INFO("连接断开");
        m_connectionChannel->disable_reading();
        m_connectionChannel->remove();

        return;
    }

    // std::cout << m_recvBuf << std::endl;

    parse_request();
    parse_header();

    process_request();
}

void HttpConnection::parse_request(){
    std::string content;

    if(get_content(content, " /") < 0){             // 获取http请求行中的method
        LOG_ERR("解析http请求行中的methed出错");
    }

    if(content == "GET"){
        m_http.method = GET;
    }
    else if(content == "POST"){
        m_http.method = POST;
    }
    else{
        m_http.method = OTHER_METHOD;
    }

    if(get_content(content, " ") < 0){              // 获取http请求行中的url
        LOG_ERR("解析http请求行中的url出错");
    }
    m_http.url = content;

    if(get_content(content, "\r\n") < 0){          // 获取http请求行中的http版本
        LOG_ERR("解析http请求行中的http版本出错");
    }

    if(content == "HTTP/1.0"){
        m_http.version = HTTPV1_0;
    }
    else if(content == "HTTP/1.1"){
        m_http.version = HTTPV1_1;
    }
    else{
        m_http.version = OTHER_VERSION;
    }
}

void HttpConnection::parse_header(){
    /*
        Host: localhost:10000
        Connection: keep-alive
        Cache-Control: max-age=0
        Accept-Language: zh-CN,zh;q=0.9

     * 如上是http请求头部部分数据，可以看出满足key-value格式，所以这里采用unordered_map来存储
    */
    
    std::string key, value;
    while(m_recvBuf[m_pos] != '\r' && m_recvBuf[m_pos + 1] != '\n'){
		if(get_content(key, ": ") < 0){
            LOG_ERR("解析http请求头部数据的key值出错");
        }

		if(get_content(value, "\r\n") < 0){
            LOG_ERR("解析http请求头部数据的value值出错");
        }

		m_http.header.insert({key, value});
	}
}

void HttpConnection::process_request(){
    if(m_http.method == GET){                               // 判断请求方式
        boost::filesystem::path curPath(::getenv("PWD"));
        const char * staticResourcePath = curPath.append("/").append(m_http.url).c_str();

        struct stat sbuf;                                   // 解析URL
        if(stat(staticResourcePath, &sbuf) < 0){
            LOG_SYSERR("访问的文件不存在");

            fill_response_header(HTTP_NO_FOUND, nullptr);
        }
        else{
            fill_response_header(HTTP_OK, &sbuf);

            int srcfd = open(staticResourcePath, O_RDONLY, 0);
            if(srcfd < 0){
                LOG_SYSERR("打开请求文件失败");
                return;
            }

            char * src_addr = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
		    close(srcfd);

            m_sendBuf += std::string(src_addr,src_addr+sbuf.st_size);
            munmap(src_addr,sbuf.st_size);
        }

        // std::cout << m_sendBuf << std::endl;

        if(send_msg() < 0){
            LOG_SYSERR("发送响应消息失败");
            return;
        }
    }
    else if(m_http.method == POST){
        // ... 暂时不处理
    }
    else{
        // ... 其他请求方式也暂不处理
    }
}

int HttpConnection::get_content(std::string & content, std::string sepa){
    int index = m_recvBuf.find(sepa, m_pos);
    if(index == std::string::npos){ // 没有找到指定的分隔符
        return -1;
    }

    content = m_recvBuf.substr(m_pos, index - m_pos);
    m_pos = index + sepa.size();

    return 0;
}

void HttpConnection::clear(){
    m_recvBuf.clear();
    m_sendBuf.clear();

    m_pos = 0;

    m_http.url.clear();
}

ssize_t HttpConnection::recv_msg(){
    ssize_t len = 0;
    char buf[MAX_BUF_SZ];
    for(;;){
        len = recv(m_sockfd, buf, MAX_BUF_SZ, 0);
        if(len < 0){
            if(errno == EINTR){                                             // 信号中断产生的错误
                continue;
            }
            else{
                return -1;
            }
        }
        else if(len == 0){                                                  // 连接断开
            return 0;
        }

        m_recvBuf += std::string(buf, buf + len);
        return len;
    }
}

void HttpConnection::fill_response_header(Http_stateCode stateCode, struct stat * p_sbuf){
    switch(m_http.version){
    case HTTPV1_0:
        m_sendBuf = "HTTP/1.0 ";
        break;
    case HTTPV1_1:
        m_sendBuf = "HTTP/1.1 ";
    default:
        break;
    }

    if(stateCode == HTTP_OK){
        m_sendBuf += "200 OK\r\n";

        std::string fileType;
        int dot_pos = m_http.url.find('.');
        if(std::string::npos == dot_pos){
            fileType = mime["default"];
        }
        else{
            fileType = mime[m_http.url.substr(m_http.url.find('.'))];
        }

        m_sendBuf += "Content-Type: " + fileType + "\r\n";
        m_sendBuf += "Content-Length: " + std::to_string(p_sbuf->st_size) + "\r\n";
    }
    else if(stateCode == HTTP_NO_FOUND){
        m_sendBuf += "404 NO Found\r\n";
        m_sendBuf += "Content-Type: text/html\r\n";
        m_sendBuf += "Content-Length: 0\r\n";
    }

    m_sendBuf += "Server: TestServer\r\n";

    if(m_http.header.find("Connection") != m_http.header.end() 
        && ("Keep-Alive" == m_http.header["Connection"]
        || "keep-alive" == m_http.header["Connection"]))
    {
        m_sendBuf += "Connection: Keep-Alive\r\n";
        m_sendBuf += "Keep-Alive: timeout=" + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    }

    m_sendBuf += "\r\n";
}

ssize_t HttpConnection::send_msg(){
    const char * sendBuf = m_sendBuf.c_str();
    size_t bufSize = m_sendBuf.size();
    ssize_t len = 0;
    while(bufSize > 0){
        len = send(m_sockfd, sendBuf, bufSize, 0);
        if(len < 0){
            if(errno == EINTR){
                len = 0;
            }
            else{
                return -1;
            }
        }

        bufSize -= len;
        sendBuf += len;
    }

    return bufSize;
}