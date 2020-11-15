//
// Created by Nathan Evans
//

#ifndef TCP_CHAT_TCP_CHAT_H
#define TCP_CHAT_TCP_CHAT_H

#define DEBUG 0

enum ChatMonType {
    MON_CONNECT = 675,
    MON_DISCONNECT,
    MON_DIRECT_MESSAGE,
    MON_MESSAGE
};

// Message sent from the chat monitor to the server
// or from the server to the chat monitor
struct ChatMonMsg {
    uint16_t type; // A ChatMonType
    uint16_t nickname_len; // Length of optional nickname to send with message
    uint16_t data_len; // Length of string message data
};

// Types of messages sent from chat client to chat server
enum ChatClientType {
    CLIENT_CONNECT = 10,
    CLIENT_DISCONNECT,
    CLIENT_SET_NICKNAME,
    CLIENT_SEND_MESSAGE,
    CLIENT_SEND_DIRECT_MESSAGE,
    CLIENT_GET_MEMBERS
};

struct ChatClientMessage {
    uint16_t type; // A ChatClientType
    uint16_t nickname_len; // Length of nickname appended to client message
    uint16_t data_length; // If additional data belongs to message, how long is it?
};

struct ServerErrorMessage {
  uint16_t error_type;
};

enum ErrorMessageTypes {
  UNKNOWN_TYPE = 999,
  INCORRECT_SIZE,
  WRONG_TYPE_FOR_CLIENT,
  WRONG_TYPE_FOR_MONITOR,
  NOT_CONNECTED
};

#endif //TCP_CHAT_TCP_CHAT_H
