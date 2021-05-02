

// 此文件用于定义 交互逻辑的处理代码申明
#include <iostream>
#include <thread>

#include "util.h"
#include "pkt.h"

using namespace std;

#define DEVTP_ACC 0x01  // ACC 服务器
#define DEVTP_LCC 0x02  // LCC 服务器
#define DEVTP_SC  0x03  // SC 服务器
#define DEVTP_SLE 0x99  // SLE 设备， 这个是自定义的。


// 多继承 https://www.cprogramming.com/tutorial/multiple_inheritance.html
// 用于反向调用
class BusiCallBack {
    public:
        // 这些方法由 Loop_Server 来实现， 并将句柄传递给 Busi 对象
        virtual int sendUp(_Pkt *p) = 0; //向上级发送消息
        virtual int sendDown(_Pkt *p, char *id_dst=NULL) = 0; //向下级节点发送消息， 如果id_dst 为NULL, 则是向所有在线的下级发送
        virtual const char *getId() = 0; // 获取当前服务器的id
        //
};

class BusiProc
{
public:
    BusiProc() {};
    BusiProc(wqueue<_Pkt *> *qr) : BusiProc() {
        q_rd = qr;
    };
    virtual void operator()() ;
    virtual void busi_func(_Pkt *p) ;
protected:
    wqueue<_Pkt *> *q_rd;
    BusiCallBack *callback = NULL;
};



class ACCBusi: public BusiProc {
public:
    ACCBusi(wqueue<_Pkt *> *qr, BusiCallBack *c) { q_rd = qr; callback=c;};
    virtual void busi_func(_Pkt *p) ;
};

class LCCBusi: public BusiProc {
public:
    LCCBusi(wqueue<_Pkt *> *qr, BusiCallBack *c) { q_rd = qr; callback=c;};
    virtual void busi_func(_Pkt *p) ;
};


class SCBusi: public BusiProc {
public:
    SCBusi(wqueue<_Pkt *> *qr, BusiCallBack *c) { q_rd = qr; callback=c;};
    virtual void busi_func(_Pkt *p) ;
};

class SLEBusi: public BusiProc {
public:
    SLEBusi(wqueue<_Pkt *> *qr, BusiCallBack *c) { q_rd = qr; callback=c;};
    virtual void busi_func(_Pkt *p) ;
};


