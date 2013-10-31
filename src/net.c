#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#define NET_SERVER_BACKLOG 50

void net_close_server(net_server *s) {
	close(s->fd);
	free(s);
}

net_server *net_create_server(int port) {
	int optval = 1;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	net_server *s = malloc(sizeof(net_server));
	s->fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	bind(s->fd, (const struct sockaddr *) addr, sizeof(struct sockaddr_in));
	listen(s->fd, NET_SERVER_BACKLOG);

	return s;
}

int net_get_client(net_server *s) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int fd = accept(s->fd, (const struct sockaddr *) &addr, &addrlen);

	// set receive timeout to 1 sec
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));

	return fd;
}

net_client_command *net_read_command(int fd) {
	int num_read;
	char buf[1024];
	num_read = read(fd, buf, sizeof(buf));
}

void net_close_client(int fd) {
	close(fd);
}