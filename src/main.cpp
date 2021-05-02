#include "main.h"

#include <memory>

#define MAX_CLIENT 50


static int sendPkt(int sock, _Pkt *p) {
    ACE_OutputCDR payload (MAX_PKT_SIZE);
    p->encode(payload);
    return tcp_writen(sock, payload.begin()->rd_ptr(), payload.total_length());
}

Loop_Server::Loop_Server(const char *_conf) {
    conf = YAML::LoadFile( _conf );

    strcpy(id, conf["id"].as<string>().c_str());
    port = conf["port"].as<int>();
    m_queue = new wqueue<_Pkt *>();
    string dtype = conf["type"].as<string>();
    if(dtype == "ACC")
        dev_type = DEVTP_ACC;
    else if(dtype == "LCC")
        dev_type = DEVTP_LCC;
    else if(dtype == "SC")
        dev_type = DEVTP_SC;
     else if(dtype == "SLE")
        dev_type = DEVTP_SLE;
    else
        LOG_F(ERROR, "invalid device type: %s", dtype.c_str());
    pthread_mutex_init(&wr_mutex, NULL);
    redis = new Redis(conf["redis_addr"].as<string>().c_str(), "");
}

Loop_Server:: ~Loop_Server () { 
    if(listen_sock != -1){
        tcp_close(listen_sock);
        listen_sock = -1;
    }

    // delete all handlers
    map<int, ConnHandler *>::iterator it;
    for (it = conns.begin(); it != conns.end(); it++) {
        //out<< it->first << " " << it->second << endl; //Printing the values after inserting
        delete it->second;
    }
    delete m_queue;

    if(redis != NULL)
        delete redis;

    if(parent_conn != NULL)
        delete parent_conn;
};

int Loop_Server::open(u_short port) {
    listen_sock = tcp_listen(NULL, port);
    return listen_sock != -1;
}

// 新的客户端连接
int Loop_Server::handle_connections (int sock, const char *client_addr) {
    ConnHandler *c = new ConnHandler(sock);
    c -> setAddr(client_addr);
    c -> setLocalId(id);
    conns.insert(pair<int, ConnHandler *>(sock, c) );
    return 0;
}

int Loop_Server::handle_data (int sock){
    _Pkt *pp;
    if(parent_conn != NULL && parent_conn->handler() == sock){
        if(parent_conn->readData(&pp) == -1){
            LOG_F(ERROR, "parent conn read data failed, disconnect it.");
            parent_conn->disconnect();
        }else{
            if(pp != NULL)
                m_queue->add(pp); // 将报文交给业务逻辑处理
        }
        return 0;
    }

    map<int, ConnHandler *>::iterator it;

    it = conns.find(sock);
    if(it == conns.end()){
        LOG_F(INFO, "ERROR: can not find sock handler for %d", sock);
        return -1;
    }
    ConnHandler *c = it->second;
    if(c->readData(&pp) == -1) {
        LOG_F(INFO, "ERROR: client [%s] read failed of %s",
            c->getId(),
             c->getAddr().c_str());
        delete c;
        conns.erase(it);
    }else{
        if(pp != NULL){
            LOG_F(INFO, "add pkt to queue......");
            m_queue->add(pp); // 将报文交给业务逻辑处理
        }
    }
}

bool Loop_Server::reg() {
    if(! redis->isOk()) {
        LOG_F(ERROR, "redis not connected, exit!");
        return false;
    }
    char local_addr[64];
    strcpy(local_addr, redis->getLocalAddr());
    strchr(local_addr, ':')[0] = 0;
    sprintf(local_addr, "%s:%d", local_addr, port);
    char key[16];
    sprintf(key, "id_%s", id);
    LOG_F(INFO, "reg: [%s]=[%s]", key, local_addr);
    return redis->setKey(key, local_addr) == 0;
}

int Loop_Server::handle_check()  {
    time_t now;
    time(&now);

    // 检查客户端连接是否超时， 否则踢掉
    map<int, ConnHandler *>::iterator it;
    ConnHandler *c;
    for (it = conns.begin(); it != conns.end(); it++) {
        c = it->second;
        if(c->check(&now) == -1){
            delete c;
            conns.erase(it);
        }

    }

    //done 检查到parent的连接
    if(parent_conn != NULL)
        parent_conn->check(&now);
}

int ConnHandler::check(time_t *t) {
    return 0;
}

ConnHandler:: ~ ConnHandler() {
    if(_sock != -1){
        tcp_close(_sock);
        _sock = -1;
    }
    delete msg;
}

// 读取数据， 如果失败， 则返回 -1
/** 
 * 返回值含义：
 *    -1:  读取失败， 应该关闭
 *    0:   无需进一步处理
 * 函数返回时如果 *pp != NULL,  则报文返回给上层逻辑处理
 * */
int ConnHandler::readData(_Pkt **pp) {
    u_short pkt_len = 0;
    int r;
    *pp = NULL;

    // 2字节包长度
    if(msg->total_length() > 1){
        memcpy(&pkt_len, msg->rd_ptr(), 2);
        LOG_F(INFO, "pkt_lenth: %d", pkt_len);
    }else{
        r = tcp_read(_sock, msg->wr_ptr(), 2);
        if(r <= 0){
            LOG_F(ERROR, "read failed");
            return -1;
        }
        msg->wr_ptr(r);
        if(msg->total_length() < 2)
            return 0;
        memcpy(&pkt_len, msg->rd_ptr(), 2);
        LOG_F(INFO, "pkt_lenth  -: %d", pkt_len);
    }
    if(pkt_len > MAX_PKT_SIZE) {
        LOG_F(ERROR, "invalid pkt length: %d", pkt_len);
        return -1;
    }

    r = tcp_read(this->_sock, msg->wr_ptr(), pkt_len + 2 - msg->total_length());
    if(r == 0) {
        LOG_F(ERROR, "read failed, connection reset");
        return -1;
    }else if(r < 0) {
        LOG_F(ERROR, "read failed, bad return");
        return -1;
    }
    msg->wr_ptr(r);
    if(msg->total_length() >= pkt_len + 2) {
        *pp = process_pkt();
        msg->reset();
        time(&active_time);
    }
    return 0;
}

_Pkt * ConnHandler::process_pkt() {
    auto_ptr<_Pkt> p (new _Pkt() );

    ACE_InputCDR cdr(msg);
    if(p->decode(cdr) != 0){
        LOG_F(ERROR,  "invalid package");
        return NULL;
    }else{
        LOG_F(INFO, "receive pkt type: 0x%x", p->header.type);
    }
    if(p->header.type == 0x2001 ){
        if( p->header.ack_flag == 0x00 ) { // 请求
            // 链路测试报文， 直接应答
            if(remote_id[0] == 0) {
                // 收到第一次链路测试， 获取对端的id
                strcpy(remote_id, p->header.id_src);
            }
            ACE_OutputCDR payload (MAX_PKT_SIZE);
            p->encodeAck(payload, 0);
            int r = tcp_writen(_sock, payload.begin()->rd_ptr(), payload.total_length());
            LOG_F(INFO, "reply link test of %s, write %d bytes", remote_id, r);
            return NULL;
        }else{
            LOG_F(INFO, "received 2001 ack");
            return NULL;
        }
    }else if(p->header.type == 0x1999) { // 人工下发的指令， 应答后继续处理
        if( p->header.ack_flag == 0x00){ 
            ACE_OutputCDR payload (MAX_PKT_SIZE);
            p->encodeAck(payload, 0);
            int r = tcp_writen(_sock, payload.begin()->rd_ptr(), payload.total_length());
            if(r <= 0) {
                LOG_F(ERROR, "send reply failed: %d", r);
            }else
                LOG_F(INFO, "reply sendcmd");
            // 这里函数不返回， 报文要交给后续的业务处理。
        }
    }
    // done 接收报文的业务逻辑处理,  从下级设备发上来的报文
    return p.release();
    // return 0;
}

int ConnHandler::writeData(_Pkt *p) {
    ACE_OutputCDR payload (MAX_PKT_SIZE);
    if(p->header.ack_flag == 0x00)
        p->header.pkt_seq = ++ pkt_seq_seed; //请求报文更新其seq
    p->encode(payload);

    // 加入写锁， 避免写入报文错位
    pthread_mutex_lock(&wr_mutex);
    int r = tcp_writen(_sock, payload.begin()->rd_ptr(), payload.total_length());
    pthread_mutex_unlock(&wr_mutex);
    LOG_F(INFO, "write pkt of 0x%x ,  written %d bytes", p->header.type, r);
    return r;
}


ParentConnHandler::ParentConnHandler(string& parent_id, Redis *c) {
    redis = c;
    strcpy(remote_id, parent_id.c_str());
}

ParentConnHandler::~ParentConnHandler() {
    if(_sock >= 0)
        tcp_close(_sock);
}

// 检查链路超时情况
int ParentConnHandler::check(time_t *t){
    if(! connected() ){
        LOG_F(INFO, "retry connect to parent");
        connect();
        return 0;
    }

    if(*t - active_time < timeout_sec)
        return 0;

    if(test_count > 2 && *t - last_test > timeout_sec){
        // 满足断开连接的条件了, 断开
        LOG_F(WARNING, "disconnect parent connection");
        msg->reset(); 
        tcp_close(_sock);
        _sock = -1;
        return -1;
    }
    if(test_count == 0 || (*t - last_test >= timeout_sec )) {
        LOG_F(INFO, "send link test cause of idle %ds ...", timeout_sec);
        sendLinkTest();
        test_count += 1;
        time(&last_test);
    }
    return 0;
}

_Pkt * ParentConnHandler::process_pkt(){
    test_count = 0;
    auto_ptr<_Pkt> p (new _Pkt() );

    ACE_InputCDR cdr(msg);
    if(p->decode(cdr) != 0){
        LOG_F(ERROR,  "invalid package");
        return NULL;
    }else{
        LOG_F(INFO, "receive pkt type: 0x%x", p->header.type);
    }
    if(p->header.type == 0x2001 ){ // link test
        if( p->header.ack_flag == 0x00 ) { // 请求
            LOG_F(ERROR, "invalid link test from parent!");
        }else{
            LOG_F(INFO, "received 2001 ack");
        }
        return NULL;
    }
    // done 接收报文的业务逻辑处理,  从上级设备发来的报文
    return p.release();
}

// 连接到上级服务器并发送一个链路探测包
int ParentConnHandler::connect(){
    LOG_F(INFO, "begin connect to parent server: %s", remote_id);
    // 从redis 查询对端地址
    char key[16];
    sprintf(key, "id_%s", remote_id);
    string addr = redis->getKey(key);
    if(addr.find(':') < 0){
        LOG_F(ERROR, "find address of parent id[%s] failed, stop connect", remote_id);
        return -1;
    }
    LOG_F(INFO, "parent server %s address: %s", remote_id, addr.c_str());

    char s[64];
    char *f[3];
    strcpy(s, addr.c_str());
    splitc(s, f, 3, ':');
    _sock = tcp_connect(f[0], atoi(f[1]));
    if(_sock < 0) {
        LOG_F(ERROR, "connect to parent server failed");
        return -1;
    }
    if(sendLinkTest() < 0){
        LOG_F(ERROR, "init parent connection failed, disconnect");
        disconnect();
    }
    time(&active_time);
    return 0;
}

int ParentConnHandler::sendLinkTest() {
    P2001 *b = new P2001();
    strcpy(b->id_dst, local_id);

    _Pkt p;
    p.setBody(b);
    strcpy(p.header.id_dst, remote_id);
    strcpy(p.header.id_src, local_id);

    return writeData(&p);
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

int Loop_Server::sendUp(_Pkt *p){//向上级发送消息
    if(parent_conn != NULL && parent_conn->connected())
        return parent_conn->writeData(p);
    return 0;
}

int Loop_Server::sendDown(_Pkt *p, char *id_dst){ // 向下级设备发送报文
    map<int, ConnHandler *>::iterator it;
    ConnHandler *c;
    for (it = conns.begin(); it != conns.end(); it++) {
        c = it->second;
        if(id_dst == NULL || strcmp(c->getId(), id_dst) == 0){
            if(c->getId()[0] != 0) // 只向有标识的客户端下发
                c->writeData(p);
        }
    }
    return 0;
}

const char *Loop_Server::getId(){
    return id;
}

// 主循环
int Loop_Server::run (){
    int client_socket[MAX_CLIENT] ;  
    char remote_addr[32];
    int  remote_port;    
 
    if (open(port) == -1){
        LOG_F(INFO, "open listen port %d failed", port);
        return -1;
    }

    // 向 redis 注册自己的地址
    if(!reg()){
        LOG_F(ERROR, "register to redis failed, exit");
        return -1;
    }

    // done 连接上级服务器
    string parent_id = conf["parent_id"].as<string>();
    if(parent_id != "") {
        parent_conn = new ParentConnHandler(parent_id, redis);
        parent_conn->setLocalId(id);
        parent_conn->setTimeout(conf["timeout_sec"].as<int>());
        if( parent_conn->connect() != 0){
            LOG_F(WARNING, "connect to parent server failed, retry later!");
        }
    }

    // 启动逻辑处理线程
    LOG_F(INFO, "starting a thread");
    thread t;
    if(dev_type == DEVTP_ACC)
        t = thread(ACCBusi( m_queue, (BusiCallBack *)this));
    else if(dev_type == DEVTP_LCC)
        t = thread(LCCBusi( m_queue, (BusiCallBack *)this ) );
    else if(dev_type == DEVTP_SC) 
        t = thread(SCBusi(m_queue, (BusiCallBack *)this));
    else if(dev_type == DEVTP_SLE)
        t = thread(SLEBusi(m_queue, (BusiCallBack *)this));
    else
        LOG_F(ERROR, "invalid device type %d", dev_type);
    //thread t(proc);
    // t.join();
    // LOG_F(INFO, "thread exited");
    
    int i = 0, rcount = 0;
    map<int, ConnHandler *>::iterator it;

    LOG_F(INFO, "server id: %s type: %s running on %d", 
            id, conf["type"].as<string>().c_str(), port);
       
    while(TRUE)  
    {  
        i = 1;
        client_socket[0] = listen_sock;

        if(parent_conn != NULL && parent_conn->connected()) { // 添加上级socket进来
            client_socket[1] = parent_conn->handler();
            i += 1;
        }
        
        for (it = conns.begin(); it != conns.end(); it++) {
            if(i >= MAX_CLIENT){
                LOG_F(ERROR, "too many clients, exit");
                return -1;
            }

            client_socket[i++] = it->second->handler();
	    }
     
        rcount = tcp_check_read(client_socket, i, 5000);
        if (rcount < 0 ){
            LOG_F(ERROR, "select failed, break loop");
            break;
        }
        for(i=0; i<rcount; i++) {
            if(client_socket[i] == listen_sock) {
                int new_sock = tcp_accept(listen_sock, remote_addr, &remote_port);
                if(new_sock < 0) {
                    LOG_F(ERROR, "accept failed");
                }else{
                    sprintf(remote_addr, "%s:%d", remote_addr, remote_port);
                    LOG_F(INFO, "--new client [%s] connected", remote_addr);
                    handle_connections(new_sock, remote_addr);
                }
            }else{
                handle_data(client_socket[i]);
            }
        }
        handle_check();

    }
    t.join();
    return 0;  
}


int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp("test", argv[1]) == 0) {
        test_send_pkt();
        return 0;
    }

    if(argc == 1){
        LOG_F(ERROR, "config file as a paramter!");
        return 1;
    }
    const char *conf_file = argv[1];
    Loop_Server server(conf_file);

    server.run();
    return 0;
}
