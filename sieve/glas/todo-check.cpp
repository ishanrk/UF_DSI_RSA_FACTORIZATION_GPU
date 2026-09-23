// SPDX-License-Identifier: LGPL-2.1-or-later
#include "special-q.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char ** argv)
{
    if (argc != 3 || std::string(argv[1]) != "-todo") {
        std::cerr << "usage: glas-todo-check -todo FILE (32-bit affine roots)\n";
        return 1;
    }
    try {
        std::ifstream in(argv[2]);
        if (!in)
            throw std::runtime_error("cannot open special-q todo list");
        glas::special_q job{};
        size_t count = 0;
        while (glas::read_special_q(in, job))
            ++count;
        std::cout << "Checked " << count << " affine special-q entries\n";
    } catch (std::exception const & error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
