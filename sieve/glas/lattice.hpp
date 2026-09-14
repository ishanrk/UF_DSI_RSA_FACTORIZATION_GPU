// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef GLAS_LATTICE_HPP
#define GLAS_LATTICE_HPP
#include "special-q.hpp"
#include <array>

namespace glas {
inline void check_special_q(special_q const & job)
{
    if (job.side > 1 || job.q < 2 || job.rho >= job.q)
        throw std::invalid_argument("invalid affine special-q");
}

inline std::array<std::array<int64_t, 2>, 2> q_lattice(special_q const & job)
{
    check_special_q(job);
    // Unreduced basis for a = rho*b (mod q); skew reduction comes later.
    return {{{job.q, 0}, {job.rho, 1}}};
}

inline bool in_q_lattice(special_q const & job, int64_t a, int64_t b)
{
    check_special_q(job);
    auto residue = [&job](int64_t value) {
        int64_t const r = value % job.q;
        return static_cast<uint64_t>(r < 0 ? r + job.q : r);
    };
    // Reduced 32-bit factors keep the product within uint64_t, even for
    // INT64_MIN/MAX coordinates; do not form a-rho*b in signed arithmetic.
    return residue(a) == (uint64_t{job.rho} * residue(b)) % job.q;
}
}
#endif
