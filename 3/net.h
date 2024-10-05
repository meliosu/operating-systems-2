#ifndef PROXY_NET_H
#define PROXY_NET_H

int net_connect_remote(char *name, char *service);
int net_listen(int port, int backlog);

#endif /* PROXY_NET_H */
