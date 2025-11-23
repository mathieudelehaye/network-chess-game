#pragma once

#include <atomic>
#include <thread>

#include "ITransport.hpp"

/**
 * @class IpcTransport
 * @brief Concrete transport implementation using POSIX Unix domain sockets
 */
class IpcTransport : public ITransport {
   public:
    /**
     * @brief Construct a transport from an already-accepted Unix socket descriptor.
     *
     * @param socket_fd File descriptor representing an open Unix domain socket connection.
     */
    IpcTransport(int socket_fd);

    /**
     * @brief Destructor closes the socket and stops background threads.
     */
    ~IpcTransport();

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
     * - Shuts down the Unix socket connection.
     * - Closes the underlying socket file descriptor.
     *
     * After this call, the session and upper layers must treat the connection as
     * closed.
     */
    void close() override;

    /**
     * @brief Sets callback to be invoked when connection closes unexpectedly.
     *
     * @param onClose Callback function to handle connection closure.
     */
    void setCloseCallback(CloseCallback onClose) override;

    /**
     * @brief Implements the connect() method from ITransport.
     * Since the socket is already connected, this simply returns true.
     */
    bool connect() override;

   private:
    int fd;                            ///< Underlying POSIX Unix socket descriptor.
    std::jthread readerThread;         ///< Background thread reading the socket.
    std::atomic<bool> running{false};  ///< Indicates whether the read loop is active.
    CloseCallback closeCallback_;      ///< Callback invoked on unexpected closure.
};