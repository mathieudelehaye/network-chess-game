#pragma once

#include <functional>
#include <string>

/**
 * @class ITransport
 * @brief Abstract interface for all transport mechanisms.
 *
 * This interface defines the communication contract between the server
 * and any transport implementation (TCP, IPC,  etc.). Concrete transport
 * classes must implement send(), start(), and close(). A transport delivers
 * raw text messages to the upper networking layer and sends serialized responses.
 *
 * This abstraction allows the server to switch between transport implementations
 * without modifying controllers or model logic (Strategy pattern).
 */
class ITransport {
   public:
    /**
     * @brief Function type called when data is received on the transport.
     *
     * The callback receives raw text extracted from the socket or other transport
     * mechanism.
     */
    using ReceiveCallback = std::function<void(const std::string&)>;

    using CloseCallback = std::function<void()>;

    /// Virtual destructor
    virtual ~ITransport() = default;

    /**
     * @brief Starts the transport input loop.
     *
     * Implementations typically start a background thread that continuously reads
     * from the connection. When data is received, the provided callback is invoked.
     *
     * @param onReceive Callback invoked each time a full payload is received.
     */
    virtual void start(ReceiveCallback onReceive) = 0;

    /**
     * @brief Sends raw text data through the transport.
     *
     * @param data Message to send
     */
    virtual void send(const std::string& data) = 0;

    /**
     * @brief Closes the underlying transport connection.
     *
     * Concrete implementations must stop background threads and release system
     * resources.
     */
    virtual void close() = 0;

    /**
     * @brief Sets a callback to be invoked when the transport closes unexpectedly.
     * @param onClose Callback to handle connection closure.
     */
    virtual void setCloseCallback(CloseCallback onClose) = 0;
};