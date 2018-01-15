#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <string>
#include <set>
#include <map>

#include <pthread.h>

#define HEADER_LENGTH 8
#define OPT_LENGTH 16        // probably will vary
#define IP_LENGTH 4
#define MSG_LENGTH 256      // maybe would be variable

std::map<std::string, void*(*)(void*)> handlers;    // key: msg header; value: handler
std::map<std::string, int> uname_sock;              // key: username  ; value: socket
std::map<int, std::string> sock_uname;              // key: socket    ; value: username
std::map<int, pthread_t*> sock_thread;              // key: socket    ; value: thread

struct sockets {
    int listener;
    std::set<int> accepted;
};

struct sockets sockets;

struct sockaddr_in serv_addr;

int readn(int s, char* buf, int b_remain) {
    int b_rcvd = 0;
    int rc;
    while(b_remain) {
        rc = read(s, buf + b_rcvd, b_remain);
        if (rc < 0) {
            return rc;
        }
        b_rcvd += rc;
        b_remain -= rc;
    }

    return b_rcvd;
}

void* auth_client(void* arg) {

    ssize_t rc;
    int sock_cli_sender = *((int*)arg);
    char buffer_opt[OPT_LENGTH];
    bzero(buffer_opt, OPT_LENGTH);

    rc = readn(sock_cli_sender, buffer_opt, OPT_LENGTH);

    std::cout << "user " << buffer_opt << " is online\n" << std::endl;                 // tbd

    uname_sock.insert(std::pair<std::string, int>(std::string(buffer_opt), sock_cli_sender));
    sock_uname.insert(std::pair<int, std::string>(sock_cli_sender, std::string(buffer_opt)));

    return NULL;
}

void* ret_msg(void* arg) {

    ssize_t rc;
    int sock_cli_sender = *((int*)arg);
    int sock_cli_recver;
    char buffer_opt[OPT_LENGTH];
    char buffer_msg[MSG_LENGTH];
    bzero(buffer_opt, OPT_LENGTH);
    bzero(buffer_msg, MSG_LENGTH);

    rc = readn(sock_cli_sender, buffer_opt, OPT_LENGTH);

    if (uname_sock.find(std::string(buffer_opt)) != uname_sock.end()) {
        sock_cli_recver = uname_sock[std::string(buffer_opt)];

    } else {
        std::cout << "user " << buffer_opt << " is offline\n" << std::endl;                 // tbd
        return NULL;
    }

    rc = readn(sock_cli_sender, buffer_msg, MSG_LENGTH);

    char buffer_head[HEADER_LENGTH] = "inc_msg";


    if (sock_uname.find(sock_cli_sender) != sock_uname.end()) {
        const char* tmp = (sock_uname[sock_cli_sender]).c_str();
        bcopy(tmp, buffer_opt, OPT_LENGTH);
    } else {
        char tmp[] = "unknown";
        bcopy(tmp, buffer_opt, OPT_LENGTH);
    }

    rc = write(sock_cli_recver, buffer_head, HEADER_LENGTH);
    rc = write(sock_cli_recver, buffer_opt, OPT_LENGTH);
    rc = write(sock_cli_recver, buffer_msg, MSG_LENGTH);

    if (rc < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    return NULL;
}

void* disc_client(void* arg) {

    int sockfd = *((int*)arg);

    std::cout << "disconnecting " << sockfd << std::endl;

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    sockets.accepted.erase(sockfd);
    uname_sock.erase(sock_uname[sockfd]);
    sock_uname.erase(sockfd);

    int* rc = new int(2);           // client disconnected return code
    return (void*)rc;
}

void* disc_chosen_client(void* arg) {

    char input_name[OPT_LENGTH];
    memset(input_name, 0, OPT_LENGTH);
    std::cin.getline(input_name, OPT_LENGTH, ' ');

    int sockfd = uname_sock[std::string(input_name)];

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    sockets.accepted.erase(sockfd);
    uname_sock.erase(std::string(input_name));
    sock_uname.erase(sockfd);

    pthread_t* thread = sock_thread[sockfd];
    pthread_detach(*thread);
    sock_thread.erase(sockfd);
    free(thread);

    return NULL;
}

void* close_server(void* arg) {
    printf("\nclosing...\n");
    int* rc = new int(1);
    return (void*)rc;
}

void* serv_msg(void* arg) {     // service msg to 1 client

    int rc;

    char header[HEADER_LENGTH] = "inc_msg";
    char input_name[OPT_LENGTH];
    char input_msg[MSG_LENGTH];
    memset(input_name, 0, OPT_LENGTH);
    std::cin.getline(input_name, OPT_LENGTH, ' ');
    std::cin.getline(input_msg, MSG_LENGTH);

    int sockfd = uname_sock[std::string(input_name)];

    char* buffer_head = header;
    char* buffer_opt = input_name;
    char* buffer_msg = input_msg;
    rc = write(sockfd, buffer_head, HEADER_LENGTH);
    rc = write(sockfd, buffer_opt, OPT_LENGTH);
    rc = write(sockfd, buffer_msg, MSG_LENGTH);

    if (rc < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    return NULL;
}

void* serv_msg_bc(void* arg) {  // service msg to all clients

    char input_msg[MSG_LENGTH];
    std::cin >> input_msg;

    for (std::set<int>::iterator it = sockets.accepted.begin();
         it != sockets.accepted.end(); it++) {

        int sockfd = *it;
        int rc;

        char header[HEADER_LENGTH] = "inc_msg";


        char* buffer_head = header;
        char  buffer_opt[OPT_LENGTH];
        bcopy(&(serv_addr.sin_addr.s_addr), buffer_opt, IP_LENGTH);

        char* buffer_msg = input_msg;
        rc = write(sockfd, buffer_head, HEADER_LENGTH);
        rc = write(sockfd, buffer_opt, OPT_LENGTH);
        rc = write(sockfd, buffer_msg, MSG_LENGTH);

        if (rc < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
    }

    return NULL;
}

void* wait_for_recv(void* arg) {

    int newsockfd = *(int*)arg;

    ssize_t rc;
    char buffer[HEADER_LENGTH];

    while(1) {

        bzero(buffer, HEADER_LENGTH);
        rc = readn(newsockfd, buffer, HEADER_LENGTH);

        std::cout << "got header: " << buffer << std::endl;

        if (rc < 0) {
            void* rc_disc = disc_client((void*)&newsockfd); // connection lost
            free(rc_disc);
            return NULL;
        }

        std::string header(buffer);
        if (handlers.find(header) != handlers.end()) {
            void* rc_handlers_recv = handlers[header]((void*)&newsockfd);

            if (rc_handlers_recv != NULL) {
                if (*((int*)rc_handlers_recv) == 2) {    // checking client disconnected condition
                    free(rc_handlers_recv);
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

void* monitor (void* arg) {

    handlers.insert(std::pair<std::string, void*(*)(void*)>(std::string("ret_msg"), &ret_msg));         // request for msg retransmission
    handlers.insert(std::pair<std::string, void*(*)(void*)>(std::string("disc_me"), &disc_client));     // request for client disconnecting
    handlers.insert(std::pair<std::string, void*(*)(void*)>(std::string("auth_me"), &auth_client));     // request for authorization

    pthread_t* thread_recv = new pthread_t;

    int newsockfd;
    int sockfd = sockets.listener;
    unsigned int clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    while (1) {
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        sockets.accepted.insert(newsockfd); // add to opened sockets list

        pthread_create(thread_recv, NULL, &wait_for_recv, (void*)&newsockfd);
        sock_thread.insert(std::pair<int, pthread_t*>(newsockfd, thread_recv));
    }
}

void* listen_cmd(void* arg) {

    std::map<std::string, void*(*)(void*)> handlers_cmd;

    handlers_cmd.insert(std::pair<std::string, void*(*)(void*)>(std::string("exit"), &close_server));
    handlers_cmd.insert(std::pair<std::string, void*(*)(void*)>(std::string("serv_msg"), &serv_msg));
    handlers_cmd.insert(std::pair<std::string, void*(*)(void*)>(std::string("serv_msg_bc"), &serv_msg_bc));
    handlers_cmd.insert(std::pair<std::string, void*(*)(void*)>(std::string("disc"), &disc_chosen_client));

    while(1) {
        std::string input;
        std::cin >> input;

        if (handlers_cmd.find(input) != handlers_cmd.end()) {
            void* rc_handlers_cmd = handlers_cmd[input](NULL);

            if (rc_handlers_cmd != NULL) {
                if (*((int*)rc_handlers_cmd) == 1) {    // checking exit condition
                    return NULL;
                }
            }
        }

    }
}

int main(int argc, char *argv[]) {

    int sockfd;
    uint16_t portno;
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);

    sockets.listener = sockfd;

    pthread_t thread_monitor;
    pthread_t thread_listen_cmd;
    pthread_create(&thread_monitor, NULL, &monitor, NULL);
    pthread_create(&thread_listen_cmd, NULL, &listen_cmd, NULL);

    printf("\nType 'exit' to end the program\n\n");
    pthread_join(thread_listen_cmd, NULL);
    pthread_detach(thread_monitor);

    std::set<int>::iterator it;
    for (it = sockets.accepted.begin(); it != sockets.accepted.end(); it++) {
        shutdown(*it, SHUT_RDWR);
        close(*it);
    }

    /* Closing socket */
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return 0;
}
