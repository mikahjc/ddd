#include <string.h>
#include <malloc.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "net.h"

#define CHECK_ERROR(cond) if (cond) { goto error; }


static volatile int iLock = 0;

int sendwait(int sock, const void *buffer, int len)
{
    while (iLock)
        usleep(5000);
    iLock = 1;

    int ret;
    while (len > 0) {
        ret = send(sock, buffer, len, 0);
        CHECK_ERROR(ret <= 0);
        len -= ret;
        buffer += ret;
    }
    iLock = 0;
    return 0;
error:
    iLock = 0;
    return ret;
}

int recvwait(int sock, void *buffer, int len)
{
    while (iLock)
        usleep(5000);
    iLock = 1;

    int ret;
    int received = 0;
    while (len > 0)
    {
        ret = recv(sock, buffer, len, 0);
        if(ret > 0)
        {
            len -= ret;
            buffer += ret;
            received += ret;
        }
        else
        {
            received = ret < 0 ? ret : -1;
            break;
        }
    }

    iLock = 0;
    return received;
}
/*
s32 create_server(void) {
    socket_lib_init();

	s32 server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (server < 0)
		return -1;

    u32 enable = 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	struct sockaddr_in bindAddress;
	memset(&bindAddress, 0, sizeof(bindAddress));
	bindAddress.sin_family = AF_INET;
	bindAddress.sin_port = 7333;
	bindAddress.sin_addr.s_addr = INADDR_ANY;

	s32 ret;
	if ((ret = bind(server, (struct sockaddr *)&bindAddress, sizeof(bindAddress))) < 0) {
		socketclose(server);
		return ret;
	}
	if ((ret = listen(server, 3)) < 0) {
		socketclose(server);
		return ret;
	}

	return server;
}

s32 accept_client(s32 server)
{
    struct sockaddr addr;
    s32 addrlen = sizeof(addr);

    return accept(server, &addr, &addrlen);
}
*/


int server_connect(int *psock, unsigned int server_ip)
{
    struct sockaddr_in addr;
    int sock, ret;

    // No ip means that we don't have any server running, so no logs
    if (server_ip == 0) {
        *psock = -1;
        return 0;
    }

    socket_lib_init();

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
        return sock;

    addr.sin_family = AF_INET;
    addr.sin_port = 7333;
    addr.sin_addr.s_addr = server_ip;

    ret = connect(sock, (void *)&addr, sizeof(addr));
    if(ret < 0)
    {
        socketclose(sock);
        return -1;
    }

    int enable = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

    *psock = sock;
    iLock = 0;
    return 0;
}
