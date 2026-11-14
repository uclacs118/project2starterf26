#pragma once
#include <stdint.h>


/**
 * Run the TCP transport protocol from project 1 for a peer on an already-open socket.
 *
 * This function handles the transport handshake and subsequent data transfer, 
 * `sockfd` must refer to the socket you are on connection, and `addr` identifies that peer.
 *
 * For a server, the caller must first obtain the client connection and its
 * address (for example through peeking on a socket read`) and pass them here. 
 * The caller must not read any bytes from the connection before calling this function;
 * the handshake is to be handled here from its beginning.
 *
 * `input_p` supplies a higher level data for transmission, while `output_p`
 * receives higher level data delivered by the peer. While this could be
 * the application layer or stdin/stdout it could be level 5 TLS as in this project
 * The callbacks are used by the protocol implementation as data becomes available or is needed.
 *
 * @param sockfd Connected socket descriptor for the peer.
 * @param addr Address of the peer associated with `sockfd`.
 * @param initial_state Initial protocol state for this endpoint.
 * @param input_p Callback that provides bytes to send.
 * @param output_p Callback that consumes bytes received from the peer.
 */
void listen_loop(int sockfd, struct sockaddr_in* addr, int initial_state,
                 ssize_t (*input_p)(uint8_t*, size_t),
                 void (*output_p)(uint8_t*, size_t));
