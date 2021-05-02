
#include "busi.h"
#include "loguru.hpp"
#include <yaml-cpp/yaml.h>

void BusiProc::operator()() { 
    LOG_F(INFO, "begin busi thread...");
    _Pkt *p = NULL;
    // this->busi_func(p);
    while(true) {
        p = q_rd->remove();
        LOG_F(INFO, "receive a busi packet! 0x%x", p->header.type);
        this->busi_func(p);
        delete p; // 在这里释放报文
    }
}

void BusiProc::busi_func(_Pkt *p){
    LOG_F(INFO, "---- busi_func call from BusiProc !!!");
}

// 解析命令文本yaml串， 生成报文
static void parseCmdStr(string &cmd, _Pkt *p, u_short *cmd_id) {
    YAML::Node c = YAML::Load(cmd.c_str());
    *cmd_id = c["cmd_id"].as<u_short>();
}



///////////  ACC 的业务处理逻辑
void ACCBusi::busi_func(_Pkt *p){
    LOG_F(INFO, "---- busi_func call from ACCBusi 0x%x", p==NULL ? 0 : p->header.type);
    u_short ptype = p->header.type;
    if(ptype == 0x1999 ){
        P1999 *b = (P1999 *)p->body;
        LOG_F(INFO, "cmd str: %s", b->yaml.c_str());
    }
}

///////////  LCC 的业务处理逻辑
void LCCBusi::busi_func(_Pkt *p){
    LOG_F(INFO, "---- busi_func call from LCCBusi 0x%x", p==NULL?0:p->header.type);
}


///////////  SC 的业务处理逻辑
void SCBusi::busi_func(_Pkt *p){
    LOG_F(INFO, "---- busi_func call from SCBusi 0x%x", p==NULL?0:p->header.type);
}

///////////  SLE 的业务处理逻辑
void SLEBusi::busi_func(_Pkt *p){
    LOG_F(INFO, "---- busi_func call from SLEBusi 0x%x", p==NULL?0:p->header.type);
}


