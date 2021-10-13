# CHAT

### 项目描述
一个基于muduo网络库的聊天服务器+聊天客户端项目，服务器端使用了redis缓存集群和Nginx/Tcp集群方案。其中，nginx负责处理客户端的请求，通过nginx的负载均衡策略，nginx会筛选出压力最小的服务器，用于处理客户端的请求信息。信息到达服务器后，再进入作为缓存队列的redis中，然后通过发布-订阅模式，转发到客户端正在进行通信的另一方客户端。客户端用户注册的账号信息会保存到MySQL数据库中。

### 项目依赖
平台：Linux ubuntu98 5.11.0-37-generic
muduo网络库
数据库：mysql  Ver 8.0.26-0ubuntu0.20.04.2 for Linux on x86_64 ((Ubuntu))
缓存数据库：Redis server v=5.0.7 sha=00000000:0 malloc=jemalloc-5.2.1 bits=64 build=636cde3b5c7a3923
反向代理服务器：nginx version: nginx/1.20.1

muduo网络库教程：https://www.cnblogs.com/conefirst/articles/15224039.html

mysql环境搭建教程：https://www.cnblogs.com/conefirst/articles/15225844.html

redis环境搭建教程：https://www.cnblogs.com/conefirst/articles/15214297.html

nginx环境搭建教程：https://www.cnblogs.com/conefirst/articles/15322964.html


### 项目构建方法
方法一：
clone本项目到Linux系统后，进入CHAT目录，直接执行```./configure```。
然后将会生成bin目录，里面对应两个可执行文件，分别是ChatServer和ChatClient，对应了该项目的服务器端和客户端。

方法二：
通过vscode打开CHAT项目，然后在CMake工作环境中按顺序点击Configure All Project和Build All Project。




### 打个草稿
```
enum EnMsgType
{
    LOGIN_MSG = 1,  // 登录消息
    LOGIN_MSG_ACK,  // 登录响应消息
    LOGINOUT_MSG,   // 注销消息
    REG_MSG,        // 注册消息
    REG_MSG_ACK,    // 注册响应消息
    ONE_CHAT_MSG=6,   // 聊天消息
    ADD_FRIEND_MSG, // 添加好友消息

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG,    // 加入群组
    GROUP_CHAT_MSG,   // 群聊天
};
```

注册：
{"msgid":4,"name":"zhao wu","password":"123456"}
添加好友：
{"msgid":7,"id":1,"friendid":2}

登录：
{"msgid":1,"id":1,"password":"123456"}
{"msgid":1,"id":2,"password":"123456"}
发送信息：
{"msgid":6,"id":2,"from":"li si","toid":1,"msg":"hello111"}

msgid
id:1
from:"zhang san"
to:3
msg:"xxxx"

```
CREATE TABLE `user`(
    `name_id` int NOT NULL,
    `role_id` int NOT NULL,
    PRIMARY KEY(`name_id`,`role_id`)
)ENGINE=InnoDB;
```

```
CREATE TABLE `User` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '用户id',
  `name` varchar(50) NOT NULL COMMENT '用户名',
  `password` varchar(50) NOT NULL COMMENT '用户密码',
  `state` enum('online','offline') DEFAULT 'offline' COMMENT '当前登录状态',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci 
```

```
CREATE TABLE `OfflineMessage` (
  `userid` int NOT NULL COMMENT '用户id',
  `message` varchar(500) NOT NULL COMMENT '离线消息（存储Json字符串）'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci   
```

```
CREATE TABLE `FriendId` (
  `userid` int NOT NULL COMMENT '用户id',
  `friend` int NOT NULL COMMENT '好友id'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci 
```

```
CREATE TABLE `AllGroup` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '组id',
  `groupname` varchar(50) NOT NULL COMMENT '组名称',
  `groupdesc` varchar(200) DEFAULT '' COMMENT '组功能描述',
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci 
```

```
CREATE TABLE `GroupUser` (
  `groupid` int NOT NULL COMMENT '组id',
  `userid` int NOT NULL COMMENT '组员id',
  `grouprole` enum('creator','normal') DEFAULT 'normal' COMMENT '组内角色'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci 
```

```
stream{
    upstream MyServer{
        hash $remote_addr consistent;
        server 192.168.10.105:6666 weight=1 max_fails=3 fail_timeout=30s;
        server 192.168.10.111:6666 weight=1 max_fails=3 fail_timeout=30s;
    }

    server{
        proxy_connect_timeout 1s;
        proxy_timeout 3s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```
