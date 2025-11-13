#include "TransportFactory.hpp"

#include "PosixTcpTransport.hpp"

std::unique_ptr<ITransport> TransportFactory::create(int fd, NetworkMode network) {
    switch (network) {
            // TODO: add IPC case

        case NetworkMode::TCP:
        default:
            return std::make_unique<PosixTcpTransport>(fd);
    }
}