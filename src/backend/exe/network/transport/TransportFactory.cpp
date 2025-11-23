#include "TransportFactory.hpp"

#include "IpcTransport.hpp"
#include "TcpTransport.hpp"

std::unique_ptr<ITransport> TransportFactory::create(int fd, NetworkMode network) {
    switch (network) {
        case NetworkMode::IPC:
            return std::make_unique<IpcTransport>(fd);
        case NetworkMode::TCP:
        default:
            return std::make_unique<TcpTransport>(fd);
    }
}