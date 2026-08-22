// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef BWC_GF2_HPP
#define BWC_GF2_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <vector>

namespace bwc {
struct gf2_matrix {
    uint32_t ncols;
    std::vector<size_t> offsets;
    std::vector<uint32_t> columns;
};

// Each word packs 64 independent GF(2) vectors at the same coordinate.
std::vector<uint64_t> matmul(gf2_matrix const & m,
                             std::vector<uint64_t> const & x);
gf2_matrix read_cado_matrix(std::istream & in, uint32_t nrows,
                           uint32_t ncols);

inline void check_matrix(gf2_matrix const & m)
{
    if (m.offsets.empty() || m.offsets.front() != 0 ||
        m.offsets.back() != m.columns.size() ||
        !std::is_sorted(m.offsets.begin(), m.offsets.end()))
        throw std::invalid_argument("invalid sparse row offsets");
    for (auto j : m.columns)
        if (j >= m.ncols)
            throw std::invalid_argument("matrix column outside dimensions");
}
}
#endif
