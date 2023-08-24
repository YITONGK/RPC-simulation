#include "rpc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>

int create_listening_socket(char* service);

struct rpc_server {
    int sockfd;
    /* Add variable(s) for server state */
    int num_functions;
};

rpc_server *rpc_init_server(int port) {
    int sockfd, newsockfd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    // translate port from int to string
    char service[20];
    sprintf(service, "%d", port);
    // Create the listening socket
    sockfd = create_listening_socket(service);

    // Listen on socket - means we're ready to accept connections,
    // incoming connection requests will be queued, man 3 listen
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept a connection - blocks until a connection is ready to be accepted
    // Get back a new file descriptor to communicate on
    client_addr_size = sizeof client_addr;
    newsockfd =
        accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
    if (newsockfd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    rpc_server* server = malloc(sizeof(rpc_server));
    assert(server != NULL);
    server->sockfd = newsockfd;
    server->num_functions = 0;
    return server;

}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    return -1;
}

void rpc_serve_all(rpc_server *srv) {

}

struct rpc_client {
    int sockfd;
    /* Add variable(s) for client state */
    int a;
};

struct rpc_handle {
    char name[1001];
    rpc_handler handler;
    /* Add variable(s) for handle */
    int a;
};

rpc_client *rpc_init_client(char *addr, int port) {
    int sockfd, s;
    struct addrinfo hints, *servinfo, *rp;

    // Create address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    // Get addrinfo of server. From man page:
    // The getaddrinfo() function combines the functionality provided by the
    // gethostbyname(3) and getservbyname(3) functions into a single interface
    char service[20];
    sprintf(service, "%d", port);
    s = getaddrinfo(addr, service, &hints, &servinfo);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Connect to first valid result
    // Why are there multiple results? see man page (search 'several reasons')
    // How to search? enter /, then text to search for, press n/N to navigate
    for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // success

        close(sockfd);
    }
    if (rp == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);

    rpc_client* client = malloc(sizeof(rpc_client));
    assert(client != NULL);
    client->sockfd = sockfd;
    return client;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {

}

void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}

int create_listening_socket(char* service) {
    int re, s, sockfd;
    struct addrinfo hints, *res;

    // Create address we're going to listen on (with given port number)
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // Connection-mode byte streams
    hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
    // node (NULL means any interface), service (port), hints, res
    s = getaddrinfo(NULL, service, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Reuse port if possible
    re = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Bind address to the socket
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    return sockfd;
}