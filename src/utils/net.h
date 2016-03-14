#ifndef NETWORK_H_
#define NETWORK_H_

//s32 create_server(void);
//s32 accept_client(s32 s);
int server_connect(int *psock, unsigned int server_ip);
int recvwait(int sock, void *buffer, int len);
int sendwait(int sock, const void *buffer, int len);

#endif
