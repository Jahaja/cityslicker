#ifndef NET_H_
#define NET_H_

#include <sys/epoll.h>

#define NET_CLIENT_BUF_LENGTH 1024*10
#define NET_MAX_EVENTS 1024
#define NET_READABLE 1
#define NET_WRITABLE 2

typedef struct net_server {
	int fd;
	int epfd;
	int num_clients;
	struct epoll_event *events;
	int max_events;
} net_server;

typedef struct net_client {
	int fd;
	char ip[16];
	int port;
	char buf[NET_CLIENT_BUF_LENGTH];
	int buflen;
} net_client;

typedef enum {
	net_file_event_type_client,
	net_file_event_type_server
} net_file_event_type_t;

typedef struct net_file_event {
	void *ptr;
	int mask;
	net_file_event_type_t type;
} net_file_event;

void net_server_close(net_server *s);
net_server *net_server_start(int port);
int net_poll(net_server *s);

#endif