//
// Created by Administrator on 2022/10/28.
//

#ifndef LST_TIMER_H
#define LST_TIMER_H

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

#include <time.h>
#include "../log/log.h"
//连接资源结构体成员需要用到定时器类
//需要前向声明
class util_timer;

//连接资源
struct client_data
{   //客户端socket地址
    sockaddr_in address;
    //socket文件描述符
    int sockfd;
    //定时器
    util_timer *timer;
};

//定时器类
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;//超时时间

    void (* cb_func)(client_data *);//回调函数
    client_data *user_data;//连接资源
    util_timer *prev;//前向定时器
    util_timer *next;//后继定时器
};

class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();//常规销毁链表

    void add_timer(util_timer *timer);//添加定时器，内部调用私有成员add_timer
    void adjust_timer(util_timer *timer);//调整定时器，任务发生变化时，调整定时器在链表中的位置
    void del_timer(util_timer *timer);
    void tick();

private:
    //私有成员，被公有成员add_timer和adjust_time调用
    void add_timer(util_timer *timer, util_timer *lst_head);//主要用于调整链表内部结点
    //头尾结点
    util_timer *head;
    util_timer *tail;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif //LST_TIMER_H
