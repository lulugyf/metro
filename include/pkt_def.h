
#ifndef __PKT_IMPL_H
 #define __PKT_IMPL_H
#include "pkt.h"

    

class P2001 : public _BodyBase{
public:
    char id_dst[8+1]; // 目的设备编码
    virtual uint16_t type() { return 0x2001; }
    virtual uint32_t len() { return 4; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P2001A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x2001; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3001 : public _BodyBase{
public:
    char timestamp[14+1]; // 起时间 时间戳
    char id_src[8+1]; // 发起方节点 ID
    char id_dst[8+1]; //  目的节点 ID,  发生降级运营模式的车站
    char oper_id[10+1]; //  操作员 ID
    u_char mode; // 降级运营模式 ID
    virtual uint16_t type() { return 0x3001; }
    virtual uint32_t len() { return 26; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3001A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x3001; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3002 : public _BodyBase{
public:
    char now[14+1]; //  当前时间 时间戳
    char id_dst[8+1]; // 模式节点 ID, 发生降级运营模式的车站或者终端设备 ID
    u_char mode; // 降级运营模式 ID
    char affect_time[14+1]; // 降级运营模式发生时间
    virtual uint16_t type() { return 0x3002; }
    virtual uint32_t len() { return 19; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3002A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x3002; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3003A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x3003; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3004 : public _BodyBase{
public:
    char id_dst[8+1]; // 被查询的线路/车站/设备的节点编号
    virtual uint16_t type() { return 0x3004; }
    virtual uint32_t len() { return 4; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3004A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x3004; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};

class P3005A : public _BodyBase{
public:

    virtual uint16_t type() { return 0x3005; }
    virtual uint32_t len() { return 0; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};
#endif // __PKT_IMPL_H
