#pragma once

#include <memory>

#include "ITransport.hpp"
#include "NetworkMode.hpp"

struct TransportFactory {
    static std::unique_ptr<ITransport> create(int fd, NetworkMode network);
};