#include "PosixTcpTransport.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

/**
 * @brief Constructs a POSIX TCP transport with an existing socket.
 * @param socket_fd The file descriptor of the connected socket.
 */
PosixTcpTransport::PosixTcpTransport(int socket_fd) : fd(socket_fd) {}

/**
 * @brief Destructor ensures that the socket is closed.
 */
PosixTcpTransport::~PosixTcpTransport() {
    close();
}

/**
 * @brief Starts receiving data on the transport.
 * @param onReceive Callback function to handle received data.
 */
void PosixTcpTransport::start(ReceiveCallback onReceive) {
    running = true;

    readerThread = std::jthread([this, onReceive](std::stop_token st) {
        char buffer[1024];

        while (!st.stop_requested() && running.load()) {
            ssize_t n = ::read(fd, buffer, sizeof(buffer));

            // connection closed or broken
            if (n <= 0) {
                running = false;
                break;
            }

            std::string payload(buffer, n);
            onReceive(payload);
        }
    });
}

/**
 * @brief Sends data over the transport.
 * @param data The data to send.
 * @return Number of bytes sent, or -1 on error.
 */
void PosixTcpTransport::send(const std::string& data) {
    ::write(fd, data.data(), data.size());
}

/**
 * @brief Closes the TCP socket and terminates the reading loop.
 */
void PosixTcpTransport::close() {
    running = false;

    if (fd >= 0) {
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
        fd = -1;
    }
}
