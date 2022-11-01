> 使用C++编写的Liunx系统下的支持多并发的网络服务器，包含Mysql后端和使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现)** 的并发模型，能够使用**状态机**解析HTTP请求报文，并且实现了**同步/异步日志系统**，记录服务器运行状态。

# 快速开始

- 安装MySQL以及相关库

  ```bash
  dpkg -l | grep mysql//本体
  sudo apt install mysql-server//服务
  sudo apt-get install libmysqlclient-dev//链接库
  ```

- 测试前确认已安装MySQL数据库

  ```mysql
  // 建立yourdb库
  create database yourdb;
  
  // 创建user表
  USE yourdb;
  CREATE TABLE user(
      username char(50) NULL,
      passwd char(50) NULL
  )ENGINE=InnoDB;
  
  // 添加数据
  INSERT INTO user(username, passwd) VALUES('name', 'passwd');
  ```

- 修改main.cpp中的数据库初始化信息

  ```mysql
  //数据库登录名,密码,库名
  string user = "root";
  string passwd = "root";
  string databasename = "yourdb";
  ```

- build

  ```
  sh ./build.sh
  ```

- 启动server

  ```
  ./server
  ```

- 浏览器端

  ```
  ip:9006
  ```

```
./server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model]
```

温馨提示:以上参数不是非必须，不用全部使用，根据个人情况搭配选用即可.

- -p，自定义端口号
  - 默认9006
- -l，选择日志写入方式，默认同步写入
  - 0，同步写入
  - 1，异步写入
- -m，listenfd和connfd的模式组合，默认使用LT + LT
  - 0，表示使用LT + LT
  - 1，表示使用LT + ET
  - 2，表示使用ET + LT
  - 3，表示使用ET + ET
- -o，优雅关闭连接，默认不使用
  - 0，不使用
  - 1，使用
- -s，数据库连接数量
  - 默认为8
- -t，线程数量
  - 默认为8
- -c，关闭日志，默认打开
  - 0，打开日志
  - 1，关闭日志
- -a，选择反应堆模型，默认Proactor
  - 0，Proactor模型
  - 1，Reactor模型

测试示例命令与含义

```
./server -p 9007 -l 1 -m 0 -o 1 -s 10 -t 10 -c 1 -a 1
```

# 成果展示

# 结构分析

## lock.h 线程同步机制封装类（锁）

> 本项目一共封装了三种类型包括信号量(sem)、互斥锁(量)(locker)以及条件变量(cond),该部分详情可以参考我博客中的另一篇文章[操作系统笔记](https://yishuihean.cn/post/操作系统笔记)多线程冲突中的内容。

### **信号量**

信号量是一种特殊的变量，它只能取自然数值并且只支持两种操作：等待(P)和信号(V).假设有信号量SV，对其的P、V操作如下：

> - P，如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
> - V，如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一

信号量的取值可以是任何自然数，最常用的，最简单的信号量是二进制信号量，只有0和1两个值.

> - sem_init函数用于初始化一个未命名的信号量
> - sem_destory函数用于销毁信号量
> - sem_wait函数将以原子操作方式将信号量减一,信号量为0时,sem_wait阻塞
> - sem_post函数以原子操作方式将信号量加一,信号量大于0时,唤醒调用sem_post的线程

以上，成功返回0，失败返回errno

### **互斥量**

互斥锁,也成互斥量,可以保护关键代码段,以确保独占式访问.当进入关键代码段,获得互斥锁将其加锁;离开关键代码段,唤醒等待该互斥锁的线程.

> - pthread_mutex_init函数用于初始化互斥锁
> - pthread_mutex_destory函数用于销毁互斥锁
> - pthread_mutex_lock函数以原子操作方式给互斥锁加锁
> - pthread_mutex_unlock函数以原子操作方式给互斥锁解锁

以上，成功返回0，失败返回errno

### **条件变量**

条件变量提供了一种线程间的通知机制,当某个共享数据达到某个值时,唤醒等待这个共享数据的线程.

> - pthread_cond_init函数用于初始化条件变量
> - pthread_cond_destory函数销毁条件变量
> - pthread_cond_broadcast函数以广播的方式唤醒**所有**等待目标条件变量的线程
> - pthread_cond_wait函数用于等待目标条件变量.该函数调用时需要传入 **mutex参数(加锁的互斥锁)** ,函数执行时,先把调用线程放入条件变量的请求队列,然后将互斥锁mutex解锁,当函数成功返回为0时,互斥锁会再次被锁上. **也就是说函数内部会有一次解锁和加锁操作**.

# 部分函数解读

## pthread_detach

***\*作用：\****从状态上实现线程分离，注意不是指该线程独自占用地址空间。

**线程分离状态：**指定该状态，线程主动与主控线程断开关系。线程结束后（不会产生僵尸线程），其退出状态不由其他线程获取，而直接自己自动释放（自己清理掉PCB的残留资源）。

能对一个已经处于detach状态的线程调用pthread_join，这样的调用将返回EINVAL错误（22号错误）。也就是说，如果已经对一个线程调用了pthread_detach就不能再调用pthread_join了。

# TODO List

- 网页装修
- 支持更多操作
- ~~公网部署~~



