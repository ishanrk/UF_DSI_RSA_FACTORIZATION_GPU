// SPDX-License-Identifier: LGPL-2.1-or-later
#include "gf2.hpp"
#include <sstream>

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
    require(bwc::matmul_transpose(m, {8, 4, 2, 1}) ==
            std::vector<uint64_t>({9, 1, 8}));
    require(bwc::matmul_transpose({3, {0}, {}}, {}) ==
            std::vector<uint64_t>({0, 0, 0}));
    rejects([&] { bwc::matmul_transpose(m, {1}); });
    require(bwc::matmul({0, {0}, {}}, {}).empty());
    rejects([&] { bwc::matmul(m, {1}); });
    rejects([] { bwc::check_matrix({2, {}, {}}); });
    rejects([] { bwc::check_matrix({2, {1}, {}}); });
    rejects([] { bwc::check_matrix({2, {0, 2, 1, 2}, {0, 1}}); });
    rejects([] { bwc::check_matrix({2, {0, 1}, {2}}); });

    std::string const bytes("\x02\0\0\0" "\0\0\0\0"
                            "\x02\0\0\0" "\0\0\0\0", 16);
    std::istringstream in(bytes);
    auto const loaded = bwc::read_cado_matrix(in, 2, 3);
    require(loaded.offsets == std::vector<size_t>({0, 2, 2}));
    require(loaded.columns == std::vector<uint32_t>({0, 2}));
    require(bwc::matmul(loaded, x) == std::vector<uint64_t>({1 ^ x[2], 0}));
    for (size_t n = 0; n < bytes.size(); ++n) {
        rejects([&] {
            std::istringstream short_input(bytes.substr(0, n));
            bwc::read_cado_matrix(short_input, 2, 3);
        });
    }
    rejects([&] {
        std::istringstream extra(bytes + '\0');
        bwc::read_cado_matrix(extra, 2, 3);
    });
    rejects([&] {
        auto bad = bytes;
        bad[8] = 3;
        std::istringstream out_of_range(bad);
        bwc::read_cado_matrix(out_of_range, 2, 3);
    });
    std::istringstream endian(std::string("\x01\0\0\0" "\0\x01\0\0", 8));
    require(bwc::read_cado_matrix(endian, 1, 257).columns[0] == 256);
    std::istringstream empty;
    require(bwc::read_cado_matrix(empty, 0, 0).offsets == std::vector<size_t>({0}));
    return 0;
}
