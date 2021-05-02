// 这个程序是向 服务器发送指令的 独立程序

/*
参数： <redis_addr> <target_id> <pkt_yaml_file>

处理过程：
  连接redis
  从redis查询到 <target_id> 服务器的地址
  连接服务器端口
  读取 <pkt_yaml_file> 中的文件内容， 拼装报文 P1999
  向服务器发送该报文
  等待应答
  关闭连接
  关闭 redis连接
  退出
*/

#include "pkt_def.h"
#include "comm.h"
#include "util.h"
#include "loguru.hpp"

#include <iostream>
#include <string>
#include <vector>
using namespace std;

static void wf(const char *file_name, const char *buf, size_t len)
{
    FILE *fp = fopen(file_name, "w");
    if(fp == NULL)
        return;
    
    fwrite(buf, 1, len, fp);
    fclose(fp);
}

static char *rf(const char *fname)
{
    struct stat st;
    char *buf;
    if(stat(fname, &st) != 0)
        return NULL;
    buf = (char *)malloc(st.st_size+1);
    buf[st.st_size] = 0;
    cout << "file size:" << st.st_size << endl;
    FILE *fp = fopen(fname, "r");
    fread(buf, 1, st.st_size, fp);
    fclose(fp);
    return buf;
}


int test_send_pkt() {
    P3001 *p1 = new P3001();
    memcpy(p1->id_src, "12345678", 8);
    memcpy(p1->id_dst, "87654321", 8);
    strcpy(p1->oper_id, "myoperid");
    //p1->setTime();

    _Pkt p;
    p.setBody(p1);
    memcpy(p.header.id_dst, "23456789", 8);
    memcpy(p.header.id_src, "98765432", 8); 

    ACE_OutputCDR payload (MAX_PKT_SIZE);
    p.encode(payload);

    int len = payload.total_length();

    const char *s = (const char *)payload.begin()->rd_ptr();

    int sd = tcp_connect("127.0.0.1", 9991);
    tcp_writen(sd, s, len);
    tcp_close(sd);
}

static int sendPkt(int sock, _Pkt *p) {
    ACE_OutputCDR payload (MAX_PKT_SIZE);
    p->encode(payload);
    return tcp_writen(sock, payload.begin()->rd_ptr(), payload.total_length());
}
static int recvPkt(int sock, _Pkt *p) {
    u_short pkt_len;
    ACE_Message_Block msg(MAX_PKT_SIZE);
    int r = tcp_read(sock, msg.wr_ptr(), 2);
    // cout << " read 2 bytes, return " << r << endl;
    if(r != 2){
        cout << "read failed!!!" << endl;
        return -1;
    }
    msg.wr_ptr(2);
    memcpy(&pkt_len, msg.rd_ptr(), 2);
    // cout << " --- pkt_len: " << pkt_len << endl;

    r = tcp_read(sock, msg.wr_ptr(), pkt_len + 2 - msg.total_length());
    // cout << "--- read msg return " << r << endl;
    msg.wr_ptr(r);
    ACE_InputCDR cdr(&msg);
    // delete p->body;
    if(p->decode(cdr) != 0){
        cout << "read Reply failed!" << endl;
        return -1;
    }else{
        //cout << "reply pkt: " << p->header.type << " " << p->header.ack << endl;
        LOG_F(INFO, "reply pkt: 0x%x isAck: %d  ack-code: %d", p->header.type, p->header.ack_flag, p->header.ack);
    }
    return 0;
}


// ./sendcmd 127.0.0.1:6379 99000101 ../test/cmd1.yaml
int main(int argc, char *argv[]){
    if(argc < 4){
        cout << "Usage: " << argv[0] << " <redis_addr> <target_id> <pkt_yaml_file>" << endl;
        return 2;
    }
    Redis redis(argv[1], "");
    if(!redis.isOk()){
        return 1;
    }
    char key[64];
    sprintf(key, "id_%s", argv[2]);

    string addr = redis.getKey(key);
    cout << key << " = server address: " << addr << endl;

    vector<string> f;
    SplitString(addr, f, ":");
    int _sock = tcp_connect(f.at(0).c_str(), atoi(f.at(1).c_str()));

    char *content = rf(argv[3]);
    P1999 body;
    body.yaml = content;
    cout << "body length:" << body.yaml.length() << endl;
    free(content);
    _Pkt p;
    strcpy(p.header.id_dst, argv[2]);
    strcpy(p.header.id_src, "00000000");
    p.setBody(&body);

    sendPkt(_sock, &p);

    recvPkt(_sock, &p);

    tcp_close(_sock);

    cout << "Done!" << endl;


    return 0;
}
