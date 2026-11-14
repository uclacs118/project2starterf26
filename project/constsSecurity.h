#pragma once

#include <stdint.h>
#include <stdio.h>


// Security states -- these might be a little confusing
// If it starts with SERVER, that means it is a state on the server side, and vice versa for CLIENT
// The second part is the description of what the state is doing
// Ex: SERVER_CLIENT_HELLO_AWAIT means the server is waiting for the client hello message
#define SERVER_CLIENT_HELLO_AWAIT 0
#define CLIENT_CLIENT_HELLO_SEND 1
#define SERVER_SERVER_HELLO_SEND 2
#define CLIENT_SERVER_HELLO_AWAIT 3
#define SERVER_KEY_EXCHANGE_REQUEST_AWAIT 4
#define CLIENT_KEY_EXCHANGE_REQUEST_SEND 5
#define SERVER_FINISHED_SEND 6
#define CLIENT_FINISHED_AWAIT 7
#define DATA_STATE 8


// Security sizes
#define NONCE_SIZE 32
#define SECRET_SIZE 32
#define MAC_SIZE 32
#define IV_SIZE 16
#define PLAINTEXT_OFFSET (12 + IV_SIZE + MAC_SIZE)

// Security types
#define CLIENT_HELLO 0x00
#define NONCE_CLIENT_HELLO 0x01

#define CERTIFICATE 0xA0
#define PUBLIC_KEY 0xA1
#define SIGNATURE 0xA2

#define SERVER_HELLO 0x10
#define NONCE_SERVER_HELLO 0x11
#define NONCE_SIGNATURE_SERVER_HELLO 0x12

#define KEY_EXCHANGE_REQUEST 0x20
#define NONCE_SIGNATURE_KEY_EXCHANGE_REQUEST 0x22

#define FINISHED 0x30

#define DATA 0x40
#define INITIALIZATION_VECTOR 0x41
#define CIPHERTEXT 0x42
#define MESSAGE_AUTHENTICATION_CODE 0x43

static inline void print_security(const char* txt) {
    fprintf(stderr, "%s\n", txt);
}

static inline void print_hex(uint8_t* buf, size_t len) {
    for (int i = 0; i < len; i++) {
        fprintf(stderr, "%02x ", *(buf + i));
    }
    fprintf(stderr, "\n");
}
