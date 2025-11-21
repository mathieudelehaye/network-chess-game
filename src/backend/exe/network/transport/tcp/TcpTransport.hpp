#pragma once

#include <atomic>
#include <thread>

#include "ITransport.hpp"

/**
 * @class TcpTransport
 * @brief Concrete transport implementation using POSIX TCP/IP sockets
 *
 * This class implements the ITransport interface using a standard
 * POSIX TCP/IP socket. It provides a bidirectional text-based
 * communication channel used by the server to exchange messages with
 * remote clients (console frontend, GUI, etc.).
 */
class TcpTransport : public ITransport {
public:
    /**
     * @brief Construct a transport from an already-accepted socket descriptor.
     *
     * @param socket_fd File descriptor representing an open TCP connection.
     */
    TcpTransport(int socket_fd);

    /**
     * @brief Destructor closes the socket and stops background threads.
     */
    ~TcpTransport();

    /**
     * @brief Starts the asynchronous reading loop.
     *
     * This method spawns a thread that monitors the socket for incoming data.
     * When data is available, the thread invokes the provided callback.
     *
     * @param onReceive Functor invoked when a full text payload is received.
     */
    void start(ReceiveCallback onReceive) override;

    /**
     * @brief Sends raw text data to the connected client.
     *
     * Internally this writes on the stored socket descriptor.
     *
     * @param data The raw string to send.
     */
    void send(const std::string& data) override;

    /**
     * @brief Closes the transport connection and terminates the reading loop.
     *
     * This method:
     * - Stops the background reading thread.
     * - Shuts down the TCP connection.
     * - Closes the underlying socket file descriptor.
     *
     * After this call, the session and upper layers must treat the connection as
     * closed.
     */
    void close() override;

    void setCloseCallback(CloseCallback onClose) override;

private:
    int fd;                            ///< Underlying POSIX socket descriptor.
    std::jthread readerThread;         ///< Background thread reading the socket.
    std::atomic<bool> running{false};  ///< Indicates whether the read loop is active.
    CloseCallback closeCallback_;
};
