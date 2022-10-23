//
// Created by Administrator on 2022/10/23.
//

#include <E:\MySQL\MySQL Server 8.0\include\mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_pool *connection_pool::GetInstance() {
    static connection_pool coonPool;
    return &coonPool;
}