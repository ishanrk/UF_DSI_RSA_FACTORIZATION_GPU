// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef GLAS_SPECIAL_Q_HPP
#define GLAS_SPECIAL_Q_HPP
#include <cstdint>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace glas {
struct special_q {
    unsigned side;
    uint32_t q, rho;
};

inline bool read_special_q(std::istream & in, special_q & job)
{
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line.substr(0, line.find('#')));
        if ((fields >> std::ws).eof())
            continue;
        int64_t side, q, rho;
        std::string tail;
        if (!(fields >> side >> q >> rho) || (fields >> tail) ||
            side < 0 || side > 1 || q < 2 || q > UINT32_MAX ||
            rho < 0 || rho >= q)
            throw std::invalid_argument("expected side q rho with 32-bit affine q");
        job = {static_cast<unsigned>(side), static_cast<uint32_t>(q),
               static_cast<uint32_t>(rho)};
        return true;
    }
    if (!in.eof())
        throw std::invalid_argument("cannot read special-q todo list");
    return false;
}
}
#endif
