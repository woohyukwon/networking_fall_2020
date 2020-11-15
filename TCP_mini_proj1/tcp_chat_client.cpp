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
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>

#include "tcp_chat.h"

bool quit = false;

std::string get_nickname() {
    std::string nickname;
    std::cout << "Enter chat nickname: ";
    std::getline(std::cin, nickname);
    return nickname;
}

std::string get_message() {
    std::string msg;
    std::cout << "Enter chat message to send, or quit to quit: ";
    std::getline(std::cin, msg);
    //std::cerr << "Got input " << msg << " from user" << std::endl;
    return msg;
}

// Handler for when ctrl+c is pressed.
// Just set the global 'stop' to true to shut down the server.
void handle_ctrl_c(int the_signal) {
    std::cout << "Handled sigint\n";
    quit = true;
}

/**
 *
 * Chat client example. Reads in HOST PORT
 *
 * e.g., ./tcpchatclient 127.0.0.1 8888
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

    // The socket used to connect/send/receive data to the TCP server
    int client_socket;
    // Variable used to check return codes from various functions
    int ret;

    std::string nickname;

    // Note: this needs to be 3, because the program name counts as an argument!
    if (argc < 3) {
        std::cerr << "Please specify HOST PORT as first two arguments." << std::endl;
        return 1;
    }
    // Set up variables "aliases"
    ip_string = argv[1];
    port_string = argv[2];

    // Signal handler setup, done for you! This allows you to hit ctrl+c when running from the command line
    // This will set the global quit variable to true and allow you to cleanly shut down from the program
    // E.g., send a disconnect message to the server and then close the TCP connection
    struct sigaction ctrl_c_handler;
    ctrl_c_handler.sa_handler = handle_ctrl_c;
    sigemptyset(&ctrl_c_handler.sa_mask);
    ctrl_c_handler.sa_flags = 0;
    sigaction(SIGINT, &ctrl_c_handler, NULL);

    // Create the TCP socket.
    // AF_INET is the address family used for IPv4 addresses
    // SOCK_STREAM indicates creation of a TCP socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Make sure socket was created successfully, or exit.
    if (client_socket == -1) {
        std::cerr << "Failed to create tcp socket!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return 1;
    }

    // TODO: Get the IP address in binary format using get getaddrinfo!
    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *results_it;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_family = AF_INET;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    ret = getaddrinfo(ip_string, port_string, &hints, &results);

    if (ret != 0) {
        std::cerr << "Getaddrinfo failed with error " << ret << std::endl;
        perror("getaddrinfo");
        return 1;
    }

    nickname = get_nickname();

    // TODO: Connect to TCP Chat Server using connect()
    results_it = results;
    while (results_it != NULL) {
        ret = connect(client_socket, results_it->ai_addr, results_it->ai_addrlen);
        if (ret == 0) {
            break;
        }
        perror("connect");
        results_it = results_it->ai_next;
    }
    results_it = results;

    if (ret != 0) {
        std::cerr << "Failed to connect to remote server!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        close(client_socket);
        return 1;
    }

    struct ChatClientMessage client_message;
    memset(&client_message, 0, sizeof(client_message));
    // TODO: Send connect message
    // Fill in client_message and send to the server
    ChatClientMessage *connect = new ChatClientMessage;
    connect->type = htons(CLIENT_CONNECT);
    connect->nickname_len = htons(0);
    connect->data_length = htons(0);
    ret = send(client_socket, connect, sizeof(ChatClientMessage), 0);

    // TODO: Send nickname message
    ChatClientMessage *nick = (ChatClientMessage *) malloc(sizeof(ChatClientMessage) + strlen(nickname.c_str()));
    nick->type = htons(CLIENT_SET_NICKNAME);
    nick->nickname_len = htons(0);
    nick->data_length = htons(strlen(nickname.c_str()));
    memcpy(&nick[1],nickname.c_str(),strlen(nickname.c_str()));
    ret = send(client_socket, nick, sizeof(ChatClientMessage)+strlen(nickname.c_str()), 0);

    // Now enter a loop to send the chat messages from this client to the server
    std::string next_message;
    next_message = get_message();

    while ((next_message != "quit") && (quit == false)) {
        std::cout << "Sending message " << next_message << std::endl;
        // TODO: parse command from next_message, either a regular message, a direct message, or a LIST message
        //       then send to the server the correct message type and data based on that
        if(next_message[0] == '/')
            //direct message
        {
            int l = next_message.length();
            std::string direct_name;
            for(int i=1;i<l;i++)
            {
                if(next_message[i] == '/')
                {
                    direct_name = next_message.substr(1, i-1);
                    break;
                }
            }
            ChatClientMessage *direct = (ChatClientMessage *) malloc(sizeof(ChatClientMessage) + strlen(next_message.c_str()));
            direct->type = htons(14);
            direct->data_length = htons(strlen(next_message.c_str()));
            direct->nickname_len = htons(0);
            memcpy(&direct[1],next_message.c_str(),strlen(next_message.c_str()));
            ret = send(client_socket,direct, sizeof(ChatClientMessage) + strlen(next_message.c_str()), 0);
            std::cout<< "Client got nickname to send to " << direct_name << std::endl;
            //LIST message
        }else if(next_message == "LIST"){
            ChatClientMessage *list = new ChatClientMessage;
            list->type = htons(15);
            list->data_length = htons(0);
            list->nickname_len = htons(0);
            ret = send(client_socket,list, sizeof(ChatClientMessage),0);
            //regular message
        }else{
            ChatClientMessage *sending = (ChatClientMessage *) malloc(sizeof(ChatClientMessage) + strlen(next_message.c_str()));
            sending->type = htons(13);
            sending->data_length = htons(strlen(next_message.c_str()));
            sending->nickname_len = htons(0);
            memcpy(&sending[1],next_message.c_str(),strlen(next_message.c_str()));
            ret = send(client_socket,sending, sizeof(ChatClientMessage)+strlen(next_message.c_str()),0);
        }
        next_message = get_message();
    }

    // TODO: build and send a client disconnect message to the server here
    ChatClientMessage *disconnect = new ChatClientMessage;
    disconnect->type = htons(11);
    disconnect->data_length = htons(0);
    disconnect->nickname_len = htons(0);
    ret = send(client_socket,disconnect, sizeof(ChatClientMessage),0);
    std::cout<< "Sending CLIENT_DISCONNECT message" << std::endl;
    std::cout<< "Send returned " << ret << std::endl;

    close(client_socket);
    return 0;
}
