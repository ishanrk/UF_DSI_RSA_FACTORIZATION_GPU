// SPDX-License-Identifier: LGPL-2.1-or-later
#include "gf2.hpp"

namespace bwc {
std::vector<std::array<uint64_t, 64>> krylov(gf2_matrix const & m,
    std::vector<uint64_t> const & x, std::vector<uint64_t> y, size_t count)
{
    check_matrix(m);
    if (m.offsets.size() - 1 != m.ncols ||
        x.size() != m.ncols || y.size() != m.ncols)
        throw std::invalid_argument("Krylov reference requires square dimensions");
    // Small CPU reference for X^T M^i Y, before CADO balancing/permutations.
    std::vector<std::array<uint64_t, 64>> sequence;
    for (size_t i = 0; i < count; ++i) {
        sequence.push_back(project(x, y));
        if (i + 1 < count)
            y = matmul(m, y);
    }
    return sequence;
}

std::array<uint64_t, 64> project(std::vector<uint64_t> const & x,
                                  std::vector<uint64_t> const & y)
{
    if (x.size() != y.size())
        throw std::invalid_argument("projection blocks have different lengths");
    std::array<uint64_t, 64> result{};
    for (unsigned b = 0; b < 64; ++b) {
        for (size_t i = 0; i < x.size(); ++i)
            if (x[i] & (uint64_t{1} << b))
                result[b] ^= y[i];
    }
    return result;
}

static uint32_t read32(std::istream & in)
{
    unsigned char bytes[4];
    if (!in.read(reinterpret_cast<char *>(bytes), 4))
        throw std::invalid_argument("truncated CADO matrix");
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i)
        value |= uint32_t{bytes[i]} << (8 * i);
    return value;
}

gf2_matrix read_cado_matrix(std::istream & in, uint32_t nrows,
                           uint32_t ncols)
{
    // CADO's GF(2) matrix file has no header; dimensions come from the run.
    gf2_matrix m{ncols, {0}, {}};
    for (uint32_t i = 0; i < nrows; ++i) {
        uint32_t const weight = read32(in);
        for (uint32_t k = 0; k < weight; ++k) {
            uint32_t const j = read32(in);
            if (j >= ncols)
                throw std::invalid_argument("CADO matrix column out of range");
            m.columns.push_back(j);
        }
        m.offsets.push_back(m.columns.size());
    }
    if (in.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("extra data after CADO matrix rows");
    return m;
}

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

std::vector<uint64_t> matmul_transpose(gf2_matrix const & m,
                                       std::vector<uint64_t> const & x)
{
    check_matrix(m);
    if (x.size() != m.offsets.size() - 1)
        throw std::invalid_argument("transpose input has wrong dimensions");
    std::vector<uint64_t> y(m.ncols, 0);
    for (size_t i = 0; i < x.size(); ++i) {
        for (size_t k = m.offsets[i]; k < m.offsets[i + 1]; ++k)
            y[m.columns[k]] ^= x[i];
    }
    return y;
}
}
