#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <arpa/inet.h>

#include "socket_format.h"

int socket_client() {

    //socket的建立
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_un addr_info;
    bzero(&addr_info, sizeof(addr_info));
    addr_info.sun_family = AF_UNIX;

    //localhost test
    strncpy(addr_info.sun_path, UI_SOCKET_PATH, sizeof(addr_info.sun_path));

    if (connect(socket_fd, (struct sockaddr *) &addr_info, sizeof(addr_info)) < 0) {
        perror("Client.connection error");
    }

    //Send a message to server
    char message[] = { "Hi server, I am client" };
    char receiveMessage[100] = { };
    send(socket_fd, message, sizeof(message), 0);
    recv(socket_fd, receiveMessage, sizeof(receiveMessage), 0);

    printf("%s", receiveMessage);
    printf("Client close socket\n");
    close(socket_fd);
    return 0;
}
