
#include "util.h"
#include "comm.h"

#include "loguru.hpp"

#include <iostream>
using namespace std;

int splitc(char *string, char *fields[], int  nfields, char sep )
{
    char *p, *p1;
    int i;

    p = string;
    fields[0] = p;
    i = 1;
    while( (p1=strchr(p,sep))!= NULL && i<nfields){
        p1[0] = 0; p1++;
        fields[i] = p1;
        p = p1; i++;
    }
    return i;
}

Redis::Redis(const char *addr, const char *pswd){
    char s[64];
    char *f[3];
    strcpy(s, addr);
    splitc(s, f, 3, ':');

    memset(local_addr, 0, sizeof local_addr);
    c = redisConnect(f[0], atoi(f[1]));
    if(c != NULL && c->err == 0)
        LOG_F(INFO, "redis [%s] connected", addr);
    else{
        LOG_F(ERROR, "connect redis [%s] failed: %d: %s", addr, c->err, c->errstr);
        redisFree(c);
        c = NULL;
        return;
    }

    tcp_get_sockaddr(c->fd, local_addr);
    LOG_F(INFO, "local addr: %s", local_addr);

    // test
    // redisReply *reply;
    // reply = (redisReply *)redisCommand(c, "SET foo %s", "bar");
    // cout << "set foo: " << reply->str << endl;
    // freeReplyObject(reply);
    // reply = (redisReply *)redisCommand(c, "GET foo");
    // cout << "get foo: ---- " << reply->str << endl;
    // freeReplyObject(reply);
}


Redis::~Redis(){
    if(c != NULL)
        redisFree(c);
}

int Redis::setKey(const char *key, const char *val){
    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "SET %s %s", key, val);
    if(reply == NULL) {
        LOG_F(ERROR, "fail: %s", c->errstr);
        return -1;
    }

    //cout << "set foo: " << reply->str << endl;
    // LOG_F(INFO, "set key return %d: %s", c->err, c->errstr);
    if(c->err == 0){
        freeReplyObject(reply);
        return 0;
    }
    freeReplyObject(reply);

    return -1;
}
string Redis::getKey(const char *key){
    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "GET %s", key);
    //cout << "get foo: ---- " << reply->str << endl;
    freeReplyObject(reply);
    if(reply->str[0] == 0) {
        return "";
    }else{
        return string(reply->str);
    }
}
bool Redis::isOk() {
    if(c == NULL)
        return false;
    return true;
}

