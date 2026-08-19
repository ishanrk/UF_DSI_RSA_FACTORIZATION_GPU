// SPDX-License-Identifier: LGPL-2.1-or-later
#include "gf2.hpp"

static void require(bool ok)
{
    if (!ok)
        throw std::runtime_error("GF(2) reference check failed");
}

template<class F> static void rejects(F f)
{
    try { f(); }
    catch (std::invalid_argument const &) { return; }
    throw std::runtime_error("invalid input was accepted");
}

int main()
{
    bwc::gf2_matrix m{3, {0, 2, 2, 4, 6}, {0, 2, 1, 1, 0, 1}};
    std::vector<uint64_t> x{1, 2, uint64_t{1} << 63};
    std::vector<uint64_t> expected{1 ^ x[2], 0, 0, 3};
    require(bwc::matmul(m, x) == expected);
    require(bwc::matmul({0, {0}, {}}, {}).empty());
    rejects([&] { bwc::matmul(m, {1}); });
    rejects([] { bwc::check_matrix({2, {}, {}}); });
    rejects([] { bwc::check_matrix({2, {1}, {}}); });
    rejects([] { bwc::check_matrix({2, {0, 2, 1, 2}, {0, 1}}); });
    rejects([] { bwc::check_matrix({2, {0, 1}, {2}}); });
    return 0;
}
