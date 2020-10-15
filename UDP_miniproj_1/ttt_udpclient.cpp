//
// Created by Liangyin Yu on 10/14/20.
//

#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "tictactoe.h"

int main(int argc, char *argv[]) {
    // Alias for argv[1] for convenience
    char *ip_string;
    // Alias for argv[2] for convenience
    char *port_string;
    // Alias for argv[3] for convenience
    char *data_string;
    // Port to send UDP data to. Need to convert from command line string to a number
    unsigned int port;
    // The socket used to send UDP data on
    int udp_socket;
    // Variable used to check return codes from various functions
    int ret;

    // IPv4 structure representing and IP address and port of the destination
    struct sockaddr_in dest_addr;
    /* buffer to use for receiving data */
    static char recv_buf[2048];

    /* recv_addr is the client who is talking to us */
    struct sockaddr_in recv_addr;
    /* recv_addr_size stores the size of recv_addr */
    socklen_t recv_addr_size;
    /* buffer to use for sending data */
    static char send_buf[2048];

    // Set dest_addr to all zeroes, just to make sure it's not filled with junk
    // Note we could also make it a static variable, which will be zeroed before execution
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));

    // Note: this needs to be 4, because the program name counts as an argument!
    if (argc < 3) {
        std::cerr << "Please specify IP PORT DATA as first three arguments." << std::endl;
        return 1;
    }
    // Set up variables "aliases"
    ip_string = argv[1];
    port_string = argv[2];

    // Create the UDP socket.
    // AF_INET is the address family used for IPv4 addresses
    // SOCK_DGRAM indicates creation of a UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Make sure socket was created successfully, or exit.
    if (udp_socket < 0) {
        std::cerr << "Failed to create udp socket!" << std::endl;
        return 1;
    }

    // convert ip address to binary form, store in dest
    ret = inet_pton(AF_INET, ip_string, (void *) &dest_addr.sin_addr);
    if (ret == -1) {
        std::cerr << "inet_pton conversion failed" << std::endl;
        return 1;
    }

    // convert port into binary form, store in dest
    ret = sscanf(port_string, "%u", &port);
    if (ret != 1) {
        std::cout << "Failed to convert port properly." << std::endl;
        return 1;
    }
    dest_addr.sin_port = htons(port);
    // set address type of dest
    dest_addr.sin_family = AF_INET;

    //create the game message
    struct GetGameMessage getMessage;
    getMessage.client_id = htons(rand());
    getMessage.hdr.type = htons(ClientGetGame);
    getMessage.hdr.len = htons(sizeof(struct GetGameMessage));

    // Send the data to the destination.
    std::cout << "Will send `GetGameMessage' via UDP to " << ip_string << ":" << port_string << std::endl;

    // Note 1: we are sending strlen(data_string) (don't include the null terminator)
    // Note 2: we are casting dest_addr to a struct sockaddr because sendto uses the size
    //         and family to determine what type of address it is.
    // Note 3: the return value of sendto is the number of bytes sent

    //send the data to server
    ret = sendto(udp_socket, &getMessage, sizeof(struct GetGameMessage), 0,
                 (struct sockaddr *) &dest_addr, sizeof(struct sockaddr_in));

    // Check if send worked, clean up and exit if not.
    if (ret <= 0) {
        std::cerr << "Sendto failed" << std::endl;
        close(udp_socket);
        return 1;
    }
    std::cout << "Sent " << ret << " bytes out via UDP" << std::endl;


    /**
   * Code to receive response from the server goes here!
   * recv or recvfrom...
   */

    // Receive data
    recv_addr_size = sizeof(struct sockaddr_in);
    ret = recvfrom(udp_socket, recv_buf, 2047, 0, (struct sockaddr *) &dest_addr, &recv_addr_size);
    std::cout << "Received " << ret << " bytes from " << ip_string << ":" << port_string << std::endl;

    //create a summary
    struct GameSummaryMessage *summary;
    summary = (struct GameSummaryMessage *) recv_buf;

    summary->hdr.type = ntohs(summary->hdr.type);
    summary->hdr.len = ntohs(summary->hdr.len);
    summary->client_id = ntohs(summary->client_id);;
    summary->game_id = ntohs(summary->game_id);
    summary->x_positions = ntohs(summary->x_positions);
    summary->o_positions = ntohs(summary->o_positions);

    std::cout << "Received game result length " << summary->hdr.len << " client ID " << summary->client_id
              << " game ID " << summary->game_id << std::endl;
    std::cout << "x positions: " << summary->x_positions << ", o positions: " << summary->o_positions << std::endl;
    char board[9];
    char initBoard[9] = {'0', '1', '2', '3', '4', '5', '6', '7', '8'};

    struct GameResultMessage result;
    result.hdr.type = htons(ClientResult);
    result.hdr.len = htons(sizeof(GameResultMessage));
    result.game_id = htons(summary->game_id);
    result.result = 0;

    bool move = false;
    //create the board and set the appropriate bit if X or O should be in that position
    for (int i = 0; i < 9; i++) {
        board[i] = initBoard[i];
    }
    for (int i = 0; i < 9; i++) {
        int posX = (summary->x_positions >> i) & 1;
        int posO = (summary->o_positions >> i) & 1;
        if (posX == 1 && posO == 1) {
            result.result = htons(INVALID_BOARD);
            board[i] = 'N';
            move = true;
        } else if (posX == 1) {
            board[i] = 'X';
        } else if (posO == 1) {
            board[i] = 'O';
        } else {
            board[i] = ' ';
        }
    }

    std::cout << std::endl << std::endl << "Tic Tac Toe" << std::endl << std::endl;

    std::cout << "Player 1 (X) - Player 2 (O)" << std::endl << std::endl;

    std::cout << "     |     |     " << std::endl
              << "  " << board[0] << "  |  " << board[1] << "  |  " << board[2] << "  " << std::endl
              << "_____|_____|_____" << std::endl
              << "     |     |     " << std::endl
              << "  " << board[3] << "  |  " << board[4] << "  |  " << board[5] << "  " << std::endl
              << "_____|_____|_____" << std::endl
              << "     |     |     " << std::endl
              << "  " << board[6] << "  |  " << board[7] << "  |  " << board[8] << "  " << std::endl
              << "     |     |     " << std::endl;

    bool x_pos = false;
    bool o_pos = false;

    int j = 0;
    while (j < 3) {
        if (board[j] == 'X' && board[j + 3] == 'X' && board[j + 6] == 'X') {
            result.result = htons(X_WIN);
            x_pos = true;
        } else if (board[j] == 'O' && board[j + 3] == 'O' && board[j + 6] == 'O') {
            result.result = htons(O_WIN);
            o_pos = true;
        }
        j++;
    }
    j = 0;
    while (j < 7) {
        if (board[j] == 'X' && board[j + 1] == 'X' && board[j + 2] == 'X') {
            result.result = htons(X_WIN);
            x_pos = true;
        } else if (board[j] == 'O' && board[j + 1] == 'O' && board[j + 2] == 'O') {
            result.result = htons(O_WIN);
            o_pos = true;
        }
        j += 3;
    }
    if ((board[0] == 'X' && board[4] == 'X' && board[8] == 'X') ||
        (board[2] == 'X' && board[4] == 'X' && board[6] == 'X')) {
        result.result = htons(X_WIN);
        x_pos = true;
    } else if ((board[0] == 'O' && board[4] == 'O' && board[8] == 'O') ||
               (board[2] == 'O' && board[4] == 'O' && board[6] == 'O')) {
        result.result = htons(O_WIN);
        o_pos = true;
    }

    if (x_pos == false && o_pos == false) {
        result.result = htons(CATS_GAME);
    }
    if (x_pos && o_pos) {
        move = true;
        result.result = htons(INVALID_BOARD);
    }

    if (ntohs(result.result) == 11) {
        std::cout << "X is a winner!" << std::endl;
    } else if (ntohs(result.result) == 12) {
        std::cout << "O is a winner" << std::endl;
    } else if (ntohs(result.result) == 13) {
        std::cout << "CATS_GAME" << std::endl;
    } else if (ntohs(result.result) == 14) {
        std::cout << "INVALID_BOARD" << std::endl;
    }

    //send the result
    ret = sendto(udp_socket, &result, sizeof(struct GameResultMessage), 0, (struct sockaddr *) &dest_addr,
                 sizeof(struct sockaddr_in));
    std::cout << "Sent " << ret << " bytes out via UDP" << std::endl;

    if (ret <= 0) {
        std::cerr << "sendto failed for some reason" << std::endl;
        close(udp_socket);
        return 1;
    }

    ret = recvfrom(udp_socket, recv_buf, 2047, 0, (struct sockaddr *) &dest_addr, &recv_addr_size);
    std::cout << "Received " << ret << " bytes from " << ip_string << ":" << port_string << std::endl;

    //create a TTTmessage
    struct TTTMessage *message;
    message = (struct TTTMessage *) recv_buf;
    message->type = ntohs(message->type);
    message->len = ntohs(message->len);

    //output the result from the server
    if (message->type == 113) {
        std::cout << "Server Invalid Request Reply " << std::endl;
    } else if (message->type == 115) {
        std::cout << "Got CORRECT result from the server " << std::endl;
    } else if (message->type == 116) {
        std::cout << "Got INCORRECT result from the server " << std::endl;
    }

    close(udp_socket);
    return 0;
}
