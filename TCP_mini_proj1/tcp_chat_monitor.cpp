//
// Created by Nathan Evans
//
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
#include "tcp_chat.h"

static bool stop = false;
void handle_ctrl_c(int the_signal) {
    std::cout << "Handled sigint\n";
    stop = true;
}

/**
 *
 * TCP client example. Reads in IP PORT
 * from the command line, and sends DATA via TCP to IP:PORT.
 *
 * e.g., ./tcpclient 127.0.0.1 8888
 *
 * @param argc count of arguments on the command line
 * @param argv array of command line arguments
 * @return 0 on success, non-zero if an error occurred
 */
int main(int argc, char *argv[]) {
    // Alias for argv[1] for convenience
    char *ip_string;
    // Alias for argv[2] for convenience
    char *port_string;
    // Alias for argv[3] for convenience
    char *nickname;

    // Port to send TCP data to. Need to convert from command line string to a number
    unsigned int port;
    // The socket used to send data
    int tcp_socket;
    // Variable used to check return codes from various functions
    int ret;
    struct pollfd pfds[2];
    // IPv4 structure representing and IP address and port of the destination
    struct sockaddr_in dest_addr;
    socklen_t dest_addr_len;
    char recv_buf[2048];
    int timeout = 2000;

    // Set dest_addr to all zeroes, just to make sure it's not filled with junk
    // Note we could also make it a static variable, which will be zeroed before execution
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));

    // Signal handler to deal with quitting the program appropriately
    struct sigaction ctrl_c_handler;
    ctrl_c_handler.sa_handler = handle_ctrl_c;
    sigemptyset(&ctrl_c_handler.sa_mask);
    ctrl_c_handler.sa_flags = 0;
    sigaction(SIGINT, &ctrl_c_handler, NULL);

    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *results_it;

    // Note: this needs to be 4, because the program name counts as an argument!
    if (argc < 3) {
        std::cerr << "Please specify IP PORT as first two arguments." << std::endl;
        return 1;
    }
    std::string test;
    test = std::string(nickname);
    nickname = NULL;
    // Indicates that a nickname was provided for the monitor (for direct messages)
    if (argc == 4) {
        nickname = argv[3];
    }

    // Set up variables "aliases"
    ip_string = argv[1];
    port_string = argv[2];

    // Create the TCP socket.
    // AF_INET is the address family used for IPv4 addresses
    // SOCK_STREAM indicates creation of a TCP socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Make sure socket was created successfully, or exit.
    if (tcp_socket == -1) {
        std::cerr << "Failed to create tcp socket!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return 1;
    }

    // TODO: build a chat client message of type MON_CONNECT
    //       if a nickname was provided, include that in the message as well

    // TODO: send the MON_CONNECT message to the server
    // Check if send worked, clean up and exit if not.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_family = AF_INET;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    // Instead of using inet_pton, use getaddrinfo to convert.
    ret = getaddrinfo(ip_string, port_string, &hints, &results);

    if (ret != 0) {
        std::cerr << "Getaddrinfo failed with error " << ret << std::endl;
        perror("getaddrinfo");
        return 1;
    }

    // Check we have at least one result
    results_it = results;

    while (results_it != NULL) {
        ret = connect(tcp_socket, results_it->ai_addr, results_it->ai_addrlen);
        if (ret == 0) {
            break;
        }
        ret = -1;
        perror("connect");
        results_it = results_it->ai_next;
    }

    freeaddrinfo(results);

    if (ret != 0) {
        std::cout << "Failed to connect to any addresses!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        close(tcp_socket);
        return 1;
    }

    std::cout << "Connected to server!"<< std::endl;

    ChatMonMsg *connect = new ChatMonMsg;
    connect->type = htons(675);
    connect->data_len = htons(0);
    connect->nickname_len = htons(0);

    ret = send(tcp_socket, connect, sizeof(ChatMonMsg),0);
    std::cout<<"Sent " << ret << " bytes of data" << '\n';

    // Placeholder for messages received from the server
    struct ChatMonMsg server_message;
    // After sending the connect monitor message, the monitor will just
    // sit and wait for messages to output.

    pfds[0].fd = tcp_socket;
    pfds[0].events = POLLIN;
    pfds[1].fd = 0;
    pfds[1].events = POLLIN;
    int num = 0;

    while(stop == false) {
        // TODO: receive messages from the server
        //       when a message from the server is received, you should determine its type and data, then print
        //       out the chat message to the screen, including the nickname of the sender
        // TODO: read from stdin, in case the user types 'quit'
        num = poll(pfds, 2, 5000);
        char buff[2048];

        //check to see if data present on tcpsocket
        if (pfds[0].revents & POLLIN) {

            //read it into buffer.
            ret = recv(pfds[0].fd, buff, 2048, 0);
            //add null terminator
            buff[ret] = '\0';


            ChatMonMsg *receive = (ChatMonMsg *) buff;

            char *nickname = (char *) malloc(ntohs(receive->nickname_len));

            if (ntohs(receive->type) == MON_MESSAGE) {

                int offset = 0;

                while (offset < ret) {

                    if (ntohs(receive->data_len) + ntohs(receive->nickname_len) + sizeof(ChatMonMsg) <= ret) {
                        char *data = (char *) malloc(ntohs(receive->data_len));
                        receive = (ChatMonMsg *) &buff[offset];
                        memcpy(nickname, &receive[1], ntohs(receive->nickname_len));
                        memcpy(data, buff + sizeof(ChatMonMsg) + ntohs(receive->nickname_len), ntohs(receive->data_len));
                        offset += sizeof(ChatMonMsg) + ntohs(receive->nickname_len) + ntohs(receive->data_len);
                        std::cout << nickname << " said: " << data << std::endl;

                    }

                }
            } else if (ntohs(receive->type) == MON_DIRECT_MESSAGE) {
                int offset = 0;
                while (offset < ret) {

                    if (ntohs(receive->data_len) + ntohs(receive->nickname_len) + sizeof(ChatMonMsg) <= ret) {
                        char *data = (char *) malloc(ntohs(receive->data_len));
                        receive = (ChatMonMsg *) &buff[offset];
                        memcpy(nickname, &receive[1], ntohs(receive->nickname_len));
                        memcpy(data, buff + sizeof(ChatMonMsg) + ntohs(receive->nickname_len), ntohs(receive->data_len));
                        offset += sizeof(ChatMonMsg) + ntohs(receive->nickname_len) + ntohs(receive->data_len);
                        std::string next_message = std::string(data);

                        int l = next_message.length();
                        std::string direct_name;
                        std::string direct_message;
                        for (int i = 1; i < l; i++) {
                            if (data[i] == '/') {
                                direct_name = next_message.substr(1, i - 1);
                                direct_message = next_message.substr(i + 1, l - 1);
                                break;
                            }
                        }
                        if (direct_name == nickname) {
                            std::cout << "[DIRECT] " << nickname << " said: " << direct_message << std::endl;

                        }

                    }
                }
            }
        }
    }
    // TODO: build and send a MON_DISCONNECT message to let the server know this monitor has gone away
    ChatMonMsg *disconnect = new ChatMonMsg;
    disconnect->type = htons(MON_DISCONNECT);
    disconnect->data_len = htons(0);
    disconnect->nickname_len = htons(0);
    ret = send(tcp_socket, &disconnect, sizeof(ChatMonMsg),0);
    std::cout << "Shut down message sent to server, exiting!\\n" << std::endl;
    close(tcp_socket);
    return 0;
}
