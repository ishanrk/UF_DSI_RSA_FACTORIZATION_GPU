// SPDX-License-Identifier: LGPL-2.1-or-later
#include "gf2.hpp"

namespace bwc {
std::vector<uint64_t> matmul(gf2_matrix const & m,
                             std::vector<uint64_t> const & x)
{
    check_matrix(m);
    if (x.size() != m.ncols)
        throw std::invalid_argument("input block has wrong dimensions");
    std::vector<uint64_t> y(m.offsets.size() - 1, 0);
    for (size_t i = 0; i < y.size(); ++i) {
        for (size_t k = m.offsets[i]; k < m.offsets[i + 1]; ++k)
            y[i] ^= x[m.columns[k]];
    }
    return y;
}
}
