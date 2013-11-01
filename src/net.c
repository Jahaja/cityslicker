#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "net.h"
#include "cs.h"

#define NET_SERVER_BACKLOG 50
#define NET_EPOLL_HINT 1024

static void net_file_event_destroy(net_file_event *fe) {
    free(fe);
}

static net_file_event *net_file_event_create(void * ptr, net_file_event_type_t type) {
    net_file_event *fe = malloc(sizeof(net_file_event));
    fe->ptr = ptr;
    fe->type = type;
    fe->mask = 0;
    return fe;
}

static net_client *net_server_accept(net_server *s) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int fd = accept(s->fd, (struct sockaddr *) &addr, &addrlen);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    // TODO: add remote ip and port to the client?
    net_client *c = malloc(sizeof(net_client));
    c->fd = fd;
    strcpy(c->ip, inet_ntoa(addr.sin_addr));
    c->port = ntohs(addr.sin_port);

    net_file_event *fe = net_file_event_create((void *) c, net_file_event_type_client);

    struct epoll_event ee;
    ee.events |= EPOLLIN;
    ee.events |= EPOLLOUT;
    ee.data.u64 = 0;
    ee.data.ptr = (void *) fe;

    epoll_ctl(s->epfd, EPOLL_CTL_ADD, fd, &ee);

    printf("Client %s:%d connected\n", c->ip, c->port);

    return c;
}

static void net_client_close(net_server *s, net_client *c) {
    printf("Client %s:%d disconnected. Removing...\n", c->ip, c->port);
    epoll_ctl(s->epfd, EPOLL_CTL_DEL, c->fd, NULL);
    close(c->fd);
    free(c);
}

static int net_client_write(net_client *c) {
    if(c->buflen) {
        int written = write(c->fd, c->buf, c->buflen);
        if(written == 0) {
            return -1;
        }
        printf("Wrote %d bytes to %s\n", written, c->ip);
        c->buflen = 0;
        return written;
    }

    return 0;
}

static int net_client_read(net_client *c) {
    int num_read;
    int datalen = 16;
    char data[datalen];
    num_read = read(c->fd, data, datalen);
    if(num_read == 0) {
        return -1;
    }

    printf("Read %d bytes from client %s.\n", num_read, c->ip);

    if(num_read != datalen) {
        printf("Got some strange input, need 16 bytes!\n");
        return -1;
    }

    float *coords = (float *) &data;
    world *res = world_get_cities_in_bounding_box(
        loaded_world, 
        (double) coords[0], (double) coords[1], (double) coords[2], (double) coords[3]
    );

    int i;
    int *ids = (int *) &c->buf;
    for(i = 0; i < res->length; i++) {
        *(ids++) = res->cities[i]->id;
    }

    c->buflen = sizeof(int) * res->length;

    return num_read;
}

void net_server_close(net_server *s) {
    close(s->epfd);
    close(s->fd);
    free(s->events);
    free(s);
}

net_server *net_server_start(int port) {
    int optval = 1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    signal(SIGPIPE, SIG_IGN);

    net_server *s = malloc(sizeof(net_server));
    s->events = malloc(sizeof(struct epoll_event) * NET_MAX_EVENTS);
    s->max_events = NET_MAX_EVENTS;
    s->fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bind(s->fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    listen(s->fd, NET_SERVER_BACKLOG);

    // create epoll instance and add the server fd
    int epfd = epoll_create(NET_EPOLL_HINT);
    s->epfd = epfd;

    net_file_event *fe = net_file_event_create((void *) s, net_file_event_type_server);

    struct epoll_event ee;
    ee.events |= EPOLLIN;
    ee.data.u64 = 0;
    ee.data.ptr = (void *) fe;

    epoll_ctl(epfd, EPOLL_CTL_ADD, s->fd, &ee);

    return s;
}

int net_poll(net_server *s) {
    int num_events = epoll_wait(s->epfd, s->events, s->max_events, -1);
    int i;
    for(i = 0; i < num_events; i++) {
        net_file_event *fe = (net_file_event *) s->events[i].data.ptr;

        if(fe->type == net_file_event_type_client) {
            if(s->events[i].events & EPOLLIN) {
                if(net_client_read((net_client *)fe->ptr) == -1) {
                    net_client_close(s, (net_client *)fe->ptr);
                    net_file_event_destroy(fe);
                    continue;
                }
            }
            if (s->events[i].events & EPOLLOUT) {
                if(net_client_write((net_client *)fe->ptr) == -1) {
                    net_client_close(s, (net_client *)fe->ptr);
                    net_file_event_destroy(fe);
                    continue;
                }
            }
            if (s->events[i].events & EPOLLHUP || s->events[i].events & EPOLLERR) {
                net_client_close(s, (net_client *)fe->ptr);
                net_file_event_destroy(fe);
            }
        }
        else if(fe->type == net_file_event_type_server && s->events[i].events & EPOLLIN) {
            net_server_accept((net_server *)fe->ptr);
        }
    }
    return num_events;
}
