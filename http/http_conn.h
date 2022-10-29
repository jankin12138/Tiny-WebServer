//
// Created by Administrator on 2022/10/23.
//

#ifndef TINY_WEBSERVER_HTTP_CONN_H
#define TINY_WEBSERVER_HTTP_CONN_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    //设置读取文件的名称m_real_file大小
    static const int FILENAME_LEN = 200;
    //设置读缓冲区m_read_buf大小
    static const int READ_BUFFER_SIZE = 2048;
    //设置写缓冲区m_write_buf大小
    static const int WRITE_BUFFER_SIZE = 1024;
    //报文的请求方法，本项目只用到GET和POST
    enum METHOD//枚举常量
    {
        GET = 0,//向服务器获取资源，常见的查询请求
        POST,//向服务器提交数据的请求
        HEAD,//与get类似，返回的相应没有具体内容，用于获取报头
        PUT,//一般用于更新请求
        DELETE,//删除资源
        TRACE,//回显服务器收到的请求，主要用于测试
        OPTIONS,//获取服务器支持的http请求方法，服务器性能，跨域检查等
        CONNECT,//让服务器代替用户去访问其它网页，之后把数据返回给用户，一般用于http代理
        PATCH//对put方法的补充，对指定资源进行局部更新
    };
    //主状态机的状态
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,//正在分析请求行
        CHECK_STATE_HEADER,//正在分析头部字段
        CHECK_STATE_CONTENT//分析数据主体
    };
    //报文解析的结果
    enum HTTP_CODE
    {
        NO_REQUEST,// 请求不完整，需要继续获取客户数据
        GET_REQUEST,// 获得了一个完整的客户请求
        BAD_REQUEST,// 客户请求有语法错误
        NO_RESOURCE,// 客户对资源没有足够的访问权限
        FORBIDDEN_REQUEST,// 服务器内部错误
        FILE_REQUEST,//可以正常访问文件，跳转process_write()
        INTERNAL_ERROR,//服务器内部错误
        CLOSED_CONNECTION// 客户端已经关闭连接
    };
    //从状态机的状态
    enum LINE_STATUS
    {
        LINE_OK = 0,//读取到完整行
        LINE_BAD,//行出错
        LINE_OPEN//行不完整
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    //初始化套接字地址，函数内部会调用私有方法init
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    //关闭http连接
    void close_conn(bool real_close = true);
    //处理程序
    void process();
    //读取浏览器端发来的全部数据
    bool read_once();
    //响应报文函数
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    //CGI使用线程池初始化数据库表
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;


private:
    void init();
    //从m_read_buf读取，并处理请求报文
    HTTP_CODE process_read();
    //向m_write_buf写入响应报文数据
    bool process_write(HTTP_CODE ret);
    //主状态机解析报文中的请求行数据
    HTTP_CODE parse_request_line(char *text);
    //主状态机解析报文中的请求头数据
    HTTP_CODE parse_headers(char *text);
    //主状态机解析报文中的请求内容
    HTTP_CODE parse_content(char *text);
    //生成响应报文
    HTTP_CODE do_request();

    //m_start_line是已经解析的字符
    //get_line用于将指针向后偏移，指向未处理的字符
    char *get_line() { return m_read_buf + m_start_line; };
    //从状态机 读取一行，分析是请求报文的哪一部分
    LINE_STATUS parse_line();

    void unmap();
    //根据响应报文格式，生成对应8个部分，以下函数均由do_request调用
    bool add_response(const char *format, ...);//可变参数
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state;  //读为0, 写为1

private:
    int m_sockfd;
    sockaddr_in m_address;
    //存储读取的请求报文数据
    char m_read_buf[READ_BUFFER_SIZE];
    //缓冲区中m_read_buf中数据的最后一个字节的下一个位置
    int m_read_idx;
    //m_read_buf读取的位置m_checked_idx
    int m_checked_idx;
    //m_read_buf中已经解析的字符个数
    int m_start_line;

    //存储发出的响应报文数据
    char m_write_buf[WRITE_BUFFER_SIZE];
    //指示buffer中的长度
    int m_write_idx;

    //主状态机的状态
    CHECK_STATE m_check_state;
    //请求方法
    METHOD m_method;

    //以下为解析请求报文中对应的6个变量
    //存储读取文件的名称
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;


    char *m_file_address;//读取服务器上的文件地址
    struct stat m_file_stat;
    struct iovec m_iv[2];//io向量机制iovec
    int m_iv_count;
    int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
    int bytes_to_send;//剩余发送字节数
    int bytes_have_send;//已发送字节数
    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif //TINY_WEBSERVER_HTTP_CONN_H
