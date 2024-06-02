#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <utility>

namespace raii {
struct open {
    template <typename... Ts>
    open(Ts &&...args) noexcept
            : descriptor(::open(std::forward<Ts>(args)...)) {}

    ~open() noexcept {
        if (descriptor != -1)
            ::close(descriptor);
    }

    operator auto() const noexcept {
        return descriptor;
    }

private:
    const int descriptor{};
};
} // namespace raii
