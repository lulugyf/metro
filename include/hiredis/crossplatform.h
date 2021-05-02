#ifndef HIREDIS_CROSSPLATFORM_H
#define HIREDIS_CROSSPLATFORM_H

#ifndef _WIN32

#include <sys/socket.h>
#include <poll.h>

typedef int fd_t;
typedef int sdf_t;

#define INVALID_FD -1

#define os_fcntl(fd, cmd, ...) \
        (fcntl(fd, cmd, ##__VA_ARGS__))

#else

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

typedef SOCKET fd_t;

#define INVALID_FD INVALID_SOCKET

#define F_GETFL		3	/* get file->f_flags */
#define F_SETFL		4	/* set file->f_flags */
#define O_NONBLOCK	00004000

int _os_fcntl(fd_t fd, int cmd, ... /* arg */ );

#define os_fcntl(fd, cmd, ...) \
        (_os_fcntl(fd, cmd, ##__VA_ARGS__))

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#ifndef nfds_t
#define nfds_t ULONG
#endif

int convertWsaErrorToUnixError(int error);

#endif

void os_close(fd_t fd);

int os_connect(fd_t sockfd, const struct sockaddr *addr, socklen_t addrlen);

int os_setsockopt(fd_t s, int level, int optname, const void *optval, socklen_t optlen);
int os_getsockopt(fd_t s, int level, int optname, void *optval, socklen_t *optlen);

ssize_t os_write(fd_t fd, const void *buf, size_t count);
ssize_t os_read(fd_t fd, void *buf, size_t count);

int os_poll(struct pollfd *fds, nfds_t nfds, int timeout);

void os_contextInit();
void os_contextCleanup();

#endif //HIREDIS_CROSSPLATFORM_H
