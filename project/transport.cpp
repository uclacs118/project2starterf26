#include "io.h"
#include "consts.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <random>

bool print_diagnostics = true;

int state = 0;           // Current state for handshake
int dup_acks = 0;        // Duplicate acknowledgements received
uint32_t ack = 0;        // Acknowledgement number
uint32_t seq = 0;        // Sequence number
uint32_t last_ack = 0;   // Last ACK number to keep track of duplicate ACKs
bool force_ack = false;  // Require ACK to be sent out

buffer recv_buf = {};
buffer send_buf = {};

struct timeval start; // Last packet sent at this time
struct timeval now;   // Temp for current time

// Function pointers for input and output to a higher layer, in our case, no longer stdin and std out, but rather the security layer
ssize_t (*input)(uint8_t*, size_t); // Get data from higher layer
void (*output)(uint8_t*, size_t);   // Output data from higher layer

// On ACK receipt, clear all in send buffer that are less than
void clear_send_buf(uint32_t other_ack) {
    for (auto it = send_buf.packets.begin(); it != send_buf.packets.end();) {
        if (ntohl(it->seq) < other_ack) {
            send_buf.bytes -= ntohs(it->length);
            it = send_buf.packets.erase(it);
        } else {
            ++it;
        } 
    }

    print_buf(&send_buf);
}

// Clear recv buf to write out sequential packets
void clear_recv_buf() {
    bool found = false;
    // Until no more found ACKs
    do {
        found = false;
        for (auto it = recv_buf.packets.begin(); it != recv_buf.packets.end();
             ++it) {
            // If found packet with next expected
            if (ntohl(it->seq) == ack) {
                found = true;

                // Set ACK to next expected
                uint32_t sack = ntohl(it->seq) + ntohs(it->length);
                if (sack > ack)
                    ack = sack;

                // Output contents
                output(it->payload, ntohs(it->length));

                recv_buf.bytes -= ntohs(it->length);
                recv_buf.packets.erase(it);
                break;
            }
        }
    } while (found);
}

// Push packet to buffer
packet* push(buffer* buf, packet* pkt) {
    size_t s = ntohs(pkt->length);
    buf->packets.push_back(*pkt);
    buf->bytes += s;


    return &buf->packets.back();
}

// Set ACK in packet to current ACK
void set_ack_packet(packet* pkt) {
    if (state != NORMAL)
        return;
    pkt->flags = pkt->flags | ACK; // Set ACK flag
    pkt->ack = htonl(ack);
}

// Get data from standard input / make handshake packets
packet* get_data() {
    switch (state) {
    case SERVER_AWAIT:
        return NULL;
    case CLIENT_START: {
        packet pkt = {0};
        pkt.flags = SYN;
        pkt.seq = htonl(seq);
        seq++;
        state = CLIENT_AWAIT;
        return push(&send_buf, &pkt);
    }
    case SERVER_SYN: {
        packet pkt = {0};
        pkt.flags = SYN_ACK;
        pkt.seq = htonl(seq);
        seq++;
        pkt.ack = htonl(ack);
        state = SERVER_AWAIT_2;
        return push(&send_buf, &pkt);
    }
    case CLIENT_AWAIT:
        return NULL;
    case SERVER_AWAIT_2:
        return NULL;
    case CLIENT_AWAIT_2:
    default: {
        // Get rest of window or max payload
        uint16_t length = MAX_WINDOW - send_buf.bytes < MAX_PAYLOAD
                      ? MAX_WINDOW - send_buf.bytes
                              : MAX_PAYLOAD;
        if (length <= 0)
            return NULL;

        // Get data from layer
        packet pkt = {};
        length = input(pkt.payload, length);
        if (length <= 0 && state != CLIENT_AWAIT_2)
            return NULL;

        // Return packet
        set_ack_packet(&pkt);
        pkt.seq = htonl(seq);
        pkt.length = htons(length);
        seq += length == 0 && state == CLIENT_AWAIT_2 ? 1 : length;
        state = NORMAL;
        return push(&send_buf, &pkt);
    }
    }
}

// Process data received from socket
void recv_data(packet* pkt) {
    switch (state) {
    case SERVER_AWAIT: {
        // If not a handshake packet, drop
        if (!(pkt->flags & SYN))
            break;
        ack = ntohl(pkt->seq) + 1;
        state = SERVER_SYN;
    }
    case CLIENT_START:
        break;
    case SERVER_SYN:
        break;
    case CLIENT_AWAIT: {
        // If not a handshake packet, drop
        if (!(pkt->flags & SYN_ACK))
            break;
        ack = ntohl(pkt->seq) + 1;
        clear_send_buf(ntohl(pkt->ack));
        state = CLIENT_AWAIT_2;
        break;
    }
    case SERVER_AWAIT_2:
        state = NORMAL;
        force_ack = true;
        if (ntohs(pkt->length) == 0)
            ack = ntohl(pkt->seq) + 1;
    default: {
        // If ACK flag set
        if (pkt->flags & ACK) {
            uint32_t other_ack = ntohl(pkt->ack);
            if (other_ack == last_ack)
                dup_acks++;
            else
                last_ack = other_ack;
            clear_send_buf(other_ack);
        }

        // If data in packet, must send ACK
        if (ntohs(pkt->length) > 0) {
            force_ack = true;
        }

        push(&recv_buf, pkt);
        clear_recv_buf();
    }
    }
}

// Main function of transport layer; never quits
void listen_loop(int sockfd, struct sockaddr_in* addr, int initial_state, 
                 ssize_t (*input_func)(uint8_t*, size_t),
                 void (*output_func)(uint8_t*, size_t)) {

    // Set initial state (whether client or server)
    state = initial_state;

    // Set input and output function pointers
    input = input_func;
    output = output_func;

    // Set socket for nonblocking
    int flags = fcntl(sockfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    // Setting timers
    gettimeofday(&now, NULL);
    gettimeofday(&start, NULL);

    // Set initial sequence number, this is overkill, but it ensures that the sequence number is random and not predictable
    std::random_device rd;
    std::mt19937 gen(rd());

    // Uniform distribution up to half of the max value of a 32-bit unsigned integer, to avoid overflow when adding payload lengths
    std::uniform_int_distribution<uint32_t> dist(0, 0b1 << 31);
    seq = dist(gen);

    // Create buffer for incoming data
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Start listen loop
    while (true) {
        packet pkt = {};
        // Get data from socket
        int bytes_recvd = recvfrom(sockfd, &pkt, sizeof(pkt), 0,
                                   (struct sockaddr*) addr, &addr_size);
        // If data, process it
        if (bytes_recvd > 0) {
            print_diag(&pkt, RECV);
            recv_data(&pkt);
        }

        packet* tosend = get_data();
        // Data available to send
        if (tosend != NULL) {
            set_ack_packet(tosend);
            print_diag(tosend, SEND);
            sendto(sockfd, tosend, PACKET_HEADER_SIZE + ntohs(tosend->length), 0,
                   (struct sockaddr*) addr, sizeof(struct sockaddr_in));
        }
        // Received a packet and must send an ACK
        else if (force_ack) {
            packet tosend = {0};
            set_ack_packet(&tosend);
            print_diag(&tosend, SEND);
            sendto(sockfd, &tosend, PACKET_HEADER_SIZE, 0, (struct sockaddr*) addr,
                   sizeof(struct sockaddr_in));
        }
        force_ack = false;

        // Check if timer went off
        gettimeofday(&now, NULL);
        if (TV_DIFF(now, start) >= RTO && !send_buf.packets.empty()) {
            packet first_pkt = send_buf.packets.front();
            set_ack_packet(&first_pkt);
            print_diag(&first_pkt, RTOD);
            sendto(sockfd, &first_pkt,
                   PACKET_HEADER_SIZE + ntohs(first_pkt.length),
                   0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
            gettimeofday(&start, NULL);

        }
        // Duplicate ACKS detected
        else if (dup_acks == DUP_ACKS && !send_buf.packets.empty()) {
            dup_acks = 0;
            packet first_pkt = send_buf.packets.front();
            set_ack_packet(&first_pkt);
            print_diag(&first_pkt, DUPA);
            sendto(sockfd, &first_pkt,
                   PACKET_HEADER_SIZE + ntohs(first_pkt.length),
                   0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));

        }
        // No data to send, so restart timer
        else if (send_buf.packets.empty()) {
            gettimeofday(&start, NULL);
        }
    }
}
