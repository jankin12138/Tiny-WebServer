//
// Created by Administrator on 2022/10/23.
//

#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H
#include <stdio.h>
#include <list>
#include <E:\MySQL\MySQL Server 8.0\include\mysql.h>//Linux下切换目录为<mysql/mysql.h>
#include <errors.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool
{
public:
    MYSQL *GetConnection();				 //获取数据库连接
    bool ReleaseConnection(MYSQL *conn); //释放连接
    int GetFreeConn();					 //获取连接
    void DestroyPool();					 //销毁所有连接

    //单例模式,通过唯一的实例化来保证一个类面向系统只有一个实例，这里采用的是饿汉式实现方式
    static  connection_pool *GetInstance();


};
#endif //TINYWEBSERVER_SQL_CONNECTION_POOL_H
