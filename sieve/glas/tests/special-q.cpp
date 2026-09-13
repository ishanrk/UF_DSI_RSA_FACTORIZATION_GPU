// SPDX-License-Identifier: LGPL-2.1-or-later
#include "special-q.hpp"

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
    return 0;
}
