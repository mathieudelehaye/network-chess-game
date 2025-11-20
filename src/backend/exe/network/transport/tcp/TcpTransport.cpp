#include "TcpTransport.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "Logger.hpp"

/**
 * @brief Constructs a POSIX TCP transport with an existing socket.
 * @param socket_fd The file descriptor of the connected socket.
 */
TcpTransport::TcpTransport(int socket_fd) : fd(socket_fd) {}

/**
 * @brief Destructor ensures that the socket is closed.
 */
TcpTransport::~TcpTransport() {
    close();
}

void TcpTransport::setCloseCallback(CloseCallback onClose) {
    closeCallback_ = onClose;
}

/**
 * @brief Starts receiving data on the transport.
 * @param onReceive Callback function to handle received data.
 */
void TcpTransport::start(ReceiveCallback onReceive) {
    // Required since nothing prevents start() from being called twice for the same instance.
    if (running.exchange(true))
        return;

    auto& logger = Logger::instance();
    logger.trace("Starting reader thread for fd " + std::to_string(fd));

    readerThread = std::jthread([this, onReceive](std::stop_token st) {
        auto& logger = Logger::instance();
        logger.trace("Reader thread started for fd " + std::to_string(fd));

        char buffer[1024];
        bool connection_closed_by_peer = false;

        while (!st.stop_requested() && running.load()) {
            logger.trace("Calling read() on fd " + std::to_string(fd));
            ssize_t n = read(fd, buffer, sizeof(buffer));
            logger.trace("read() returned " + std::to_string(n) + " for fd " + std::to_string(fd));

            if (n <= 0) {
                if (n == 0) {
                    logger.trace("Client disconnected (EOF) on fd " + std::to_string(fd));
                    connection_closed_by_peer = true;
                } else {
                    logger.error("Read error on fd " + std::to_string(fd) + ": " +
                                 std::string(strerror(errno)));
                    connection_closed_by_peer = true;
                }
                running = false;
                break;
            }

            std::string payload(buffer, n);
            onReceive(payload);
        }

        logger.trace("Reader thread EXITING for fd " + std::to_string(fd));

        // Notify session that connection died
        if (connection_closed_by_peer && closeCallback_) {
            logger.trace("Invoking close callback for fd " + std::to_string(fd));
            closeCallback_();
        }
    });
}

/**
 * @brief Sends data over the transport.
 * @param data The data to send.
 * @return Number of bytes sent, or -1 on error.
 */
void TcpTransport::send(const std::string& data) {
    if (!running.load()) {
        return;
    }

    ssize_t sent = write(fd, data.data(), data.size());
    if (sent < 0) {
        auto& logger = Logger::instance();
        logger.error("Write error on fd " + std::to_string(fd) + ": " +
                     std::string(strerror(errno)));
        running = false;
    }
}

/**
 * @brief Closes the TCP socket and terminates the reading loop.
 */
void TcpTransport::close() {
    // Required to be consistent with the similar check in start().
    if (!running.exchange(false))
        return;

    auto& logger = Logger::instance();
    logger.debug("Closing transport on fd " + std::to_string(fd));

    // Close socket first to unblock read()
    if (fd >= 0) {
        shutdown(fd, SHUT_RDWR);
        ::close(fd);
        fd = -1;
    }

    logger.debug("Transport closed");
}
