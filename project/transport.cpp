#include "io.h"
#include "consts.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <random>

// Function pointers for input and output to a higher layer, in our case, no longer stdin and std out, but rather the security layer
ssize_t (*input)(uint8_t*, size_t); // Get data from higher layer
void (*output)(uint8_t*, size_t);   // Output data from higher layer

// Main function of transport layer; never quits
void listen_loop(int sockfd, struct sockaddr_in* addr, int initial_state, 
                 ssize_t (*input_func)(uint8_t*, size_t),
                 void (*output_func)(uint8_t*, size_t)) {

    // Set input and output function pointers
    input = input_func;
    output = output_func;

    // Your own implementation of the transport layer goes here
}
