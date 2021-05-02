
#ifndef _MAIN_H
#define _MAIN_H

#include "comm.h"
#include "util.h"

#include "pkt_def.h"
#include "yaml-cpp/yaml.h"
#include "busi.h"

#include "loguru.hpp"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"

#include <string>
#include <map>
#include <time.h>
#include <thread>

using namespace std;


// 客户端连接
class ConnHandler
{
    public:
        ConnHandler() {
            msg = new ACE_Message_Block (MAX_PKT_SIZE);
            time(&active_time);
            pthread_mutex_init(&wr_mutex, NULL);
        };
        ConnHandler(int sock): ConnHandler() { _sock = sock; };
        virtual ~ConnHandler();
        int handler() { return _sock; }
        void setAddr(const char *addr) {
            remote_addr = addr;
        }
        string &getAddr() { return remote_addr; }
        int readData(_Pkt **);
        int writeData(_Pkt *p);
        void setLocalId(const char *id) { strcpy(local_id, id); };
        const char *getId() { return remote_id; };

        virtual int check(time_t *t);
        virtual _Pkt *process_pkt();

    protected:
        int _sock = -1;
        char remote_id[9] = {0};  // 此连接的客户端标识id,  通过对端发送的链路测试报文判断
        char local_id[9] = {0};   // 本服务器的id
        time_t active_time; // 活动时间
        u_int pkt_seq_seed = 0;  // 报文发送种子 
        ACE_Message_Block *msg = NULL; ///接收报文缓存
        pthread_mutex_t wr_mutex; // 写锁

    private:
        string remote_addr;
};

// 向上级服务的连接
class ParentConnHandler: public ConnHandler {
    public:
        ParentConnHandler(string &parent_id, Redis *c);
        ~ParentConnHandler();

        virtual int check(time_t *t); // 超时检查， 以便发送链路检测包
        virtual _Pkt *process_pkt();
        int connect(); // 建立连接
        bool connected() { return _sock >= 0; }; // 是否已经连接
        void disconnect() { tcp_close(_sock); _sock = -1; msg->reset(); }
        void setTimeout(int t) {timeout_sec = t; }; // 修改默认超时

    private:
        Redis *redis;
        int test_count = 0;  // 发送链路探测的次数， 超过3次未收到任何报文， 就要断开重连
        time_t last_test; // 上次发送链路探测的时间
        time_t timeout_sec = 60; // 单次的超时间隔， 活动时间超过这个
        int sendLinkTest();
};


class Loop_Server: public BusiCallBack
{
public:
  // Template Method that runs logging server's event loop.
    virtual int run ();
    Loop_Server(const char *_conf);

    // The following four methods are ``hooks'' that can be
    // overridden by subclasses.
    virtual int open (u_short port = 0);
    virtual int handle_connections (int sock, const char *client_addr);
    virtual int handle_data (int sock);
    int handle_check();  // 检查各连接的情况，看是否有闲置超时的情况

    // Close the socket endpoint.
    ~Loop_Server ();

    virtual int sendUp(_Pkt *p); //向上级发送消息
    virtual int sendDown(_Pkt *p, char *id_dst=NULL); //向下级节点发送消息， 如果id_dst 为NULL, 则是向所有在线的下级发送
    virtual const char *getId(); 

private:
    bool reg();

private:
    pthread_mutex_t wr_mutex; // 保护数据结构， 主要是对 conns 的遍历或修改
    int listen_sock = -1;
    map<int, ConnHandler *> conns;
    ParentConnHandler *parent_conn = NULL; // 到上级服务器的连接
    YAML::Node conf;
    char id[9] = {0};   // 本服务器id
    int port;    // 监听端口
    int dev_type; // 本服务器类型  
    Redis *redis; // 连接的 redis 服务
    wqueue<_Pkt *> *m_queue;
};

#endif /* _MAIN_H */
