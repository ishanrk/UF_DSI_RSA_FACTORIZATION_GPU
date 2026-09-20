// SPDX-License-Identifier: LGPL-2.1-or-later
#include "lattice.hpp"
#include <limits>

static void require(bool ok)
{
    if (!ok)
        throw std::runtime_error("special-q reference check failed");
}

int main()
{
    glas::special_q job{};
    std::istringstream in("\n # comment\n1 101 7 # root\n0 103 0\n");
    require(glas::read_special_q(in, job));
    require(job.side == 1 && job.q == 101 && job.rho == 7);
    require(glas::read_special_q(in, job));
    require(job.side == 0 && job.q == 103 && job.rho == 0);
    require(!glas::read_special_q(in, job));
    std::istringstream largest("1 4294967295 4294967294");
    require(glas::read_special_q(largest, job) && job.q == UINT32_MAX);
    for (auto text : {"2 101 7", "-1 101 7", "1 0 0", "1 -7 0",
                      "1 101 -1", "1 101 101", "1 4294967296 0",
                      "1 101", "1 101 7 extra", "bad", "1 101 7.5"}) {
        std::istringstream invalid(text);
        bool caught = false;
        try { glas::read_special_q(invalid, job); }
        catch (std::invalid_argument const &) { caught = true; }
        require(caught);
    }
    glas::special_q const q{1, 101, 7};
    auto const basis = glas::q_lattice(q);
    require(basis[0][0] * basis[1][1] - basis[0][1] * basis[1][0] == q.q);
    for (int64_t u = -4; u <= 4; ++u) {
        for (int64_t v = -4; v <= 4; ++v) {
            int64_t const a = u * basis[0][0] + v * basis[1][0];
            int64_t const b = u * basis[0][1] + v * basis[1][1];
            require(glas::in_q_lattice(q, a, b));
            require(!glas::in_q_lattice(q, a + 1, b));
        }
    }
    for (int64_t a = -20; a <= 20; ++a)
        for (int64_t b = -20; b <= 20; ++b)
            require(glas::in_q_lattice(q, a, b) == ((a - 7 * b) % 101 == 0));
    auto const lo = std::numeric_limits<int64_t>::min();
    auto const hi = std::numeric_limits<int64_t>::max();
    require(glas::in_q_lattice({1, UINT32_MAX, 1}, lo, hi));
    require(glas::in_q_lattice({1, UINT32_MAX, UINT32_MAX - 1}, -hi, hi));
    require(glas::in_q_lattice({1, UINT32_MAX, UINT32_MAX - 1}, 1, -1));
    require(glas::in_q_lattice({0, 2, 1}, lo, lo));
    bool caught = false;
    try { glas::q_lattice({1, 0, 0}); }
    catch (std::invalid_argument const &) { caught = true; }
    require(caught);
    return 0;
}
