
// 报文格式定义



#ifndef _PKT_H_
 #define _PKT_H_

#include <string>
#include "ace/CDR_Stream.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/SOCK_Stream.h"

using namespace std;

#define MAX_PKT_SIZE 512

#define __BYTE_ORDER__ 1

void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);

// 5.1.7.1.1 包头
class _Header {
    public: 
        u_short type; //消息类型码 参见线网规程交易类型定义 WORD 2
        // 对于BCD 码， 存在结构体中的是ASCII 码的串， 比如： "1234", 从网络读写时再作BCD转换
        char id_src[8+1]; // 发起方标识码 BCD 4
        char id_dst[8+1]; // 接收方标识码 BCD 4
        char id_trn[8+1] = {0}; // 转发方标识码 BCD 4
        u_int   pkt_seq; //  会话流水号 long 4
        u_char  ack_flag = 0x00; // 请求应答标志 0x00：请求消息；0x01：应答消息 BYTE 1
        u_char  ver = 0x01;      // 消息报文版本 缺省为 0x01 BTYE 1
        u_char  ack = 0;      // 应答码 BTYE 1
    public:
        void setid(string &id, int _type); // 设置BCD编码, _type: 1-id_src 2-id_dst 3-id_trn
        void getid(string &id, int _type); // 获取BCD编码
        int  len() {return 21; };
        void genAck(_Header & h, u_char code) {
            h.type = type;
            strcpy(h.id_dst, id_src);
            strcpy(h.id_src, id_dst);
            strcpy(h.id_trn, id_trn);
            h.ack_flag = 0x01;
            h.ack = code;
        };
        int encode(ACE_OutputCDR &cdr);
        int decode(ACE_InputCDR &cdr);

};

// ACE_Export int operator>> (ACE_InputCDR &cdr, _Header &header);
// ACE_Export int operator<< (ACE_OutputCDR &cdr, const _Header &header);

// 消息包体的基类
class _BodyBase {
    public:
        virtual int encode(ACE_OutputCDR &cdr) {return 0;};
        virtual int decode(ACE_InputCDR  &cdr) {return 0;};
        virtual uint16_t type(){return 0;};
        virtual uint32_t len(){return 0;};
        virtual ~ _BodyBase() {};
};

class _Pkt {
    public:
        uint16_t pkt_len; //包长度，  本数据包的长度（不包括长度本身） Word 2
        _Header header;  // 包头
        _BodyBase *body;  // 包体
        uint8_t checksum[16]; // 校验码 验证内容包括包头和包体，算法为 MD5 Block 16

        void setBody(_BodyBase *_body) {
            this->body = _body;
            header.type = body->type();
        }
        int encode(ACE_OutputCDR &cdr); // 报文编码
        int decode(ACE_InputCDR  &cdr); // 报文解码

        int encodeAck(ACE_OutputCDR &cdr,  uint8_t ack); // 编码应答报文

        ~ _Pkt() {
            if(body != NULL ) {
                delete body;
            }
        };
};


ACE_CDR::Char * _str2bcd4(ACE_CDR::Char *dst, const char *src, int _l=4);
void _bcd2str4(char *dst, const ACE_CDR::Char *src, int _l=4);

/*
// 设备控制命令
class P3001 : public _BodyBase{
    public:
        // 报文字段
        char timestamp[14+1]; // 发起时间 7 Timestamp YYYYMMDDHH24MISS BCD
        char id_src[8+1];  //  发起方节点 ID 4 BCD
        char id_dst[8+1];  // 目的节点 ID 4 BCD
        char oper_id[10+1];   //   操作员 ID 10 Char
        u_char  mode;        //   降级运营模式 ID 1 Byte 参见: 4.6 降级运营模式 ID

        virtual uint16_t type() { return 0x3001; }
        virtual uint32_t len() { return 7+4+4+10+1; }
        virtual int encode(ACE_OutputCDR &cdr) ;
        virtual int decode(ACE_InputCDR  &cdr) ;

        void setTime(long long sec=0);
};
class P3001A : public _BodyBase{}; // 应答报文

// 链路测试报文
class P2001 : public _BodyBase{
    public:
        // 报文字段
        char id_dst[8+1]; // 目的方节点id

        // 包体为0长度
        virtual uint16_t type() { return 0x2001; }
        virtual uint32_t len() { return 4; }
        virtual int encode(ACE_OutputCDR &cdr) ;
        virtual int decode(ACE_InputCDR  &cdr) ;
};
class P2001A : public _BodyBase{};
*/

#endif

