#ifndef NET_H_
#define NET_H_

typedef struct net_server {
	int fd;
	int num_clients;
} net_server;

typedef enum {
	net_command_type_bounding_box
} net_command_type_t;

typedef struct net_client_command {
	net_command_type_t type;
	void *ptr; // data read from client
} net_client_command;

net_server *net_start_server(int port);
int net_get_client(server *s);
void net_close_client(int fd);
net_client_command *net_read_command(int fd);

#endif