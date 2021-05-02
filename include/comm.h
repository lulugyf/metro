#ifndef _UTIL_TCP_H_
#define _UTIL_TCP_H_

// #ifndef zero
// #define zero(a) memset(a,0,sizeof(a))
// #endif

#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"

typedef void (*tcp_call_back) (int sockid, char *clientip, int clientport,
			       void *parm, int servsockid);

const char *tcp_name2ip (const char *server_name);

int tcp_close (int sd);

int tcp_socket ();

/*接收连接*/
int tcp_accept (int sd, char *ip, int *port);

/* 获取socket连接上本地地址 */
int tcp_get_sockaddr(int sd, char *addr) ;
/*
 * 绑定本地端口
 */
int tcp_bind (const char *local_ip, int port);

/*绑定本地端口并 进入 listen 状态*/
int tcp_listen (const char *local_ip, int port);

int tcp_connect (const char *server_name, int port);

int tcp_connecta (const char *server_name, int port, int timeout);

int tcp_read (int fd, char *ptr, int len);

int tcp_reada (int fd, char *ptr, int len, int timeout);

int tcp_write (int fd, const char *ptr, int len);

/* 从 fd 读取固定长度字节, 返回实际读取的字节数 */
int tcp_readn (int fd, char *ptr, int n);

/* 从 fd 读取固定长度字节, 返回实际读取的字节数, 超时控制timeout 毫秒 */
int tcp_readna (int fd, char *ptr, int n, int timeout);

/* 向 fd 写入固定长度的字节, 返回实际写入的字节数 */
int tcp_writen (int fd, const char *ptr, int nbytes);

int tcp_check_read (int *socks, int count, int timeout);

/*读取一行， 以 \r\n 分界*/
int tcp_readline(int sockid, char* command_buf, const int BUFLEN);
int tcp_writeline(int sockid, char *buf);



#endif /*_UTIL_TCP_H_*/